// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "Components/BoxComponent.h"

#include "ForkBranch.generated.h"

/**
 * 
 */
UCLASS()
class TRAFFICMONITOR_API UForkBranch : public USplineComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	UForkBranch();

protected:
	virtual void PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent) override;

public:
	UPROPERTY(EditAnywhere)
	UBoxComponent* ExitTriggerVolume;
};
