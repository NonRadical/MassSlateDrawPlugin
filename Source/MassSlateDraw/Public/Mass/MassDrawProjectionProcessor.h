// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassDrawProjectionProcessor.generated.h"

//Processor responsible for taking all FMassDrawStateFragment fragments and updating their projection information for the current frame.
UCLASS()
class MASSSLATEDRAW_API UMassDrawProjectionProcessor : public UMassProcessor
{
	GENERATED_UCLASS_BODY()

protected:
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery DrawProjectionQuery;
};