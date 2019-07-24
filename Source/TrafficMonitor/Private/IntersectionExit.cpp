// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "IntersectionExit.h"

// Sets default values
AIntersectionExit::AIntersectionExit(const FObjectInitializer &ObjectInitializer)
	: Super(ObjectInitializer)
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
	TriggerVolume->SetBoxExtent(FVector{ 20.0f, 150.f, 50.0f });
	TriggerVolume->ShapeColor = FColor(255, 0, 0);

	ForwardArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("ForwardArrow"));
	ForwardArrow->SetupAttachment(RootComponent);
	ForwardArrow->SetHiddenInGame(true);
	ForwardArrow->SetMobility(EComponentMobility::Static);
	ForwardArrow->SetWorldScale3D(FVector{ 3.f, 3.f, 3.f });
}

// Called when the game starts or when spawned
void AIntersectionExit::BeginPlay()
{
	Super::BeginPlay();
	
	TriggerVolume->SetCollisionProfileName(FName("OverlapAll"));
	TriggerVolume->SetGenerateOverlapEvents(true);
	TriggerVolume->OnComponentEndOverlap.AddDynamic(this, &AIntersectionExit::OnExit);
}

void AIntersectionExit::OnExit(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (OtherActor != nullptr)
	{
		if (MyMonitor != nullptr)
		{
			int32 TimeStep = FMath::FloorToInt(GetWorld()->GetTimeSeconds() / MyMonitor->TimeResolution);
			FString EventMessage = "exitsIntersectionAtTime(_" 
				+ OtherActor->GetFName().ToString() + ", " 
				+ FString::FromInt(TimeStep) + ").";
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
		UE_LOG(LogTemp, Warning, TEXT("OtherActor is null in AIntersectionExit::OnExit!"));
	}
}