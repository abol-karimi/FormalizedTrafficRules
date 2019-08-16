// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// Engine built-in
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"

// Developer
#include "Exit.h"
#include "Lane.h"
#include "IntersectionMonitor.h"

// Generated
#include "Fork.generated.h"

USTRUCT()
struct FExitCheckbox
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	AExit* Exit;

	UPROPERTY(EditAnywhere)
	bool bActive;

	UPROPERTY()
	ALane* Lane;

	FExitCheckbox(AExit* Exit=nullptr)
	{
		this->Exit = Exit;
		bActive = false;
		Lane = nullptr;
	}
};

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
	bool IsToTheRightOf(const AFork* OtherFork) const;
	void AddExit(AExit* Exit);

public:	
	UPROPERTY(EditAnywhere)
	UBoxComponent* EntranceTriggerVolume;

	UPROPERTY(EditAnywhere)
	UBoxComponent* ArrivalTriggerVolume;
	
	UPROPERTY()
	UArrowComponent* ForwardArrow;

	UPROPERTY(EditAnywhere, EditFixedSize)
	TArray<FExitCheckbox> Exits;
};
