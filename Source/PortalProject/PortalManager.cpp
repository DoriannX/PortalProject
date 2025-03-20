// Fill out your copyright notice in the Description page of Project Settings.


#include "PortalManager.h"
#include "Portal.h"

#include "PortalProjectWeaponComponent.h"
#include "Components/SceneCaptureComponent2D.h"

void UPortalManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UPortalProjectWeaponComponent::OnPortalSpawned.AddDynamic(this, &UPortalManager::OnPortalSpawned);
}

void UPortalManager::OnPortalSpawned(APortal* SpawnedPortal)
{
	Portals.Add(SpawnedPortal);
	if (Portals.Num() % 2 == 0)
	{
		Portals[Portals.Num() - 1]->Link(Portals[Portals.Num() - 2]);
		Portals[Portals.Num() - 2]->Link(Portals[Portals.Num() - 1]);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Portal linked"));
	}
}
