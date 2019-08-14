// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// Engine built-in
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"

// Developer
#include "IntersectionExit.h"
#include "Lane.h"
#include "IntersectionMonitor.h"

// Generated
#include "Fork.generated.h"

UCLASS()
class TRAFFICMONITOR_API AFork : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFork(const FObjectInitializer &ObjectInitializer);

//	virtual void OnConstruction(const FTransform &Transform) override;
//	virtual void PostEditMove(bool bFinished) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent) override;
#endif // WITH_EDITOR

public:
	void AddLane(int32 index);
	void RemoveLane(int32 index);
	bool IsToTheRightOf(const AFork* OtherFork) const;

public:	
	UPROPERTY(EditAnywhere)
	UBoxComponent* EntranceTriggerVolume;

	UPROPERTY(EditAnywhere)
	UBoxComponent* ArrivalTriggerVolume;
	
	UPROPERTY()
	UArrowComponent* ForwardArrow;

	UPROPERTY(EditAnywhere, meta=(NoElementDuplicate))
	TArray<AIntersectionExit*> Exits;

	UPROPERTY(VisibleAnywhere)
	TArray<ALane*> Lanes;
};
