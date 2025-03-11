// Fill out your copyright notice in the Description page of Project Settings.


#include "Portal.h"

#include "Components/BoxComponent.h"


// Sets default values
APortal::APortal()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	ConstructorHelpers::FObjectFinder<UStaticMesh> PortalMesh(TEXT("/Game/LevelPrototyping/Meshes/SM_ChamferCube.SM_ChamferCube"));
	ConstructorHelpers::FObjectFinder<UMaterialInterface> PortalMaterial(TEXT("/Game/RenderTargets/M_Portal.M_Portal"));

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalMesh"));
	Mesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	Mesh->SetMobility(EComponentMobility::Static);
	Mesh->CastShadow = false;
	Mesh->bReceivesDecals = false;
	if (PortalMesh.Object && PortalMaterial.Object)
	{
		Mesh->SetStaticMesh(PortalMesh.Object);
		Mesh->SetMaterial(0, PortalMaterial.Object);
	}
	SetRootComponent(Mesh);

	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	Trigger->SetMobility(EComponentMobility::Static);
	Trigger->SetBoxExtent(FVector(0, 400, 400));
	Trigger->SetRelativeLocation(FVector(-60, 0, 0));
	//Trigger->OnComponentEndOverlap.AddDynamic(this, &APortal::TeleportAttempt);
	Trigger->SetupAttachment(Mesh);
}

// Called when the game starts or when spawned
void APortal::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

