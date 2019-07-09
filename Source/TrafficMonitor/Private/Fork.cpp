// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "Fork.h"


#include "Engine/CollisionProfile.h"

// Sets default values
AFork::AFork(const FObjectInitializer &ObjectInitializer)
	: Super(ObjectInitializer)
{
	RootComponent =
		ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("SceneRootComponent"));
	RootComponent->SetMobility(EComponentMobility::Static);

	EntranceTriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	EntranceTriggerVolume->SetupAttachment(RootComponent);
	EntranceTriggerVolume->SetHiddenInGame(true);
	EntranceTriggerVolume->SetMobility(EComponentMobility::Static);
	EntranceTriggerVolume->SetCollisionProfileName(FName("OverlapAll"));
	EntranceTriggerVolume->SetGenerateOverlapEvents(true);
	EntranceTriggerVolume->SetBoxExtent(FVector{ 40.0f, 100.0f, 50.0f });

	ForwardArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("ForwardArrow"));
	ForwardArrow->SetupAttachment(RootComponent);
	ForwardArrow->SetHiddenInGame(true);
	ForwardArrow->SetMobility(EComponentMobility::Static);
	ForwardArrow->SetWorldScale3D(FVector{ 2.f, 2.f, 2.f });
}

// Called when the game starts or when spawned
void AFork::BeginPlay()
{
	Super::BeginPlay();
	
}


#if WITH_EDITOR
void AFork::PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	UE_LOG(LogTemp, Warning, TEXT("Change Type: %d"), PropertyChangedEvent.ChangeType);
	if (!PropertyChangedEvent.Property)
	{
		UE_LOG(LogTemp, Warning, TEXT("PropertyChangedEvent.Property is null!"));
		return;
	}
	else if (PropertyChangedEvent.GetPropertyName() != FName("Exits"))
	{
		UE_LOG(LogTemp, Warning, TEXT("A property other than Exits changed!"));
		return;
	}

	// The index at which the array Exits is modified
	const auto index = PropertyChangedEvent.GetArrayIndex(FString("Exits"));
	UE_LOG(LogTemp, Warning, TEXT("PropertyChangedEvent.GetArrayIndex(): %d"), index);

	if (PropertyChangedEvent.ChangeType == EPropertyChangeType::ValueSet)
	{
		if (Exits[index] == nullptr)
		{
			RemoveBranch(index); // Does nothing if Branches[index] == nullptr
		}
		else if (index == Exits.Find(Exits[index]) && index == Exits.FindLast(Exits[index]))
		{
			AddBranch(index); // Exits[index] is valid and unique in Exits
		}
		else // Exit and its branch already exists
		{			
			Exits[index] = nullptr; // Avoid duplicate Exit pointers
			RemoveBranch(index);
		}
	}
	else if (PropertyChangedEvent.ChangeType == EPropertyChangeType::ArrayRemove)
	{
		// Destroy the corresponding fork branch
		RemoveBranch(index);
		Branches.RemoveAt(index);
	}
	else if (PropertyChangedEvent.ChangeType == EPropertyChangeType::ArrayAdd) // Size of Exits incremented
	{
		// Increment the size of Branches
		Branches.Add(nullptr);
	}
	else if (PropertyChangedEvent.ChangeType == EPropertyChangeType::Duplicate)
	{
		Exits.RemoveAt(index);
	}
}
#endif // WITH_EDITOR


void AFork::AddBranch(int32 index)
{
	if (Exits[index] == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Attempted to add branch for a null exit!"));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("Exits[%d] is set to %s"), index, *(Exits[index]->GetName()));
	Branches[index] = NewObject<UForkBranch>(this); // "this" is the owner of the new component
	Branches[index]->Init(RootComponent,
		GetActorLocation(),
		GetActorForwardVector(),
		Exits[index]->GetActorLocation(),
		Exits[index]->GetActorForwardVector());
}

void AFork::RemoveBranch(int32 index)
{
	if (Branches[index] != nullptr)
	{
		Branches[index]->DestroyComponent();
		Branches[index] = nullptr;
	}
}