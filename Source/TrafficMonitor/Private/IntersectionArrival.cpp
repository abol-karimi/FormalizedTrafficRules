// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "IntersectionArrival.h"

#include "Vehicle/CarlaWheeledVehicle.h"


// Sets default values
AIntersectionArrival::AIntersectionArrival(const FObjectInitializer &ObjectInitializer)
	:Super(ObjectInitializer)
{
	RootComponent =
		ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("SceneRootComponent"));
	RootComponent->SetMobility(EComponentMobility::Static);

	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	TriggerVolume->SetupAttachment(RootComponent);
	TriggerVolume->SetHiddenInGame(true);
	TriggerVolume->SetMobility(EComponentMobility::Static);
	TriggerVolume->SetCollisionProfileName(FName("OverlapAll"));
	TriggerVolume->SetGenerateOverlapEvents(true);
	TriggerVolume->SetBoxExtent(FVector{ 100.0f, 150.f, 50.0f });
	TriggerVolume->ShapeColor = FColor(0, 0, 255);

	ForwardArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("ForwardArrow"));
	ForwardArrow->SetupAttachment(RootComponent);
	ForwardArrow->SetHiddenInGame(true);
	ForwardArrow->SetMobility(EComponentMobility::Static);
	ForwardArrow->SetWorldScale3D(FVector{ 3.f, 3.f, 3.f });

}

// Called when the game starts or when spawned
void AIntersectionArrival::BeginPlay()
{
	Super::BeginPlay();

	TriggerVolume->SetCollisionProfileName(FName("OverlapAll"));
	TriggerVolume->SetGenerateOverlapEvents(true);
	TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AIntersectionArrival::OnArrival);
	
}

void AIntersectionArrival::OnArrival(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (OtherActor != nullptr)
	{
		if (MyMonitor != nullptr)
		{
			int32 TimeStep = FMath::FloorToInt(GetWorld()->GetTimeSeconds() / MyMonitor->TimeResolution);
			FString EventMessage = "arrivesFromAtTime(_" 
				+ OtherActor->GetName() + ", _" 
				+ GetName() + ", "
				+ FString::FromInt(TimeStep) + ").";
			UE_LOG(LogTemp, Warning, TEXT("%s"), *(EventMessage));
			MyMonitor->AddEvent(EventMessage);

			ACarlaWheeledVehicle* OverLappingVehicle = Cast<ACarlaWheeledVehicle>(OtherActor);
			if (OverLappingVehicle != nullptr)
			{
				auto Signal = OverLappingVehicle->Signal;
				FString SignalString;
				switch (Signal)	{
				case EVehicleSignalState::Off:
					SignalString = "off";
					break;
				case EVehicleSignalState::Left:
					SignalString = "left";
					break;
				case EVehicleSignalState::Right:
					SignalString = "right";
					break;
				case EVehicleSignalState::Emergency:
					SignalString = "emergency";
					break;
				}
				EventMessage = "signalsFromAtTime(_"
					+ OtherActor->GetName() + ", "
					+ SignalString + ", _"
					+ GetName() + ", "
					+ FString::FromInt(TimeStep) + ").";
				UE_LOG(LogTemp, Warning, TEXT("%s"), *(EventMessage));
				MyMonitor->AddEvent(EventMessage);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Cast to ACarlaWheeledVehicle failed!"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("MyMonitor is null in AFork::OnEntrance!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("OtherActor is null in AIntersectionExit::OnExit!"));
	}
}

