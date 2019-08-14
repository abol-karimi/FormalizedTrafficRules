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
	void AddEvent(FString Actor, FString Atom);
	FString GetEventsString();

	UFUNCTION()
	void OnArrival(
			UPrimitiveComponent* OverlappedComp,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex,
			bool bFromSweep,
			const FHitResult& SweepResult);

	UFUNCTION()
	void OnEntrance(
			UPrimitiveComponent* OverlappedComp,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex,
			bool bFromSweep,
			const FHitResult& SweepResult);

	UFUNCTION()
	void OnExitMonitor(
			UPrimitiveComponent* OverlappedComp,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex);

	UFUNCTION()
	void OnEnterLane(AActor* ThisActor, AActor* OtherActor);

	UFUNCTION()
	void OnExitLane(AActor* ThisActor, AActor* OtherActor);

public:
	UPROPERTY()
	UBillboardComponent* Billboard;

	UPROPERTY(EditAnywhere)
	UBoxComponent* ExtentBox;

	float TimeResolution = 0.5f; // Events occuring in the same 0.5 seconds interval are simultaneous

private:
	void CreateLogFile();
	void SetupTriggers();
	void LoadGeometryFacts();
	void WriteGeometryToFile();
	void AppendToLogfile(FString EventMessage);
	void Solve();
	
	FString LogFileName;
	FString LogFileFullName;
	size_t NumberOfForks;

	TMap<FString, FString> ActorToEventsMap;

	FString Geometry;
	
	TMap<FString, class ACarlaWheeledVehicle*> VehiclePointers;
};
