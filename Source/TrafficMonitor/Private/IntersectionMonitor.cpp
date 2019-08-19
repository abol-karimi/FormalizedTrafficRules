// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "IntersectionMonitor.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "Runtime/Core/Public/Misc/Paths.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

// Carla
#include "Vehicle/WheeledVehicleAIController.h"


// Clingo staticly linked library
THIRD_PARTY_INCLUDES_START
#pragma push_macro("check")
#undef check
#include <clingo.hh>
#pragma pop_macro("check")
THIRD_PARTY_INCLUDES_END


// Developer
#include "Fork.h"


// STL
#include <fstream>
#include <sstream>


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


void AIntersectionMonitor::OnConstruction(const FTransform &Transform)
{
	TArray<AExit*> MyExits;
	GetIntersectingActors<AExit>(MyExits);
	TArray<AFork*> MyForks;
	GetIntersectingActors<AFork>(MyForks);
	for (AFork* Fork : MyForks)
	{
		for (AExit* Exit : MyExits)
		{
			Fork->AddExit(Exit);
		}
	}
}


// Called when the game starts or when spawned
void AIntersectionMonitor::BeginPlay()
{
	Super::BeginPlay();

	CreateLogFile();

	SetupTriggers();

	LoadGeometryFacts();
	WriteGeometryToFile();

	LoadTrafficRules();
}


void AIntersectionMonitor::SetupTriggers()
{
	ExtentBox->OnComponentEndOverlap.AddDynamic(this, &AIntersectionMonitor::OnExitMonitor);

	TArray<AActor *> OverlappingActors;
	GetOverlappingActors(OverlappingActors);
	for (AActor* Actor : OverlappingActors)
	{
		ALane* Lane = Cast<ALane>(Actor);
		AFork* Fork = Cast<AFork>(Actor);
		if (Lane != nullptr)
		{
			Lane->OnActorBeginOverlap.AddDynamic(this, &AIntersectionMonitor::OnEnterLane);
			Lane->OnActorEndOverlap.AddDynamic(this, &AIntersectionMonitor::OnExitLane);
		}
		else if (Fork != nullptr)
		{
			Fork->ArrivalTriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AIntersectionMonitor::OnArrival);
			Fork->EntranceTriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AIntersectionMonitor::OnEntrance);
		}
	}
}


void AIntersectionMonitor::CreateLogFile()
{
	// Init logfile name and path
	LogFileName = GetName() + "Log.cl";
	LogFileFullName = FPaths::ProjectSavedDir() + LogFileName;

	// Create the logfile 
	std::ofstream LogFile(TCHAR_TO_UTF8(*LogFileFullName), std::ios::trunc);

	// Close the file stream explicitly
	LogFile.close();
}


void AIntersectionMonitor::LoadGeometryFacts()
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
			else
			{
				continue;
			}
			FString FactString = "isToTheRightOf("
				+ RightFork + ", "
				+ LeftFork + ").";
			Geometry += TCHAR_TO_ANSI(*FactString);
			Geometry += "\n";
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
		FString FactString = "laneFromTo(l_"
			+ Lane->GetName() + ", f_"
			+ Lane->MyFork->GetName() + ", e_"
			+ Lane->MyExit->GetName() + ").";
		Geometry += TCHAR_TO_ANSI(*FactString);
		Geometry += "\n";
		
		FactString = "laneCorrectSignal(l_"
			+ Lane->GetName() + ", "
			+ Lane->GetCorrectSignal() + ").";
		Geometry += TCHAR_TO_ANSI(*FactString);
		Geometry += "\n";
	}

	// Lane overlaps
	for (size_t i = 0; i < Lanes.Num(); i++)
	{
		FString FactString = "overlaps(l_"
			+ Lanes[i]->GetName() + ", l_"
			+ Lanes[i]->GetName() + ").";
		Geometry += TCHAR_TO_ANSI(*FactString);
		Geometry += "\n";
		for (size_t j = i + 1; j < Lanes.Num(); j++)
		{
			if (Lanes[i]->IsOverlappingActor(Lanes[j]))
			{
				FactString = "overlaps(l_"
					+ Lanes[i]->GetName() + ", l_"
					+ Lanes[j]->GetName() + ").";
				Geometry += TCHAR_TO_ANSI(*FactString);
				Geometry += "\n";
				FactString = "overlaps(l_"
					+ Lanes[j]->GetName() + ", l_"
					+ Lanes[i]->GetName() + ").";
				Geometry += TCHAR_TO_ANSI(*FactString);
				Geometry += "\n";
			}
		}
	}
}


void AIntersectionMonitor::LoadTrafficRules()
{
	FString RulesFileFullName = FPaths::ProjectSavedDir() + "../Plugins/TrafficMonitor/LogicSolver/all-way-stop_new.cl";
	std::ifstream RulesFile(TCHAR_TO_UTF8(*RulesFileFullName));
	std::stringstream buffer;
	buffer << RulesFile.rdbuf();
	TrafficRules = buffer.str();
	RulesFile.close();
}


void AIntersectionMonitor::AddEvent(FString Actor, FString Atom)
{
	// TODO: Use TimeStep to buffer concurrent events
	FString& PreviousAtoms = ActorToEventsMap.FindOrAdd(Actor);
	PreviousAtoms += Atom + "\n";
	UE_LOG(LogTemp, Warning, TEXT("Event: %s"), *Atom);
}


void AIntersectionMonitor::AppendToLogfile(std::string Content)
{
	std::ofstream LogFile(TCHAR_TO_UTF8(*LogFileFullName), std::ios::app);
	LogFile << Content << std::endl;
	LogFile.close();
}


void AIntersectionMonitor::WriteGeometryToFile()
{
	FString GeometryFileFullName = FPaths::ProjectSavedDir() + GetName() + "Geometry.cl";
	std::ofstream GeometryFile(TCHAR_TO_UTF8(*GeometryFileFullName), std::ios::trunc);
	GeometryFile << Geometry;
	GeometryFile.close();
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
	AddEvent(OtherActor->GetName(), EventAtom);

	ACarlaWheeledVehicle* ArrivingVehicle = Cast<ACarlaWheeledVehicle>(OtherActor);
	if (ArrivingVehicle != nullptr)
	{
		FString SignalString = ArrivingVehicle->GetSignalString();
		EventAtom = "signalsAtForkAtTime("
			+ ArrivingVehicleID + ", "
			+ SignalString + ", "
			+ Fork + ", "
			+ FString::FromInt(TimeStep) + ").";
		AddEvent(OtherActor->GetName(), EventAtom);
		VehiclePointers.Add(OtherActor->GetName(), ArrivingVehicle);
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

	AddEvent(OtherActor->GetName(), Atom);
	Solve();
}


void AIntersectionMonitor::OnEnterLane(AActor* ThisActor, AActor* OtherActor)
{
		int32 TimeStep = FMath::FloorToInt(GetWorld()->GetTimeSeconds() / TimeResolution);
		FString EnteringActorName = "v_" + OtherActor->GetName();
		FString LaneName = "l_" + ThisActor->GetName();
		FString Atom = "entersLaneAtTime("
			+ EnteringActorName + ", "
			+ LaneName + ", "
			+ FString::FromInt(TimeStep) + ").";
		AddEvent(OtherActor->GetName(), Atom);
		Solve();
}


void AIntersectionMonitor::OnExitLane(AActor* ThisActor, AActor* OtherActor)
{
		int32 TimeStep = FMath::FloorToInt(GetWorld()->GetTimeSeconds() / TimeResolution);
		FString ExitingActorName = "v_" + OtherActor->GetName();
		FString LaneName = "l_" + ThisActor->GetName();
		FString Atom = "leavesLaneAtTime("
			+ ExitingActorName + ", "
			+ LaneName + ", "
			+ FString::FromInt(TimeStep) + ").";
		//UE_LOG(LogTemp, Warning, TEXT("%s"), *(Atom));
		AddEvent(OtherActor->GetName(), Atom);
		Solve();
}


void AIntersectionMonitor::OnExitMonitor(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	ActorToEventsMap.Remove(OtherActor->GetName());
	VehiclePointers.Remove(OtherActor->GetName());
	Solve();
}


std::string AIntersectionMonitor::GetEventsString()
{
	FString EventsString;
	for (auto& Pair : ActorToEventsMap)
	{
		EventsString += Pair.Value;
	}
	return std::string(TCHAR_TO_ANSI(*EventsString));
}


void AIntersectionMonitor::Solve()
{
	try {
		Clingo::Logger logger = [](Clingo::WarningCode, char const *message) {
			//UE_LOG(LogTemp, Warning, TEXT("Clingo logger message: %s"), ANSI_TO_TCHAR(message));
		};
		
		//std::string ProgramTitle = "#program time_" 
		//	+ std::to_string(FMath::FloorToInt(GetWorld()->GetTimeSeconds() * 1000))
		//	+ ".\n";
		std::string EventsString = GetEventsString();
		//AppendToLogfile(ProgramTitle + EventsString);

		// Without the "-n 0" option, at most one model is found.
		Clingo::Control ctl{ {}, logger, 20 };

		ctl.add("base", {}, EventsString.c_str());
		ctl.add("base", {}, Geometry.c_str());
		ctl.add("base", {}, TrafficRules.c_str());

		ctl.ground({ {"base", {}} });
		auto solveHandle = ctl.solve();
		auto solveResult = solveHandle.get();
		if (solveResult.is_unsatisfiable())
		{
			UE_LOG(LogTemp, Error, TEXT("Not satisfiable!"));
		}
		if (solveResult.is_unknown())
		{
			UE_LOG(LogTemp, Error, TEXT("Satisfiability is unknown!"));
		}
		
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
						UE_LOG(LogTemp, Warning, TEXT("Setting %s's controller to yield!"), *YieldingVehicleName);
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

template <class ActorClass>
void AIntersectionMonitor::GetIntersectingActors(TArray<ActorClass*>& OutArray)
{
	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ActorClass::StaticClass(), AllActors);
	for (AActor* Actor : AllActors)
	{
		FVector MonitorToActor = Actor->GetActorLocation() - GetActorLocation();
		FVector Displacement = UKismetMathLibrary::InverseTransformDirection(GetActorTransform(), MonitorToActor);
		if ((abs(Displacement.X) <= ExtentBox->GetScaledBoxExtent().X)
			&& (abs(Displacement.Y) <= ExtentBox->GetScaledBoxExtent().Y))
		{
			ActorClass* ActorTyped = Cast<ActorClass>(Actor);
			OutArray.Add(ActorTyped);
		}
	}
}