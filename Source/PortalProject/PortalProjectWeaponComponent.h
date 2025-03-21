// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "PortalProjectWeaponComponent.generated.h"

class APortal;
class APortalProjectCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPortalSpawned, APortal*, SpawnedPortal);

UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PORTALPROJECT_API UPortalProjectWeaponComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()

public:
	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<APortal> Portal;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	USoundBase* FireSound;
	
	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	UAnimMontage* FireAnimation;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector MuzzleOffset;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputMappingContext* FireMappingContext;

	/** Fire Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* FireAction;

	/** Sets default values for this component's properties */
	UPortalProjectWeaponComponent();

	/** Attaches the actor to a FirstPersonCharacter */
	UFUNCTION(BlueprintCallable, Category="Weapon")
	bool AttachWeapon(APortalProjectCharacter* TargetCharacter);

	bool CreateLineTrace(const FVector& Start, const FVector& End, FHitResult& HitResult, const TArray<ECollisionChannel>& CollisionChannels) const;

	static TArray<TEnumAsByte<EObjectTypeQuery>> ConvertCollisionChannelToObjectType(const TArray<ECollisionChannel>& CollisionChannels);

	static TArray<FVector> ConvertSurfaceExtremityToLineTraceExtremity(const FVector& LineTraceOrigin, const TArray<FVector>& Extremities);
	
	static TArray<FVector> GetSurfaceExtremity(const FVector& Center, const FVector& Normal, const FVector& Forward,
	                                           const FVector2D& Size);

	bool HitValidSurface(
		const FVector& LineTraceOrigin, const FVector& LineTraceEnd, const FVector& Forward, FHitResult&
		MainLineTraceHitResult) const;

	bool CanPlacePortalHere() const;
	void SpawnPortal(UWorld* World, const FHitResult& MainLineTraceHitResult) const;
	void PlaySfxes() const;
	/** Make the weapon Fire a Projectile */
	UFUNCTION(BlueprintCallable, Category="Weapon")
	void Fire();
	
	static FOnPortalSpawned OnPortalSpawned;

protected:
	/** Ends gameplay for this component. */
	UFUNCTION()
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	static bool IsPointOnPlane(const FVector& Point, const FVector& PlaneOrigin, const FVector& PlaneNormal);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Transform)
	FVector2D PortalSize;

private:
	/** The Character holding this weapon*/
	APortalProjectCharacter* Character;
};
