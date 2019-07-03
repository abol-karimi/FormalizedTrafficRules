// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/SplineComponent.h"

#include "Fork.generated.h"

UCLASS()
class TRAFFICMONITOR_API AFork : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFork(const FObjectInitializer &ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent) override;
#endif // WITH_EDITOR

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
	UBoxComponent* ArrivalTriggerVolume;

	UPROPERTY(EditAnywhere)
	UArrowComponent* ForwardArrow;

	UPROPERTY(BlueprintReadWrite, Category = "Fork Lanes", EditAnywhere)
	TArray<USplineComponent *> Lanes;

	UPROPERTY(VisibleAnywhere, Category = "Fork Lanes", EditFixedSize)
	TArray<UBoxComponent *> Exits;
};
