// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Runtime/Engine/Classes/Components/StaticMeshComponent.h"
#include "Runtime/Engine/Classes/Components/BillboardComponent.h"
#include "Runtime/Engine/Classes/Components/BoxComponent.h"

#include "IntersectionMonitor.generated.h"

UCLASS()
class TRAFFICMONITOR_API AIntersectionMonitor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AIntersectionMonitor(const FObjectInitializer &ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	void AddEvent(FString EventMessage);

public:
	UPROPERTY()
	UBillboardComponent* Billboard;

	UPROPERTY(EditAnywhere)
	UBoxComponent* ExtentBox;

	float TimeResolution = 0.5f;

private:
	void CreateLogFile();
	void AddToLoggers();
	void LogGeometry();

	FString FileName;
	FString AbsoluteFilePath;
};
