// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"

#include "EntranceTrigger.generated.h"

/**
 * 
 */
UCLASS()
class TRAFFICMONITOR_API UEntranceTrigger : public UBoxComponent
{
	GENERATED_BODY()
	
public:
	// Sets default values for this component's properties
	UEntranceTrigger();

	UPROPERTY(EditAnywhere)
	UArrowComponent* ForwardArrow;
};