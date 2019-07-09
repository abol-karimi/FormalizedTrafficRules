// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "ForkBranch.h"

// Sets default values
UForkBranch::UForkBranch()
{
	UE_LOG(LogTemp, Warning, TEXT("ForkBranch constructor is called!"));
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

}


#if WITH_EDITOR
void UForkBranch::PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	UE_LOG(LogTemp, Warning, TEXT("PostEditChangeProperty called inside ForkBranch!"));
	UE_LOG(LogTemp, Warning, TEXT("Change Type: %d \n"), PropertyChangedEvent.ChangeType);

}
#endif // WITH_EDITOR

void UForkBranch::OnExitFromLane(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (OtherActor != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s exited from lane"), *(OtherActor->GetFName().ToString()));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("OtherActor is null in OnExitFromLane!"));
	}

}

void UForkBranch::BeginPlay()
{
	Super::BeginPlay();

	//if (!ExitTriggerVolume->OnComponentBeginOverlap.IsAlreadyBound(this, &UForkBranch::OnExitFromLane))
	//{
	//	ExitTriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &UForkBranch::OnExitFromLane);
//	}
	
}