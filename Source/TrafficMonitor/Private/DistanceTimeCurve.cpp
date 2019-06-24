// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#include "DistanceTimeCurve.h"

// Sets default values for this component's properties
UDistanceTimeCurve::UDistanceTimeCurve()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UDistanceTimeCurve::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UDistanceTimeCurve::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


void UDistanceTimeCurve::AddKey(float InTime, float InDistance)
{
	FKeyHandle KeyHandle = RichCurve.AddKey(InTime, InDistance);
	//RichCurve.SetKeyInterpMode(KeyHandle, ERichCurveInterpMode::RCIM_Cubic);
	//RichCurve.SetKeyTangentMode(KeyHandle, ERichCurveTangentMode::RCTM_Auto);
	//RichCurve.SetKeyTangentWeightMode(KeyHandle, ERichCurveTangentWeightMode::RCTWM_WeightedBoth);
}

float UDistanceTimeCurve::Eval(float InTime)
{
	return RichCurve.Eval(InTime);
}

