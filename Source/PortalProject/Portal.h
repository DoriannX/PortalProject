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
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	bool IsLinked() const;

	void Link(APortal* NewPortal);

	UFUNCTION(BlueprintCallable, Category = "Portal")
	FVector GetBackwardVector(const FVector& ForwardVector) const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	FVector TransformDirectionBetweenPortals(const FVector& DirectionToTransform) const;
	static FRotator MakeRotationFromAxes(FVector Forward, FVector Right, FVector Up);
	static void BreakRotIntoAxes(const FRotator& InRotation, FVector& X, FVector& Y, FVector& Z);
	static FTransform GetMirroredPortalTransform(const FTransform& SourcePortalTransform);
	FVector GetPortalCameraPosition(const FTransform& MirroredPortalTransform, const FTransform& ViewerCameraTransform) const;
	FRotator GetPortalCameraRotation(const FTransform& ViewerCameraTransform) const;

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

	bool IsPointCrossingPortal(const FVector& Point, const FVector& PortalLocation, const FVector& PortalNormal);
	FVector GetRotAxis(const FVector& Axis) const;
	FRotator GetTeleportedRotation(const FRotator& DefaultRot) const;
	void TeleportCharacter() const;
	FVector UpdateVelocity(const FVector& Velocity) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Portal, meta=(AllowPrivateAccess=true))
	UStaticMeshComponent* PortalVisual;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Portal, meta=(AllowPrivateAccess=true))
	USceneCaptureComponent2D* PortalCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Portal, meta=(AllowPrivateAccess=true))
	UBoxComponent* PlayerDetection;
	
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
	
};
