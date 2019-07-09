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
	TriggerVolume->SetBoxExtent(FVector{ 20.0f, 150.0f, 50.0f });
	TriggerVolume->ShapeColor = FColor(255, 0, 0);

	ForwardArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("ForwardArrow"));
	ForwardArrow->SetupAttachment(RootComponent);
	ForwardArrow->SetHiddenInGame(true);
	ForwardArrow->SetMobility(EComponentMobility::Static);
	ForwardArrow->SetWorldScale3D(FVector{ 2.f, 2.f, 2.f });
}

// Called when the game starts or when spawned
void AIntersectionExit::BeginPlay()
{
	Super::BeginPlay();
	
}

