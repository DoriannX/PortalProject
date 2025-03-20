// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PortalManager.generated.h"

class APortal;

UCLASS(Blueprintable, BlueprintType)
class PORTALPROJECT_API UPortalManager : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION()
	void OnPortalSpawned(APortal* SpawnedPortal);

protected:

	UPROPERTY()
	TArray<TObjectPtr<APortal>> Portals;

	
};
