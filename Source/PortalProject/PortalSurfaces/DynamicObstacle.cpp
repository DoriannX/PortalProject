// Fill out your copyright notice in the Description page of Project Settings.


#include "DynamicObstacle.h"
#include "KismetProceduralMeshLibrary.h"


ADynamicObstacle::ADynamicObstacle()
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	MeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	SetCubeSize(TargetCubeSize);
}

void ADynamicObstacle::BeginPlay()
{
	Super::BeginPlay();
}

#if WITH_EDITOR
  void ADynamicObstacle::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	UpdateCubeSize();
}
#endif


void ADynamicObstacle::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateCubeSize();
}

void ADynamicObstacle::UpdateCubeSize()
{
	if (PreviousCubeSize == TargetCubeSize)
	{
		return;
	}
	PreviousCubeSize = TargetCubeSize;
	SetCubeSize(TargetCubeSize);
}

void ADynamicObstacle::SetCubeSize(const FVector& NewCubeSize)
{
	UKismetProceduralMeshLibrary::GenerateBoxMesh(NewCubeSize, Vertices, Triangles, Normals, UVs, Tangents);
	if (MeshComponent->GetNumSections() > 0)
	{
		MeshComponent->UpdateMeshSection(0, Vertices, Normals, UVs, TArray{FColor::Yellow}, Tangents);
	}
	else
	{
		MeshComponent->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, TArray{FColor::Yellow}, Tangents, true);
	}
	
	MeshComponent->SetRelativeLocation(FVector(NewCubeSize.X, NewCubeSize.Y, NewCubeSize.Z));
}

