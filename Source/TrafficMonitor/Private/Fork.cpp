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
	EntranceTriggerVolume->SetBoxExtent(FVector{ 40.0f, 160.0f, 50.0f });
	EntranceTriggerVolume->ShapeColor = FColor(0, 255, 0);

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
			RemoveBranch(index); // Does nothing if Branches[index] == nullptr
		}
		else if (index == Exits.Find(Exits[index]) && index == Exits.FindLast(Exits[index]))
		{ // Exits[index] is valid and unique in Exits
			// Remove previously associated branch, if any
			RemoveBranch(index); 
			// Associate a new branch
			AddBranch(index); 
		}
		else // Exits[index] already exists at another location of array Exits
		{			
			Exits[index] = nullptr; // Avoid duplicate pointers in Exits
			RemoveBranch(index);
		}
	}
	else if (PropertyChangedEvent.ChangeType == EPropertyChangeType::ArrayRemove)
	{
		// Destroy the corresponding fork branch
		RemoveBranch(index);
		Branches.RemoveAt(index);
	}
	else if (PropertyChangedEvent.ChangeType == EPropertyChangeType::ArrayClear)
	{
		for (int32 index = 0; index < Branches.Num(); index++)
		{
			RemoveBranch(index);
		}
		Branches.Empty();
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
	Branches[index]->Init(RootComponent, Exits[index]);
}

void AFork::RemoveBranch(int32 index)
{
	if (Branches[index] != nullptr)
	{
		Branches[index]->DestroyComponent(true);
		Branches[index] = nullptr;
	}
}

//void AFork::OnConstruction(const FTransform &Transform)
//{
//	Super::OnConstruction(Transform);
//
//	UE_LOG(LogTemp, Warning, TEXT("AFork OnConstruction called!"));
//}


void AFork::OnEntrance(
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
			FString EventMessage = "entersIntersectionAtTime(" + OtherActor->GetName() + ", " + FString::FromInt(TimeStep) + ").";
			UE_LOG(LogTemp, Warning, TEXT("%s"), *(EventMessage));
			MyMonitor->AddEvent(EventMessage);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("MyMonitor is null in AFork::OnEntrance!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("OtherActor is null in AFork::OnEntrance!"));
	}
}