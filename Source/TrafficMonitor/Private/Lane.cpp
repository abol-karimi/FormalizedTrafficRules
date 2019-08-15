// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "Lane.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "Fork.h"



// Sets default values
ALane::ALane(const FObjectInitializer &ObjectInitializer)
	: Super(ObjectInitializer)
{
	RootComponent =
		ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("SceneRootComponent"));
	RootComponent->SetMobility(EComponentMobility::Static);

	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	Spline->SetupAttachment(RootComponent);
	Spline->SetHiddenInGame(true);
	Spline->SetMobility(EComponentMobility::Static);
	Spline->RegisterComponent();

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder
	(TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'"));
	Mesh = MeshFinder.Object;

	static ConstructorHelpers::FObjectFinder<UMaterial> MaterialFinder
	(TEXT("Material'/TrafficMonitor/TransparentBlue.TransparentBlue'"));
	Material = MaterialFinder.Object;
}

void ALane::OnConstruction(const FTransform &Transform)
{
	Super::OnConstruction(Transform);

	UE_LOG(LogTemp, Warning, TEXT("ALane OnConstruction called!"));
}

void ALane::Init(AFork* MyFork, AExit* MyExit)
{
	this->MyFork = MyFork;
	this->MyExit = MyExit;
	
	SetupSpline();
	SetupSplineMeshes();
}

FString ALane::GetCorrectSignal()
{
	auto EntranceDirection = MyFork->GetActorForwardVector();
	auto ExitDirection = MyExit->GetActorForwardVector();

	float Z = FVector::CrossProduct(EntranceDirection, ExitDirection).Z;
	auto Cosine = EntranceDirection.CosineAngle2D(FVector::VectorPlaneProject(ExitDirection, FVector(0.f, 0.f, 1.f)));

	EVehicleSignalState Signal = EVehicleSignalState::Off;
	if (Cosine < 0.7f) // turning angle more than 45 degrees
	{
		if (Z < 0)
		{
			Signal = EVehicleSignalState::Left;
		}
		else
		{
			Signal = EVehicleSignalState::Right;
		}
	}
	switch (Signal) {
	case EVehicleSignalState::Left:
		return FString("left");
	case EVehicleSignalState::Right:
		return FString("right");
	case EVehicleSignalState::Emergency:
		return FString("emergency");
	default:
		return FString("off");
	}
}

void ALane::SetupSpline()
{
	FVector EntranceLocation = MyFork->GetActorLocation();
	FVector EntranceDirection = MyFork->GetActorForwardVector();
	FVector ExitLocation = MyExit->GetActorLocation();
	FVector ExitDirection = MyExit->GetActorForwardVector();

	Spline->SetLocationAtSplinePoint(0, EntranceLocation, ESplineCoordinateSpace::World);
	Spline->SetLocationAtSplinePoint(1, ExitLocation, ESplineCoordinateSpace::World);

	FVector2D p0 = FVector2D(EntranceLocation.X, EntranceLocation.Y);
	FVector2D p1 = FVector2D(ExitLocation.X, ExitLocation.Y);
	FVector2D d0 = FVector2D(EntranceDirection.X, EntranceDirection.Y);
	FVector2D d1 = FVector2D(ExitDirection.X, ExitDirection.Y);
	float Alpha0 = 1.f;
	float Alpha1 = 1.f;
	float TurnAngleCosine = EntranceDirection.CosineAngle2D(ExitDirection);
	if (TurnAngleCosine < -0.8f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Need to make a U-turn!"));
		// Need a mid-turn interpolant
	}
	else if (MinimumCurvatureVariation(p0, p1, d0, d1, Alpha0, Alpha1))
	{
		Spline->SetTangentAtSplinePoint(0, EntranceDirection*Alpha0, ESplineCoordinateSpace::World);
		Spline->SetTangentAtSplinePoint(1, ExitDirection*Alpha1, ESplineCoordinateSpace::World);
	}
	else // Need an inflection point
	{
		UE_LOG(LogTemp, Warning, TEXT("Need an inflection point!"));
	}

}

void ALane::SetupSplineMeshes()
{
	// remove previous spline meshes
	for (auto& SplineMeshComponent : SplineMeshComponents)
	{
		SplineMeshComponent->DestroyComponent();
	}
	SplineMeshComponents.Empty();

	// setup the SplineMeshComponents
	float MaxMeshLengthCM = 100.f*MaxMeshLength;
	int NumberOfMeshes = FMath::CeilToInt(Spline->GetSplineLength() / MaxMeshLengthCM);
	float MeshLength = Spline->GetSplineLength() / NumberOfMeshes;

	float EntranceWidth = MyFork->EntranceTriggerVolume->GetScaledBoxExtent().Y;
	float ExitWidth = MyExit->TriggerVolume->GetScaledBoxExtent().Y;
	float WidthChange = ExitWidth - EntranceWidth;

	for (int MeshIndex = 0; MeshIndex < NumberOfMeshes; MeshIndex++)
	{
		auto StartPosition = Spline->GetLocationAtDistanceAlongSpline(MeshIndex*MeshLength, ESplineCoordinateSpace::Local);
		auto EndPosition = Spline->GetLocationAtDistanceAlongSpline((MeshIndex + 1)*MeshLength, ESplineCoordinateSpace::Local);
		auto StartTangent = Spline->GetTangentAtDistanceAlongSpline(MeshIndex*MeshLength, ESplineCoordinateSpace::Local);
		auto EndTangent = Spline->GetTangentAtDistanceAlongSpline((MeshIndex + 1)*MeshLength, ESplineCoordinateSpace::Local);
		auto StartDirection = StartTangent.GetSafeNormal();
		auto EndDirection = EndTangent.GetSafeNormal();

		USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this);

		SplineMesh->CreationMethod = EComponentCreationMethod::Instance;
		SplineMesh->SetMobility(EComponentMobility::Static);
		SplineMesh->SetupAttachment(RootComponent);
		SplineMesh->SetStaticMesh(Mesh);
		SplineMesh->SetMaterial(0, Cast<UMaterialInterface>(Material));

		SplineMesh->SetStartPosition(StartPosition);
		SplineMesh->SetEndPosition(EndPosition);
		SplineMesh->SetStartTangent(StartDirection);
		SplineMesh->SetEndTangent(EndDirection);

		SplineMesh->SetStartScale(FVector2D{ (EntranceWidth + WidthChange * MeshIndex / NumberOfMeshes) / 50.f, 1.f });
		SplineMesh->SetEndScale(FVector2D{ (EntranceWidth + WidthChange * (MeshIndex + 1) / NumberOfMeshes) / 50.f, 1.f });

		SplineMesh->SetCollisionProfileName(FName("OverlapAll"));
		SplineMesh->SetGenerateOverlapEvents(true);
		
		SplineMesh->RegisterComponent();
		SplineMesh->UpdateRenderStateAndCollision();

		SplineMeshComponents.Add(SplineMesh);
	}
}


// Reference: "2011_Curvature variation minimizing cubic Hermite interpolants"
bool ALane::MinimumCurvatureVariation(FVector2D p0, FVector2D p1, FVector2D d0, FVector2D d1, float& OutAlpha0, float& OutAlpha1)
{
	float c01 = FVector2D::CrossProduct(d0, d1);
	FVector2D delta_p = p1 - p0;
	float c0 = FVector2D::CrossProduct(d0, delta_p);
	float c1 = FVector2D::CrossProduct(d1, delta_p);
	float alpha0 = 1.f;
	float alpha1 = 1.f;
	if (c0 * c01 > 0 && c1 * c01 < 0)
	{
		OutAlpha0 = -2.f*c1 / c01;
		OutAlpha1 = 2.f*c0 / c01;
		return true;
	}
	return false;
}
