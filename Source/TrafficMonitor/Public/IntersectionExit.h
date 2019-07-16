// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// Engine's built-in
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"

// Developer
#include "IntersectionMonitor.h"

// Generated
#include "IntersectionExit.generated.h"

UCLASS()
class TRAFFICMONITOR_API AIntersectionExit : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AIntersectionExit(const FObjectInitializer &ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UFUNCTION()
		void OnExit(
			UPrimitiveComponent* OverlappedComp,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex);
public:
	UPROPERTY(EditAnywhere)
	UBoxComponent* TriggerVolume;

	UPROPERTY()
	UArrowComponent* ForwardArrow;

	UPROPERTY(EditAnywhere)
	AIntersectionMonitor* MyMonitor;

};
