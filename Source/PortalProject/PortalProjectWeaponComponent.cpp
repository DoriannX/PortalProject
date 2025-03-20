// Copyright Epic Games, Inc. All Rights Reserved.


#include "PortalProjectWeaponComponent.h"
#include "PortalProjectCharacter.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Portal.h"
#include "RotationConverter.h"
#include "Animation/AnimInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include <vector>

#include "PortalManager.h"

// Sets default values for this component's properties
UPortalProjectWeaponComponent::UPortalProjectWeaponComponent()
{
	// Default offset from the character location for projectiles to spawn
	MuzzleOffset = FVector(100.0f, 0.0f, 10.0f);
}

TArray<TEnumAsByte<EObjectTypeQuery>> UPortalProjectWeaponComponent::ConvertCollisionChannelToObjectType(
	const TArray<ECollisionChannel>& CollisionChannels)
{
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	for (const ECollisionChannel CollisionChannel : CollisionChannels)
	{
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(CollisionChannel));
	}
	return ObjectTypes;
}

TArray<FVector> UPortalProjectWeaponComponent::ConvertSurfaceExtremityToLineTraceExtremity(
	const FVector& LineTraceOrigin, const TArray<FVector>& Extremities)
{
	TArray<FVector> SurfaceExtremity = Extremities;
	for (FVector& Extremity : SurfaceExtremity)
	{
		const FVector RaycastDirection = Extremity -
			LineTraceOrigin;
		Extremity = RaycastDirection * 1000;
	}
	return SurfaceExtremity;
}

TArray<FVector> UPortalProjectWeaponComponent::GetSurfaceExtremity(const FVector& Center, const FVector& Normal,
                                                                   const FVector& Forward, const FVector2D& Size)
{
	TArray<FVector> Extremity;
	const FVector Up = FRotationConverter::GenerateForward(Normal, Forward);
	const FVector Right = FRotationConverter::GenerateRight(Normal, Up);
	for (int x = -1; x <= 1; ++x)
	{
		for (int y = -1; y <= 1; ++y)
		{
			if (x == 0 || y == 0)
			{
				continue;
			}

			Extremity.Add((Center + Up * x * Size.X / 2 + Right * y * Size.Y / 2));
		}
	}
	return Extremity;
}

bool UPortalProjectWeaponComponent::HitValidSurface(const FVector& RaycastOrigin,
                                                    const FHitResult& MainLineTraceHitResult,
                                                    const TArray<FVector>& PortalRaycastExtremities) const
{
	for (auto Extremity : PortalRaycastExtremities)
	{
		FHitResult ExtremityRaycastHitResult;
		if (!CreateLineTrace(RaycastOrigin, Extremity, ExtremityRaycastHitResult, {ECC_WorldStatic}))
		{
			return false;
		}

		if (!IsPointOnPlane(ExtremityRaycastHitResult.Location, MainLineTraceHitResult.Location,
		                    MainLineTraceHitResult.Normal))
		{
			return false;
		}
	}
	return true;
}

void UPortalProjectWeaponComponent::SpawnPortal(UWorld* const World, const FHitResult& MainLineTraceHitResult) const
{
	UPortalManager* PortalManager = World->GetSubsystem<UPortalManager>();

	if (PortalManager == nullptr)
	{
		return;
	}

	const APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!IsValid(PlayerController))
	{
		return;
	}

	//If the normal is perpendicular to the character up vector then the up vector is the player forward vector translated perpendicular to the normal
	FVector3d RightVector = FRotationConverter::IsPerpendicular(MainLineTraceHitResult.Normal, FVector::UpVector)
		                     ? FRotationConverter::GenerateRight(FVector::UpVector, MainLineTraceHitResult.Normal)
		                     : -PlayerController->GetPawn()->GetActorRightVector();
	const FRotator SpawnRotation = FRotationMatrix::MakeFromXY(MainLineTraceHitResult.Normal, RightVector).Rotator();

	const FVector SpawnLocation = MainLineTraceHitResult.Location;
	APortal* SpawnedPortal = World->SpawnActor<APortal>(Portal, SpawnLocation, SpawnRotation);
	if (!IsValid(SpawnedPortal))
	{
		return;
	}
	SpawnedPortal->SetActorLocation(SpawnLocation + SpawnedPortal->GetActorForwardVector() * -SpawnedPortal->GetPortalVisual()->GetRelativeLocation().X); 
	PortalManager->OnPortalSpawned(SpawnedPortal);
}

void UPortalProjectWeaponComponent::PlaySfxes() const
{
	UGameplayStatics::PlaySoundAtLocation(this, FireSound, Character->GetActorLocation());

	UAnimInstance* AnimInstance = Character->GetMesh1P()->GetAnimInstance();
	if (AnimInstance != nullptr)
	{
		AnimInstance->Montage_Play(FireAnimation, 1.f);
	}
}

void UPortalProjectWeaponComponent::Fire()
{
	UWorld* const World = GetWorld();
	if (Character == nullptr || Character->GetController() == nullptr || World == nullptr || FireSound == nullptr ||
		FireAnimation == nullptr || Portal == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Something went wrong in Fire function"));
		return;
	}

	const APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());

	const FVector LineTraceStart = PlayerController->PlayerCameraManager->GetCameraLocation();
	const FVector LineTraceEnd = LineTraceStart + PlayerController->PlayerCameraManager->GetActorForwardVector() *
		10000;

	FHitResult MainLineTraceHitResult;

	const bool HitSomething = CreateLineTrace(LineTraceStart, LineTraceEnd, MainLineTraceHitResult, {ECC_WorldStatic});

	if (!HitSomething)
	{
		return;
	}
	const TArray<FVector> SurfaceExtremities = GetSurfaceExtremity(MainLineTraceHitResult.Location,
	                                                               MainLineTraceHitResult.Normal,
	                                                               PlayerController->PlayerCameraManager->
	                                                               GetActorForwardVector(),
	                                                               Portal.GetDefaultObject()->GetPortalSize());
	const TArray<FVector> PortalRaycastExtremities = ConvertSurfaceExtremityToLineTraceExtremity(
		LineTraceStart, SurfaceExtremities);

	if (!HitValidSurface(LineTraceStart, MainLineTraceHitResult, PortalRaycastExtremities))
	{
		return;
	}

	SpawnPortal(World, MainLineTraceHitResult);

	PlaySfxes();
}

bool UPortalProjectWeaponComponent::AttachWeapon(APortalProjectCharacter* TargetCharacter)
{
	Character = TargetCharacter;

	// Check that the character is valid, and has no weapon component yet
	if (Character == nullptr || Character->GetInstanceComponents().FindItemByClass<UPortalProjectWeaponComponent>())
	{
		return false;
	}

	// Attach the weapon to the First Person Character
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
	AttachToComponent(Character->GetMesh1P(), AttachmentRules, FName(TEXT("GripPoint")));

	// Set up action bindings
	if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			// Set the priority of the mapping to 1, so that it overrides the Jump action with the Fire action when using touch input
			Subsystem->AddMappingContext(FireMappingContext, 1);
		}

		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(
			PlayerController->InputComponent))
		{
			// Fire
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this,
			                                   &UPortalProjectWeaponComponent::Fire);
		}
	}

	return true;
}

bool UPortalProjectWeaponComponent::CreateLineTrace(
	const FVector& Start, const FVector& End, FHitResult& HitResult,
	const TArray<ECollisionChannel>& CollisionChannels = {}) const
{
	const UWorld* World = GetWorld();

	if (World == nullptr)
	{
		return false;
	}


	const bool HitSomething = UKismetSystemLibrary::LineTraceSingleForObjects(
		World,
		Start,
		End,
		ConvertCollisionChannelToObjectType(CollisionChannels),
		false,
		{},
		EDrawDebugTrace::ForDuration,
		HitResult,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		5.0f
	);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("line trace"));

	return HitSomething;
}

void UPortalProjectWeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// ensure we have a character owner
	if (Character != nullptr)
	{
		// remove the input mapping context from the Player Controller
		if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
				UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			{
				Subsystem->RemoveMappingContext(FireMappingContext);
			}
		}
	}
	// maintain the EndPlay call chain
	Super::EndPlay(EndPlayReason);
}

bool UPortalProjectWeaponComponent::IsPointOnPlane(const FVector& Point, const FVector& PlaneOrigin,
                                                   const FVector& PlaneNormal)
{
	FVector PlaneToPoint = Point - PlaneOrigin;

	float Distance = FVector::DotProduct(PlaneToPoint, PlaneNormal);

	constexpr float Epsilon = 0.0001f;
	return FMath::Abs(Distance) < Epsilon;
}

FOnPortalSpawned UPortalProjectWeaponComponent::OnPortalSpawned;
