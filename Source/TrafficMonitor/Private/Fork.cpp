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
		AddBranch(index);
	}
	else if (PropertyChangedEvent.ChangeType == EPropertyChangeType::ArrayRemove)
	{
		// Destroy the corresponding fork branch

		// Remove the index from the array Branches
		Branches.RemoveAt(index);
	}
	else if (PropertyChangedEvent.ChangeType == EPropertyChangeType::ArrayAdd)
	{
		// Increment the size of the array Branches
		Branches.Add(nullptr);
	}
}
#endif // WITH_EDITOR


void AFork::AddBranch(int32 index)
{
	// destroy the fork branch Lanes[index]
	// How?

	if (Exits[index] != nullptr)
		// instantiate the corresponding lane into Lanes[index]
	{
		UE_LOG(LogTemp, Warning, TEXT("Exits[%d] is set to %s"), index, *(Exits[index]->GetName()));
		Branches[index] = NewObject<UForkBranch>(this);
		Branches[index]->SetupAttachment(RootComponent);
		Branches[index]->SetHiddenInGame(true);
		Branches[index]->SetMobility(EComponentMobility::Static);
		Branches[index]->RegisterComponent();
		
		// setup the spline points (assuming two points on the spline)
		Branches[index]->SetLocationAtSplinePoint(0, GetActorLocation(), ESplineCoordinateSpace::World);
		Branches[index]->SetTangentAtSplinePoint(0, GetActorForwardVector()*1000.f, ESplineCoordinateSpace::World);
		Branches[index]->SetLocationAtSplinePoint(1, Exits[index]->GetActorLocation(), ESplineCoordinateSpace::World);
		Branches[index]->SetTangentAtSplinePoint(1, Exits[index]->GetActorForwardVector()*1000.f, ESplineCoordinateSpace::World);

	}

}