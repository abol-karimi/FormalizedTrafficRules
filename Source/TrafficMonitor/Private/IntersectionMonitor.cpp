// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "IntersectionMonitor.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "Runtime/Core/Public/Misc/Paths.h"

#include "Vehicle/WheeledVehicleAIController.h"


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

	SetupTriggers();

	RecordGeometry();
}


void AIntersectionMonitor::SetupTriggers()
{
	ExtentBox->OnComponentEndOverlap.AddDynamic(this, &AIntersectionMonitor::OnExitMonitor);

	TArray<AActor *> OverlappingActors;
	GetOverlappingActors(OverlappingActors, AFork::StaticClass());
	for (AActor* Actor : OverlappingActors)
	{
		AFork* Fork = Cast<AFork>(Actor);
		if (Fork != nullptr)
		{
			Fork->ArrivalTriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AIntersectionMonitor::OnArrival);
			Fork->EntranceTriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AIntersectionMonitor::OnEntrance);
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


void AIntersectionMonitor::LogGeometry()
{
	//Creates an instance of ofstream, and opens for appending
	std::ofstream LogFile(TCHAR_TO_UTF8(*AbsoluteFilePath), std::ios::app);

	LogFile << "#program geometry." << std::endl;

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
	NumberOfForks = Forks.Num();

	// "isToTheRightOf()" facts
	for (size_t i = 0; i < NumberOfForks; i++)
	{
		for (size_t j = i + 1; j < NumberOfForks; j++)
		{
			if (Forks[i]->IsToTheRightOf(Forks[j])) // angle in (30, 150)
			{
				FString FactString = "isToTheRightOf(_"
					+ Forks[i]->GetName() + ", _"
					+ Forks[j]->GetName() + ").";
				LogFile << TCHAR_TO_UTF8(*FactString) << std::endl;
			}
			else if (Forks[j]->IsToTheRightOf(Forks[i])) // angle in (-150, -30)
			{
				FString FactString = "isToTheRightOf(_"
					+ Forks[j]->GetName() + ", _"
					+ Forks[i]->GetName() + ").";
				LogFile << TCHAR_TO_UTF8(*FactString) << std::endl;
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
		FString FactString = "laneFromTo(_"
			+ Lane->GetName() + ", _"
			+ Lane->MyFork->GetName() + ", _"
			+ Lane->MyExit->GetName() + ").";
		LogFile << TCHAR_TO_UTF8(*FactString) << std::endl;
	}

	// Lane overlaps
	for (size_t i = 0; i < Lanes.Num(); i++)
	{
		for (size_t j = i + 1; j < Lanes.Num(); j++)
		{
			if (Lanes[i]->IsOverlappingActor(Lanes[j]))
			{
				FString FactString = "overlaps(_"
					+ Lanes[i]->GetName() + ", _"
					+ Lanes[j]->GetName() + ").";
				LogFile << TCHAR_TO_UTF8(*FactString) << std::endl;
				FactString = "overlaps(_"
					+ Lanes[j]->GetName() + ", _"
					+ Lanes[i]->GetName() + ").";
				LogFile << TCHAR_TO_UTF8(*FactString) << std::endl;
			}
		}
	}

	// Close the file stream explicitly
	LogFile.close();
}


void AIntersectionMonitor::RecordGeometry()
{
	//Geometry += "#program geometry.\n";

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
	NumberOfForks = Forks.Num();

	// "isToTheRightOf()" facts
	for (size_t i = 0; i < NumberOfForks; i++)
	{
		for (size_t j = i + 1; j < NumberOfForks; j++)
		{
			FString LeftFork = "f_";
			FString RightFork = "f_";
			if (Forks[i]->IsToTheRightOf(Forks[j])) // angle in (30, 150)
			{
				LeftFork += Forks[j]->GetName();
				RightFork += Forks[i]->GetName();
			}
			else if (Forks[j]->IsToTheRightOf(Forks[i])) // angle in (-150, -30)
			{
				LeftFork += Forks[i]->GetName();
				RightFork += Forks[j]->GetName();
			}
			FString FactString = "isToTheRightOf("
				+ RightFork + ", "
				+ LeftFork + ").";
			Geometry += FactString + "\n";
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
		FString FactString = "laneFromTo("
			+ Lane->GetName().ToLower() + ", "
			+ Lane->MyFork->GetName().ToLower() + ", "
			+ Lane->MyExit->GetName().ToLower() + ").";
		Geometry += FactString + "\n";
	}

	// Lane overlaps
	for (size_t i = 0; i < Lanes.Num(); i++)
	{
		for (size_t j = i + 1; j < Lanes.Num(); j++)
		{
			if (Lanes[i]->IsOverlappingActor(Lanes[j]))
			{
				FString FactString = "overlaps(_"
					+ Lanes[i]->GetName().ToLower() + ", _"
					+ Lanes[j]->GetName().ToLower() + ").";
				Geometry += FactString + "\n";
				FactString = "overlaps(_"
					+ Lanes[j]->GetName().ToLower() + ", _"
					+ Lanes[i]->GetName().ToLower() + ").";
				Geometry += FactString + "\n";
			}
		}
	}
}


void AIntersectionMonitor::AddEvent(FString Actor, FString Atom, uint32 TimeStep)
{
	// TODO: Use TimeStep to buffer concurrent events
	ActorToEventsMap.Add(Actor, Atom);	
}


void AIntersectionMonitor::AddExitEvent(FString Actor, FString Atom, uint32 TimeStep)
{
	ActorToEventsMap.Remove(Actor);
	Solve();
}


void AIntersectionMonitor::LogEvent(FString EventMessage)
{
	std::ofstream LogFile(TCHAR_TO_UTF8(*AbsoluteFilePath), std::ios::app);
	LogFile << TCHAR_TO_UTF8(*EventMessage) << std::endl;
	LogFile.close();
}


void AIntersectionMonitor::OnArrival(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	int32 TimeStep = FMath::FloorToInt(GetWorld()->GetTimeSeconds() / TimeResolution);
	FString ArrivingVehicleID = "v_" + OtherActor->GetName();
	FString Fork = "f_" + OverlappedComp->GetOwner()->GetName();
	FString EventAtom = "arrivesAtForkAtTime("
		+ ArrivingVehicleID + ", "
		+ Fork + ", "
		+ FString::FromInt(TimeStep) + ").";
	ActorToEventsMap.Add(OtherActor->GetName(), EventAtom);

	ACarlaWheeledVehicle* ArrivingVehicle = Cast<ACarlaWheeledVehicle>(OtherActor);
	if (ArrivingVehicle != nullptr)
	{
		FString SignalString = ArrivingVehicle->GetSignalString();
		EventAtom = "signalsDirectionAtForkAtTime("
			+ ArrivingVehicleID + ", "
			+ SignalString + ", "
			+ Fork + ", "
			+ FString::FromInt(TimeStep) + ").";
		ActorToEventsMap.Add(OtherActor->GetName(), EventAtom);
		VehiclePointers.Add(OtherActor->GetName(), ArrivingVehicle); // TODO: remove pointer in OnExit
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Cast to ACarlaWheeledVehicle failed!"));
	}

	Solve();
}


void AIntersectionMonitor::OnEntrance(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	uint32 TimeStep = FMath::FloorToInt(GetWorld()->GetTimeSeconds() / TimeResolution);
	FString EnteringVehicle = "v_" + OtherActor->GetName();
	FString Fork = "f_" + OverlappedComp->GetOwner()->GetName();
	FString Atom = "entersForkAtTime("
		+ EnteringVehicle + ", "
		+ Fork + ", "
		+ FString::FromInt(TimeStep) + ").";

	ActorToEventsMap.Add(OtherActor->GetName(), Atom);
	Solve();
}


void AIntersectionMonitor::OnExitMonitor(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	ActorToEventsMap.Remove(OtherActor->GetName());
	Solve();
}


void AIntersectionMonitor::Solve()
{
	try {
		Clingo::Logger logger = [](Clingo::WarningCode, char const *message) {
			//UE_LOG(LogTemp, Warning, TEXT("Clingo logger message: %s"), ANSI_TO_TCHAR(message));
		};

		Clingo::Control ctl{ {}, logger, 20 };

		char* GeometryString = TCHAR_TO_UTF8(*Geometry);
		ctl.add("base", {}, GeometryString);

		for (auto& Pair : ActorToEventsMap)
		{
			char* Atom = TCHAR_TO_UTF8(*Pair.Value);
			//UE_LOG(LogTemp, Warning, TEXT("Adding to the program: %s"), *Pair.Value);
			ctl.add("base", {}, Atom);
		}

		const char* RulesFileName = TCHAR_TO_UTF8(*(FPaths::ProjectDir() + "Plugins/TrafficMonitor/LogicSolver/all-way-stop_new.cl"));
		ctl.load(RulesFileName);

		ctl.ground({ {"base", {}} });
		auto solveHandle = ctl.solve();
		for (auto &model : solveHandle) {
			FString Model;
			for (auto &atom : model.symbols()) {
				if (atom.match("mustYieldToForRule", 3))
				{
					FString YieldingVehicleName = FString(atom.arguments()[0].name()).RightChop(2);
					ACarlaWheeledVehicle* YieldingVehicle = VehiclePointers[YieldingVehicleName];
					AWheeledVehicleAIController* Controller = Cast<AWheeledVehicleAIController>(YieldingVehicle->GetController());
					if (Controller != nullptr)
					{
						Controller->SetTrafficLightState(ETrafficLightState::Red);
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("%s's controller not found! (mustYield)"), *YieldingVehicleName);
					}
				}
				else if (atom.match("hasRightOfWay", 1))
				{
					FString VehicleName = FString(atom.arguments()[0].name()).RightChop(2);
					ACarlaWheeledVehicle* Vehicle = VehiclePointers[VehicleName];
					AWheeledVehicleAIController* Controller = Cast<AWheeledVehicleAIController>(Vehicle->GetController());
					if (Controller != nullptr)
					{
						Controller->SetTrafficLightState(ETrafficLightState::Green);
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("%s's controller not found! (hasRightOfWay)"), *VehicleName);
					}					
				}
				FString AtomString(atom.to_string().c_str());
				Model.Append("\t" + AtomString + "\n");
			}
			UE_LOG(LogTemp, Error, TEXT("Clingo Model:\n%s\n"), *Model);

		}
	}
	catch (std::exception const &e) {
		UE_LOG(LogTemp, Warning, TEXT("Clingo failed with: %s"), ANSI_TO_TCHAR(e.what()));
	}

}