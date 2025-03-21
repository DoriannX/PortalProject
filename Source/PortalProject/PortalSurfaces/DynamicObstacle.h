// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "DynamicObstacle.generated.h"

UCLASS()
class PORTALPROJECT_API ADynamicObstacle : public AActor
{
	GENERATED_BODY()
//Functions
public:
	ADynamicObstacle();

	virtual void Tick(float DeltaTime) override;
	void UpdateCubeSize();
	void SetCubeSize(const FVector& NewCubeSize);

protected:
	virtual void BeginPlay() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	
//Properties
public:

protected:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category=Components)
	UProceduralMeshComponent* MeshComponent;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Transform)
	FVector TargetCubeSize = FVector::One();

	UPROPERTY()
	FVector PreviousCubeSize;

	UPROPERTY()
	TArray<FVector> Vertices;

	UPROPERTY()
	TArray<FVector> Normals;

	UPROPERTY()
	TArray<int32> Triangles;
	
	UPROPERTY()
	TArray<FVector2D> UVs;
	
	UPROPERTY()
	TArray<FProcMeshTangent> Tangents;
	
	
};
