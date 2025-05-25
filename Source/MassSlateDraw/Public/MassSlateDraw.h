// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMassSlateDraw, Log, All);

DECLARE_STATS_GROUP(TEXT("MassDraw"), STATGROUP_MassDraw, STATCAT_Advanced);

class FMassSlateDrawModule : public IModuleInterface
{
//~ Begin IModuleInterface Interface
public:
	virtual void StartupModule() override {}
	virtual void ShutdownModule() override {}
//~ End IModuleInterface Interface
};