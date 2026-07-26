// 
// Logger.h
// 
// Utility that generalizes printing to screen. 
// 
// ----------------------------------------  x  ---------------------------------------- 
// 
// © 2026 CylindriKill. All rights reserved.
// 

#pragma once

#include "Runtime/Engine/Classes/Engine/Engine.h"

#define DEBUG_BUFFER_SIZE 256
#define PREPARE_BUFFER()                 \
	char DebugBuffer[DEBUG_BUFFER_SIZE]; \
	va_list ArgList;                     \
	va_start(ArgList, Message);          \
	vsnprintf(DebugBuffer,               \
		DEBUG_BUFFER_SIZE,               \
		Message,                         \
		ArgList                          \
	)

static void ConsoleLog(const FString& Prefix, const FColor Color, const FString& Message)
{
	const FString FormattedMessage = Prefix + Message;
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			5.0f,
			Color,
			FormattedMessage
		);
	}

	UE_LOG(LogTemp, Log, TEXT("%s"), *(Prefix + Message));
}

namespace FLogger
{	
	static void DebugLog(const char* Message, ...)
	{
		PREPARE_BUFFER();
		ConsoleLog("[DEBUG]: ", FColor::Cyan, DebugBuffer);
	}

	static void WarningLog(const char* Message, ...)
	{
		PREPARE_BUFFER();
		ConsoleLog("[WARNING]: ", FColor::Yellow, DebugBuffer);
	}
	
	static void ErrorLog(const char* Message, ...)
	{
		PREPARE_BUFFER();
		ConsoleLog("[ERROR]: ", FColor::Red, DebugBuffer);
	}
};
