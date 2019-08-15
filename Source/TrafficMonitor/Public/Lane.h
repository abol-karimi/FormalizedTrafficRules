// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Exit.h"

// carla
#include "Vehicle/CarlaWheeledVehicle.h"

// Generated
#include "Lane.generated.h"

UCLASS()
class TRAFFICMONITOR_API ALane : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALane(const FObjectInitializer &ObjectInitializer);

protected:
	virtual void OnConstruction(const FTransform &Transform) override;

public:	
	void Init(class AFork* MyFork, AExit* MyExit);
	void SetupSpline();
	void SetupSplineMeshes();
	FString GetCorrectSignal();

public:
	UPROPERTY(VisibleAnywhere)
	class AFork* MyFork = nullptr;

	UPROPERTY(VisibleAnywhere)
	AExit* MyExit = nullptr;

	UPROPERTY()
	AIntersectionMonitor* MyMonitor = nullptr;

	UPROPERTY(EditAnywhere)
	USplineComponent* Spline = nullptr;

	UPROPERTY()
	TArray<USplineMeshComponent*> SplineMeshComponents;

	UPROPERTY(EditAnywhere)
	UStaticMesh* Mesh;

	UPROPERTY(EditAnywhere)
	UMaterial* Material;

	UPROPERTY(EditAnywhere)
	float MaxMeshLength = 1.0f; // in meters

private:
	bool MinimumCurvatureVariation(
		FVector2D p0, 
		FVector2D p1, 
		FVector2D d0, 
		FVector2D d1, 
		float& OutAlpha0, 
		float& OutAlpha1);
};
