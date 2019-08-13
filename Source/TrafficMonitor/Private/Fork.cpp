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
	EntranceTriggerVolume->SetHiddenInGame(false);
	EntranceTriggerVolume->SetMobility(EComponentMobility::Static);
	EntranceTriggerVolume->SetCollisionProfileName(FName("OverlapAll"));
	EntranceTriggerVolume->SetGenerateOverlapEvents(true);
	EntranceTriggerVolume->SetBoxExtent(FVector{ 50.0f, 150.0f, 50.0f });
	EntranceTriggerVolume->ShapeColor = FColor(0, 255, 0);
	EntranceTriggerVolume->SetRelativeLocation(FVector(50.f, 0.f, 0.f));

	ArrivalTriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("Arrival"));
	ArrivalTriggerVolume->SetupAttachment(RootComponent);
	ArrivalTriggerVolume->SetHiddenInGame(false);
	ArrivalTriggerVolume->SetMobility(EComponentMobility::Static);
	ArrivalTriggerVolume->SetCollisionProfileName(FName("OverlapAll"));
	ArrivalTriggerVolume->SetGenerateOverlapEvents(true);
	ArrivalTriggerVolume->ShapeColor = FColor(0, 0, 255);
	ArrivalTriggerVolume->SetBoxExtent(FVector{ 180.0f, 150.0f, 50.0f });
	ArrivalTriggerVolume->SetRelativeLocation(FVector(-180.f, 0.f, 0.f));

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

	EntranceTriggerVolume->SetCollisionProfileName(FName("OverlapAll"));
	EntranceTriggerVolume->SetGenerateOverlapEvents(true);
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


/// Formalization of "isToTheRightOf()" based on approaching angles of forks
bool AFork::IsToTheRightOf(const AFork* OtherFork) const
{
	auto Ego = FVector2D(GetActorForwardVector());
	auto Other = FVector2D(OtherFork->GetActorForwardVector());
	float Sine = FVector2D::CrossProduct(Ego, Other); // Ego and Other are unit vectors
	if (Sine > 0.5f) // angle in (30, 150) degrees
	{
		return true;
	}
	// if Sine < -0.5 then IsToTheLeftOf(OtherFork)
	// if Sine in [-0.5, 0.5] and Cosine > 0 then heading the same direction
	// if Sine in [-0.5, 0.5] and Cosine < 0 then heading the opposite direction
	return false;
}
