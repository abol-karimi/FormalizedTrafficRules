// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "IntersectionMonitor.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "Runtime/Core/Public/Misc/Paths.h"


THIRD_PARTY_INCLUDES_START

#pragma push_macro("check")

#undef check

#include <clingo.hh>

#pragma pop_macro("check")

THIRD_PARTY_INCLUDES_END


// Developer
#include "Fork.h"

#include <fstream>
#include <iostream>

// Sets default values
AIntersectionMonitor::AIntersectionMonitor(const FObjectInitializer &ObjectInitializer)
	:Super(ObjectInitializer)
{
	RootComponent =
		ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("SceneRootComponent"));
	RootComponent->SetMobility(EComponentMobility::Static);

	static ConstructorHelpers::FObjectFinder<UTexture2D> MonitorBillboardAsset(TEXT("Texture2D'/TrafficMonitor/Monitor.Monitor'"));
	Billboard = ObjectInitializer.CreateEditorOnlyDefaultSubobject<UBillboardComponent>(this, TEXT("Billboard"), true);
	if (MonitorBillboardAsset.Object != nullptr && Billboard != nullptr)
	{
		Billboard->SetSprite(MonitorBillboardAsset.Object);
		Billboard->SetupAttachment(RootComponent);
		Billboard->SetRelativeLocation(FVector{ 0.f, 0.f, 300.f });
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to instantiate billboard or its sprite!"));
	}

	ExtentBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	ExtentBox->SetupAttachment(RootComponent);
	ExtentBox->SetHiddenInGame(true);
	ExtentBox->SetMobility(EComponentMobility::Static);
	ExtentBox->SetCollisionProfileName(FName("OverlapAll"));
	ExtentBox->SetGenerateOverlapEvents(true);
	ExtentBox->SetBoxExtent(FVector{ 1800.0f, 1800.0f, 100.0f });
	ExtentBox->ShapeColor = FColor(255, 255, 255);
}

// Called when the game starts or when spawned
void AIntersectionMonitor::BeginPlay()
{
	Super::BeginPlay();

	CreateLogFile();

	AddToLoggers();

	LogGeometry();

	try {
		Clingo::Logger logger = [](Clingo::WarningCode, char const *message) {
			UE_LOG(LogTemp, Warning, TEXT("Clingo logger message: %s"), message);
		};
	
		Clingo::Control ctl{ {}, logger, 20 };		
		ctl.add("base", {}, "a :- not b. b :- not a.");
		ctl.ground({ {"base", {}} });
		auto solveHandle = ctl.solve();
		for (auto &m : solveHandle) {
			for (auto &atom : m.symbols()) {
				FString AtomString(atom.to_string().c_str());
				UE_LOG(LogTemp, Warning, TEXT("Clingo: Model: %s"), *AtomString);
			}
		}
	}
	catch (std::exception const &e) {
		UE_LOG(LogTemp, Warning, TEXT("Clingo failed with: %s"), e.what());
	}

}


void AIntersectionMonitor::AddToLoggers()
{
	TArray<AActor *> OverlappingActors;
	GetOverlappingActors(OverlappingActors, AFork::StaticClass());
	for (AActor* Actor : OverlappingActors)
	{
		AFork* Fork = Cast<AFork>(Actor);
		if (Fork != nullptr)
		{
			Fork->SetMonitor(this);
			//UE_LOG(LogTemp, Warning, TEXT("%s connected to a logger!"), *(Fork->GetName()));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Could not connect: Overlapping actor was null!"));
		}
	}
}


void AIntersectionMonitor::CreateLogFile()
{
	// Init logfile name and path
	FileName = GetName() + ".cl";
	AbsoluteFilePath = FPaths::ProjectSavedDir() + this->FileName;

	// Create the logfile 
	std::ofstream LogFile(std::string(TCHAR_TO_UTF8(*AbsoluteFilePath)), std::ios::trunc);

	// Close the file stream explicitly
	LogFile.close();
}


void AIntersectionMonitor::AddEvent(FString EventMessage)
{
	//Creates an instance of ofstream, and opens 
	std::ofstream LogFile(TCHAR_TO_UTF8(*AbsoluteFilePath), std::ios::app);
	// Outputs to example.txt through a_file
	LogFile << TCHAR_TO_UTF8(*EventMessage) << std::endl;
	// Close the file stream explicitly
	LogFile.close();
}


void AIntersectionMonitor::LogGeometry()
{
	// Get a list of Forks
	TArray<AActor *> OverlappingActors;
	GetOverlappingActors(OverlappingActors);
	TArray<AFork *> Forks;
	for (AActor* OverlappingActor : OverlappingActors)
	{
		AFork* Fork = Cast<AFork>(OverlappingActor);
		if (Fork != nullptr)
		{
			Forks.Add(Fork);
		}
	}

	// "isToTheRightOf()" facts
	for (int i = 0; i < Forks.Num(); i++)
	{
		for (int j = i + 1; j < Forks.Num(); j++)
		{
			if (Forks[i]->IsToTheRightOf(Forks[j])) // angle in (30, 150)
			{
				FString EventMessage = "isToTheRightOf(_"
					+ Forks[i]->GetName() + ", _"
					+ Forks[j]->GetName() + ").";
				AddEvent(EventMessage);
			}
			else if (Forks[j]->IsToTheRightOf(Forks[i])) // angle in (-150, -30)
			{
				FString EventMessage = "isToTheRightOf(_"
					+ Forks[j]->GetName() + ", _"
					+ Forks[i]->GetName() + ").";
				AddEvent(EventMessage);
			}
		}
	}

	// Get a list of Lanes
	TArray<ALane *> Lanes;
	for (AActor* OverlappingActor : OverlappingActors)
	{
		ALane* Lane = Cast<ALane>(OverlappingActor);
		if (Lane != nullptr)
		{
			Lanes.Add(Lane);
		}
	}

	// Graph connectivity
	for (ALane* Lane : Lanes)
	{
		FString EventMessage = "laneFromTo(_"
			+ Lane->GetName() + ", _"
			+ Lane->MyFork->GetName() + ", _"
			+ Lane->MyExit->GetName() + ").";
		AddEvent(EventMessage);
	}

	// Lane overlaps
	for (int i = 0; i < Lanes.Num(); i++)
	{
		for (int j = i + 1; j < Lanes.Num(); j++)
		{
			if (Lanes[i]->IsOverlappingActor(Lanes[j]))
			{
				FString EventMessage = "overlaps(_"
					+ Lanes[i]->GetName() + ", _"
					+ Lanes[j]->GetName() + ").";
				AddEvent(EventMessage);
				EventMessage = "overlaps(_"
					+ Lanes[j]->GetName() + ", _"
					+ Lanes[i]->GetName() + ").";
				AddEvent(EventMessage);
			}
		}
	}


}