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
	// Sets default values for this component's properties
	UForkBranch();

protected:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent) override;
#endif // WITH_EDITOR

	virtual void BeginPlay() override;


public:
	UFUNCTION()
	void OnExitFromLane(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex, 
		bool bFromSweep, 
		const FHitResult& SweepResult);
//public:
//	UPROPERTY(EditAnywhere)
//	UBoxComponent* ExitTriggerVolume;
};
