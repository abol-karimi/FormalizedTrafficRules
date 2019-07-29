// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "Fork.h"
#include "Vehicle/CarlaWheeledVehicle.h"

#include "Engine/CollisionProfile.h"

// Sets default values
AFork::AFork(const FObjectInitializer &ObjectInitializer)
	: Super(ObjectInitializer)
{
	RootComponent =
		ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("SceneRootComponent"));
	RootComponent->SetMobility(EComponentMobility::Static);

	EntranceTriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("Entrance"));
	EntranceTriggerVolume->SetupAttachment(RootComponent);
	EntranceTriggerVolume->SetHiddenInGame(true);
	EntranceTriggerVolume->SetMobility(EComponentMobility::Static);
	EntranceTriggerVolume->SetCollisionProfileName(FName("OverlapAll"));
	EntranceTriggerVolume->SetGenerateOverlapEvents(true);
	EntranceTriggerVolume->SetBoxExtent(FVector{ 50.0f, 150.0f, 50.0f });
	EntranceTriggerVolume->ShapeColor = FColor(0, 255, 0);
	EntranceTriggerVolume->SetRelativeLocation(FVector(50.f, 0.f, 0.f));

	ArrivalTriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("Arrival"));
	ArrivalTriggerVolume->SetupAttachment(RootComponent);
	ArrivalTriggerVolume->SetHiddenInGame(true);
	ArrivalTriggerVolume->SetMobility(EComponentMobility::Static);
	ArrivalTriggerVolume->SetCollisionProfileName(FName("OverlapAll"));
	ArrivalTriggerVolume->SetGenerateOverlapEvents(true);
	ArrivalTriggerVolume->SetBoxExtent(FVector{ 100.0f, 150.0f, 50.0f });
	ArrivalTriggerVolume->ShapeColor = FColor(0, 0, 255);
	ArrivalTriggerVolume->SetRelativeLocation(FVector(-100.f, 0.f, 0.f));

	ForwardArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("ForwardArrow"));
	ForwardArrow->SetupAttachment(RootComponent);
	ForwardArrow->SetHiddenInGame(true);
	ForwardArrow->SetMobility(EComponentMobility::Static);
	ForwardArrow->SetWorldScale3D(FVector{ 3.f, 3.f, 3.f });
	ForwardArrow->SetArrowColor(FLinearColor(0, 255, 0));
}

// Called when the game starts or when spawned
void AFork::BeginPlay()
{
	Super::BeginPlay();

	ArrivalTriggerVolume->SetCollisionProfileName(FName("OverlapAll"));
	ArrivalTriggerVolume->SetGenerateOverlapEvents(true);
	ArrivalTriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AFork::OnArrival);

	EntranceTriggerVolume->SetCollisionProfileName(FName("OverlapAll"));
	EntranceTriggerVolume->SetGenerateOverlapEvents(true);
	EntranceTriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AFork::OnEntrance);
}


#if WITH_EDITOR
void AFork::PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	UE_LOG(LogTemp, Warning, TEXT("Change Type: %d"), PropertyChangedEvent.ChangeType);
	if (!PropertyChangedEvent.Property)
	{
		UE_LOG(LogTemp, Warning, TEXT("AFork PropertyChangedEvent.Property is null!"));
		return;
	}
	else if (PropertyChangedEvent.GetPropertyName() != FName("Exits"))
	{
		UE_LOG(LogTemp, Warning, TEXT("AFork property %s changed!"), *(PropertyChangedEvent.GetPropertyName().ToString()));
		return;
	}

	// The index at which the array Exits is modified
	const auto index = PropertyChangedEvent.GetArrayIndex(FString("Exits"));
	//UE_LOG(LogTemp, Warning, TEXT("AFork PropertyChangedEvent.GetArrayIndex(): %d"), index);

	if (PropertyChangedEvent.ChangeType == EPropertyChangeType::ValueSet)
	{
		if (Exits[index] == nullptr)
		{
			RemoveLane(index); // Does nothing if Branches[index] == nullptr
		}
		else if (index == Exits.Find(Exits[index]) && index == Exits.FindLast(Exits[index]))
		{ // Exits[index] is valid and unique in Exits
			// Remove previously associated branch, if any
			RemoveLane(index); 
			// Associate a new branch
			AddLane(index); 
		}
		else // Exits[index] already exists at another location of array Exits
		{			
			Exits[index] = nullptr; // Avoid duplicate pointers in Exits
			RemoveLane(index);
		}
	}
	else if (PropertyChangedEvent.ChangeType == EPropertyChangeType::ArrayRemove)
	{
		// Destroy the corresponding fork branch
		RemoveLane(index);
		Lanes.RemoveAt(index);
	}
	else if (PropertyChangedEvent.ChangeType == EPropertyChangeType::ArrayClear)
	{
		for (int32 i = 0; i < Lanes.Num(); i++)
		{
			RemoveLane(i);
		}
		Lanes.Empty();
	}
	else if (PropertyChangedEvent.ChangeType == EPropertyChangeType::ArrayAdd) // Size of Exits incremented
	{
		// Increment the size of Branches
		Lanes.Add(nullptr);
	}
	else if (PropertyChangedEvent.ChangeType == EPropertyChangeType::Duplicate)
	{
		Exits.RemoveAt(index);
	}
}
#endif // WITH_EDITOR


void AFork::AddLane(int32 index)
{
	if (Exits[index] == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Attempted to add branch for a null exit!"));
		return;
	}
	FString LaneName = GetName() + "_to_" + Exits[index]->GetName();
	FActorSpawnParameters spawnParams;
	spawnParams.Name = FName(*LaneName);
	Lanes[index] = GetWorld()->SpawnActor<ALane>(spawnParams);
	Lanes[index]->Init(this, Exits[index]);
}

void AFork::RemoveLane(int32 index)
{
	if (Lanes[index] != nullptr)
	{
		Lanes[index]->Destroy();
		Lanes[index] = nullptr;
	}
}

//void AFork::OnConstruction(const FTransform &Transform)
//{
//	Super::OnConstruction(Transform);
//
//	UE_LOG(LogTemp, Warning, TEXT("AFork OnConstruction called!"));
//}

//void AFork::PostEditMove(bool bFinished)
//{
//	Super::PostEditMove(bFinished);
//
//	if (bFinished)
//	{
//		UE_LOG(LogTemp, Warning, TEXT("AFork PostEditMove called!"));
//		ConnectToMonitor();
//	}
//}


void AFork::OnArrival(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (MyMonitor == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("MyMonitor is null in AFork::OnEntrance!"));
		return;
	}
	int32 TimeStep = FMath::FloorToInt(GetWorld()->GetTimeSeconds() / MyMonitor->TimeResolution);
	FString EventMessage = "arrivesAtForkAtTime(_"
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
		switch (Signal) {
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
		EventMessage = "signalsDirectionAtForkAtTime(_"
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


void AFork::OnEntrance(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (MyMonitor != nullptr)
	{
		int32 TimeStep = FMath::FloorToInt(GetWorld()->GetTimeSeconds() / MyMonitor->TimeResolution);
		FString EventMessage = "entersForkAtTime(_" 
			+ OtherActor->GetName() + ", _" 
			+ GetName() + ", "
			+ FString::FromInt(TimeStep) + ").";
		UE_LOG(LogTemp, Warning, TEXT("%s"), *(EventMessage));
		MyMonitor->AddEvent(EventMessage);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MyMonitor is null in AFork::OnEntrance!"));
	}
}

void AFork::SetMonitor(AIntersectionMonitor* Monitor)
{
	MyMonitor = Monitor;
	for (ALane* Lane : Lanes)
	{
		if (Lane != nullptr)
		{
			Lane->SetMonitor(Monitor);
		}
	}
}