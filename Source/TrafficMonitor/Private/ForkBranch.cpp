// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "ForkBranch.h"

// Sets default values
UForkBranch::UForkBranch()
{
	UE_LOG(LogTemp, Warning, TEXT("ForkBranch constructor is called!"));

	//ExitTriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("ExitTriggerVolume"));
	//ExitTriggerVolume->SetupAttachment(this);
	//ExitTriggerVolume->SetHiddenInGame(true);
	//ExitTriggerVolume->SetMobility(EComponentMobility::Static);
	//ExitTriggerVolume->SetCollisionProfileName(FName("OverlapAll"));
	//ExitTriggerVolume->SetGenerateOverlapEvents(true);
	//ExitTriggerVolume->SetBoxExtent(FVector{ 20.0f, 200.0f, 50.0f });
	//ExitTriggerVolume->SetRelativeLocation(FVector{ 200.f, 0.f, 0.f });
	//ExitTriggerVolume->RegisterComponent();
}

#if WITH_EDITOR
void UForkBranch::PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	UE_LOG(LogTemp, Warning, TEXT("PostEditChangeProperty called inside ForkBranch!"));
	UE_LOG(LogTemp, Warning, TEXT("Change Type: %d \n"), PropertyChangedEvent.ChangeType);

	//auto LastIndex = GetNumberOfSplinePoints() - 1;
	//auto EndPointLocation = GetLocationAtSplinePoint(LastIndex, ESplineCoordinateSpace::Local);
	//ExitTriggerVolume->SetRelativeLocation(EndPointLocation);
	//auto EndPointRotation = GetRotationAtSplinePoint(LastIndex, ESplineCoordinateSpace::Local);
	//ExitTriggerVolume->SetRelativeRotation(EndPointRotation);
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