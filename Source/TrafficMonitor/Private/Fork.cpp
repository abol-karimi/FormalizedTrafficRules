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

	ArrivalTriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	ArrivalTriggerVolume->SetupAttachment(RootComponent);
	ArrivalTriggerVolume->SetHiddenInGame(true);
	ArrivalTriggerVolume->SetMobility(EComponentMobility::Static);
	ArrivalTriggerVolume->SetCollisionProfileName(FName("OverlapAll"));
	ArrivalTriggerVolume->SetGenerateOverlapEvents(true);
	ArrivalTriggerVolume->SetBoxExtent(FVector{ 40.0f, 100.0f, 50.0f });

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

// Called every frame
void AFork::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

//#if WITH_EDITOR
void AFork::PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	UE_LOG(LogTemp, Warning, TEXT("Change Type: %d"), PropertyChangedEvent.ChangeType);
	if (PropertyChangedEvent.Property && PropertyChangedEvent.ChangeType == EPropertyChangeType::ArrayAdd)
	{
		const auto NewIndex = Lanes.Num() - 1;
		UE_LOG(LogTemp, Warning, TEXT("NewIndex: %d"), NewIndex);
		Lanes[NewIndex] = NewObject<UForkBranch>(this);
		Lanes[NewIndex]->SetupAttachment(RootComponent);
		Lanes[NewIndex]->SetHiddenInGame(true);
		Lanes[NewIndex]->SetMobility(EComponentMobility::Static);
		Lanes[NewIndex]->RegisterComponent();
	}

	//const auto Size = Lanes.Num();
	//if (PropertyChangedEvent.Property)
	//{
	//	for (auto i = 0; i < Size; ++i)
	//	{
	//		if (Lanes[i] == nullptr)
	//		{
	//			Lanes[i] = NewObject<USplineComponent>(this);
	//			Lanes[i]->SetupAttachment(RootComponent);
	//			Lanes[i]->SetHiddenInGame(true);
	//			Lanes[i]->SetMobility(EComponentMobility::Static);
	//			Lanes[i]->RegisterComponent();
	//		}
	//	}
	//	UE_LOG(LogTemp, Warning, TEXT("Property changed: %s"), *(PropertyChangedEvent.Property->GetFName().ToString()));
	//	UE_LOG(LogTemp, Warning, TEXT("ArrayIndex: %d"), PropertyChangedEvent.GetArrayIndex(PropertyChangedEvent.Property->GetFName().ToString()));
	//}

}


//#endif // WITH_EDITOR