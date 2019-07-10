// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "ForkBranch.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"

// Sets default values
UForkBranch::UForkBranch()
{
	UE_LOG(LogTemp, Warning, TEXT("ForkBranch constructor is called!"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder
	(TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'"));
	Mesh = MeshFinder.Object;
}


void UForkBranch::Init(USceneComponent* RootComponent,
	FVector EntranceLocation,
	FVector EntranceDirection,
	FVector ExitLocation,
	FVector ExitDirection)
{
	SetupAttachment(RootComponent);
	SetHiddenInGame(true);
	SetMobility(EComponentMobility::Static);
	RegisterComponent();

	// setup the spline points (assuming two points on the spline)
	SetLocationAtSplinePoint(0, EntranceLocation, ESplineCoordinateSpace::World);
	SetTangentAtSplinePoint(0, EntranceDirection*1000.f, ESplineCoordinateSpace::World);
	SetLocationAtSplinePoint(1, ExitLocation, ESplineCoordinateSpace::World);
	SetTangentAtSplinePoint(1, ExitDirection*1000.f, ESplineCoordinateSpace::World);

	// setup the SplineMeshComponents
	USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this);

	SplineMesh->CreationMethod = EComponentCreationMethod::Instance;
	SplineMesh->SetMobility(EComponentMobility::Static);
	SplineMesh->SetupAttachment(this);
	SplineMesh->SetStaticMesh(Mesh);
	SplineMesh->SetStartPosition(GetLocationAtSplinePoint(0, ESplineCoordinateSpace::Local));
	SplineMesh->SetEndPosition(GetLocationAtSplinePoint(1, ESplineCoordinateSpace::Local));
	SplineMesh->SetStartTangent(GetTangentAtSplinePoint(0, ESplineCoordinateSpace::Local));
	SplineMesh->SetEndTangent(GetTangentAtSplinePoint(1, ESplineCoordinateSpace::Local));
//	SplineMesh->SetStartScale();
//	SplineMesh->SetEndScale();

	SplineMesh->RegisterComponent();
	SplineMesh->UpdateRenderStateAndCollision();

	SplineMeshComponents.Add(SplineMesh);

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