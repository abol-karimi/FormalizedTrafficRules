// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "EntranceTrigger.h"


UEntranceTrigger::UEntranceTrigger()
{
	UE_LOG(LogTemp, Warning, TEXT("EntranceTrigger constructor!"));

	ForwardArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("EntranceDirection"));
	ForwardArrow->SetupAttachment(this);
	ForwardArrow->SetHiddenInGame(true);
	ForwardArrow->SetMobility(EComponentMobility::Static);
	ForwardArrow->RegisterComponent();
}

