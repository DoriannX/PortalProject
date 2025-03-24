// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Portal.generated.h"

class UBoxComponent;
enum class ERenderTargetTexture : uint8;

UCLASS()
class PORTALPROJECT_API APortal : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APortal();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	UStaticMeshComponent* GetPortalVisual() const;

	UFUNCTION()
	USceneCaptureComponent2D* GetPortalCapture() const;

	UFUNCTION()
	void SetSpawnedOnActor(AActor* NewActor);
	
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool
	                    bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                  int32 OtherBodyIndex);

	bool IsLinked() const;

	void Link(APortal* NewPortal);

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Portal")
	void UpdateSceneCapture() const;

	UFUNCTION()
	static FVector MirrorVectorByNormal(const FVector& InVector, FVector InNormal);
	
	UFUNCTION()
	void SetClipPlanes() const;

	UFUNCTION(BlueprintCallable, Category = "Portal")
	void CheckViewportSize() const;
	
	UFUNCTION(BlueprintCallable, Category = "Portal")
	bool ShouldTeleport();

	UFUNCTION(BlueprintCallable, Category = "Portal")
	void UpdateCollision(const AActor* TouchedActor, bool bIsOverlapping);
	
	FVector TransformDirectionThroughPortal(const FVector& DirectionToTransform) const;
	
	FVector CalculateTeleportedCameraPosition(const FTransform& MirroredPortalTransform, const FTransform& ViewerCameraTransform) const;
	
	FRotator GetPortalCameraRotation(const FTransform& ViewerCameraTransform) const;

	bool IsPointCrossingPortal(const FVector& Point, const FVector& PortalLocation, const FVector& PortalNormal);
	
	FVector GetRotAxis(const FVector& Axis) const;
	
	FRotator GetTeleportedRotation(const FRotator& DefaultRot) const;
	
	void TeleportCharacter() const;
	
	FVector UpdateVelocity(const FVector& Velocity) const;
	
	static FTransform GetInvertedPortalTransform(const FTransform& SourcePortalTransform);
	
	void HandleLag(float DeltaTime);
	
	void IncreaseLagTime(float DeltaTime);
	
	void ResetLagTime();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Portal, meta=(AllowPrivateAccess=true))
	UStaticMeshComponent* PortalVisual;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Portal, meta=(AllowPrivateAccess=true))
	USceneCaptureComponent2D* PortalCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Portal, meta=(AllowPrivateAccess=true))
	UBoxComponent* PlayerDetection;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Optimisation, meta=(AllowPrivateAccess=true))
	float LagThresholdFPS;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Optimisation, meta=(AllowPrivateAccess=true))
	double LagTimeThresholdSecond;
	
	UPROPERTY()
	UMaterialInstanceDynamic* PortalMat;

	UPROPERTY()
	UTextureRenderTarget2D* PortalRenderTexture;

	UPROPERTY()
	APortal* LinkedPortal;

	UPROPERTY()
	bool LastInFront;

	UPROPERTY()
	AActor* SpawnedOnActor;

	UPROPERTY()
	double LagTimeSecond;
};
