// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "IntersectionMonitor.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "Runtime/Core/Public/Misc/Paths.h"
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
	ExtentBox->SetBoxExtent(FVector{ 1500.0f, 1500.0f, 100.0f });
	ExtentBox->ShapeColor = FColor(255, 255, 255);
}

// Called when the game starts or when spawned
void AIntersectionMonitor::BeginPlay()
{
	Super::BeginPlay();

	CreateLogFile();

	AddToLoggers();
}


void AIntersectionMonitor::AddToLoggers()
{
	TArray<AActor *> OverlappingActors;
	GetOverlappingActors(OverlappingActors, AFork::StaticClass());
	for (AActor* Fork : OverlappingActors)
	{
		AFork* ForkPtr = Cast<AFork>(Fork);
		if (ForkPtr != nullptr)
		{
			ForkPtr->MyMonitor = this;
			UE_LOG(LogTemp, Warning, TEXT("%s connected to a logger!"), *(ForkPtr->GetName()));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Could not connect: Overlapping actor was null!"));
		}
	}
	
	OverlappingActors.Empty();
	GetOverlappingActors(OverlappingActors, AIntersectionExit::StaticClass());
	for (AActor* Exit : OverlappingActors)
	{
		AIntersectionExit* ExitPtr = Cast<AIntersectionExit>(Exit);
		if (ExitPtr != nullptr)
		{
			ExitPtr->MyMonitor = this;
			UE_LOG(LogTemp, Warning, TEXT("%s connected to a logger!"), *(ExitPtr->GetName()));
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
