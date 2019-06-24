// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#include "LogWriter.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"
#include "Runtime/Core/Public/Misc/Paths.h"

#include <fstream>
#include <iostream>

// Sets default values for this component's properties
ULogWriter::ULogWriter()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void ULogWriter::BeginPlay()
{
	Super::BeginPlay();

	FileName = UKismetSystemLibrary::GetDisplayName(GetOwner()) + ".cl";
	this->AbsoluteFilePath = FPaths::ProjectSavedDir() + this->FileName;
	//Creates an instance of ofstream, and creates a new file with address AbsoluteFilePath
	std::ofstream a_file(std::string(TCHAR_TO_UTF8(*AbsoluteFilePath)), std::ios::trunc);
	// Close the file stream explicitly
	a_file.close();
}


// Called every frame
void ULogWriter::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void ULogWriter::AddEvent(FString EventMessage)
{
	//Creates an instance of ofstream, and opens 
	std::ofstream logfile(TCHAR_TO_UTF8(*AbsoluteFilePath), std::ios::app);
	// Outputs to example.txt through a_file
	logfile << TCHAR_TO_UTF8(*EventMessage) << std::endl;
	// Close the file stream explicitly
	logfile.close();
}
