// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "ForkBranch.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "Runtime/Engine/Public/DrawDebugHelpers.h"
#include "Fork.h"

// Sets default values
UForkBranch::UForkBranch()
{
	UE_LOG(LogTemp, Warning, TEXT("ForkBranch constructor is called!"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder
	(TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'"));
	Mesh = MeshFinder.Object;

	static ConstructorHelpers::FObjectFinder<UMaterial> MaterialFinder
	(TEXT("Material'/TrafficMonitor/TransparentBlue.TransparentBlue'"));
	Material = MaterialFinder.Object;
}


void UForkBranch::Init(USceneComponent* RootComponent,	AIntersectionExit* MyExit)
{
	MyFork = Cast<AFork>(GetOwner());
	
	this->MyExit = MyExit;
	SetupAttachment(RootComponent);
	SetHiddenInGame(true);
	SetMobility(EComponentMobility::Static);
	RegisterComponent();

	// setup the two spline points and tangents
	MinimizeCurvatureVariation();

	// setup the SplineMeshComponents
	float MaxMeshLengthCM = 100.f*MaxMeshLength;
	int NumberOfMeshes = FMath::CeilToInt(GetSplineLength() / MaxMeshLengthCM);
	float MeshLength = GetSplineLength() / NumberOfMeshes;

	float EntranceWidth = MyFork->EntranceTriggerVolume->GetScaledBoxExtent().Y;
	float ExitWidth = MyExit->TriggerVolume->GetScaledBoxExtent().Y;
	float WidthChange = ExitWidth - EntranceWidth;

	for (int MeshIndex = 0; MeshIndex < NumberOfMeshes; MeshIndex++)
	{
		auto StartPosition = GetLocationAtDistanceAlongSpline(MeshIndex*MeshLength, ESplineCoordinateSpace::Local);
		auto EndPosition = GetLocationAtDistanceAlongSpline((MeshIndex+1)*MeshLength, ESplineCoordinateSpace::Local);
		auto StartTangent = GetTangentAtDistanceAlongSpline(MeshIndex*MeshLength, ESplineCoordinateSpace::Local);
		auto EndTangent = GetTangentAtDistanceAlongSpline((MeshIndex+1)*MeshLength, ESplineCoordinateSpace::Local);
		auto StartDirection = StartTangent.GetSafeNormal();
		auto EndDirection = EndTangent.GetSafeNormal();

		USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this);

		SplineMesh->CreationMethod = EComponentCreationMethod::Instance;
		SplineMesh->SetMobility(EComponentMobility::Static);
		SplineMesh->SetupAttachment(this);
		SplineMesh->SetStaticMesh(Mesh);
		SplineMesh->SetMaterial(0, Cast<UMaterialInterface>(Material));

		SplineMesh->SetStartPosition(StartPosition);
		SplineMesh->SetEndPosition(EndPosition);
		SplineMesh->SetStartTangent(StartDirection);
		SplineMesh->SetEndTangent(EndDirection);

		SplineMesh->SetStartScale(FVector2D{ (EntranceWidth + WidthChange * MeshIndex / NumberOfMeshes)/50.f, 1.f });
		SplineMesh->SetEndScale(FVector2D{ (EntranceWidth + WidthChange * (MeshIndex + 1) / NumberOfMeshes)/50.f, 1.f });

		SplineMesh->RegisterComponent();
		SplineMesh->UpdateRenderStateAndCollision();

		SplineMeshComponents.Add(SplineMesh);
	}
}

// Reference: 2011_Curvature variation minimizing cubic Hermite interpolants
void UForkBranch::MinimizeCurvatureVariation()
{
	FVector EntranceLocation = MyFork->GetActorLocation();
	FVector EntranceDirection = MyFork->GetActorForwardVector();
	FVector ExitLocation = MyExit->GetActorLocation();
	FVector ExitDirection = MyExit->GetActorForwardVector();

	SetLocationAtSplinePoint(0, EntranceLocation, ESplineCoordinateSpace::World);
	SetLocationAtSplinePoint(1, ExitLocation, ESplineCoordinateSpace::World);

	FVector2D p0 = FVector2D(EntranceLocation.X, EntranceLocation.Y);
	FVector2D p1 = FVector2D(ExitLocation.X, ExitLocation.Y);
	FVector2D d0 = FVector2D(EntranceDirection.X, EntranceDirection.Y);
	FVector2D d1 = FVector2D(ExitDirection.X, ExitDirection.Y);
	float c01 = FVector2D::CrossProduct(d0, d1);
	FVector2D delta_p = p1 - p0;
	float c0 = FVector2D::CrossProduct(d0, delta_p);
	float c1 = FVector2D::CrossProduct(d1, delta_p);
	float alpha0 = 0.f;
	float alpha1 = 0.f;
	if (c0 * c01 > 0 && c1 * c01 < 0)
	{
		alpha0 = -2.f*c1 / c01;
		alpha1 = 2.f*c0 / c01;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Need to handle a special case!"));
		// TODO: seprate this case into all subcases
	}

	SetTangentAtSplinePoint(0, EntranceDirection*alpha0, ESplineCoordinateSpace::World);
	SetTangentAtSplinePoint(1, ExitDirection*alpha1, ESplineCoordinateSpace::World);
}

#if WITH_EDITOR
void UForkBranch::PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	UE_LOG(LogTemp, Warning, TEXT("PostEditChangeProperty called inside ForkBranch!"));
	UE_LOG(LogTemp, Warning, TEXT("Change Type: %d \n"), PropertyChangedEvent.ChangeType);
}
#endif // WITH_EDITOR


void UForkBranch::OnBranchBeginOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (OtherActor != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s begins overlap lane %s"), *(OtherActor->GetFName().ToString()), *(GetFName().ToString()));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("OtherActor is null in OnBranchBeginOverlap!"));
	}
}

void UForkBranch::BeginPlay()
{
	Super::BeginPlay();

	for (auto& SplineMeshComponent : SplineMeshComponents)
	{
		//if (!SplineMeshComponent->OnComponentBeginOverlap.IsAlreadyBound(this, &UForkBranch::OnBranchBeginOverlap))
		SplineMeshComponent->SetCollisionProfileName(FName("OverlapAll"));
		SplineMeshComponent->SetGenerateOverlapEvents(true);
		SplineMeshComponent->OnComponentBeginOverlap.AddDynamic(this, &UForkBranch::OnBranchBeginOverlap);
	}
}

void UForkBranch::DestroyComponent(bool bPromoteChildren=false)
{
	for (auto& SplineMeshComponent : SplineMeshComponents)
	{
		SplineMeshComponent->DestroyComponent();
	}
	Super::DestroyComponent(bPromoteChildren);
}