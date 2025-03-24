// Fill out your copyright notice in the Description page of Project Settings.


#include "Portal.h"

#include "PortalProjectCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"


// Sets default values
APortal::APortal()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	PortalCamera = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("PortalCamera"));
	PortalCamera->SetupAttachment(RootComponent);
	PortalVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalVisual"));
	PortalVisual->SetupAttachment(RootComponent);

	PlayerDetection = CreateDefaultSubobject<UBoxComponent>(TEXT("PlayerDetection"));
	PlayerDetection->SetupAttachment(RootComponent);
}

UStaticMeshComponent* APortal::GetPortalVisual() const
{
	if (PortalVisual == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("APortalVisual is not referenced"));
	}
	return PortalVisual;
}

USceneCaptureComponent2D* APortal::GetPortalCapture() const
{
	if (PortalCamera == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("APortalCapture is not referenced"));
	}
	return PortalCamera;
}

FVector APortal::TransformDirectionThroughPortal(const FVector& DirectionToTransform) const
{
	const FTransform& CurrentPortalTransform = GetActorTransform();
	const FTransform TargetPortalTransform = LinkedPortal->GetActorTransform();

	const FVector LocalDirection = CurrentPortalTransform.InverseTransformVector(DirectionToTransform);

	const FVector MirroredDirection = MirrorVectorByNormal(
		MirrorVectorByNormal(LocalDirection, FVector(1, 0, 0)), FVector(0, 1, 0));

	return TargetPortalTransform.TransformVector(
		MirroredDirection);
}

FTransform APortal::GetInvertedPortalTransform(const FTransform& SourcePortalTransform)
{
    FTransform InvertedPortalTransform = SourcePortalTransform;
    FVector3d InvertedScale = InvertedPortalTransform.GetScale3D();
    InvertedScale.Set(InvertedScale.X * -1, InvertedScale.Y * -1, InvertedScale.Z);
    InvertedPortalTransform.SetScale3D(InvertedScale);
    return InvertedPortalTransform;
}

FVector APortal::CalculateTeleportedCameraPosition(const FTransform& MirroredPortalTransform,
                                         const FTransform& ViewerCameraTransform) const
{
	const FVector ViewerPosition = ViewerCameraTransform.GetLocation();
	const FVector RelativePositionToSourcePortal = MirroredPortalTransform.InverseTransformPosition(ViewerPosition);

	const FTransform DestinationPortalTransform = LinkedPortal->GetActorTransform();
	const FVector TeleportedViewerPosition = DestinationPortalTransform.TransformPosition(
		RelativePositionToSourcePortal);
	return TeleportedViewerPosition;
}

FRotator APortal::GetPortalCameraRotation(const FTransform& ViewerCameraTransform) const
{
	FRotator ViewerRotation = ViewerCameraTransform.GetRotation().Rotator();
	FVector ViewerForwardVector;
	FVector ViewerRightVector;
	FVector ViewerUpVector;
	UKismetMathLibrary::BreakRotIntoAxes(ViewerRotation, ViewerForwardVector, ViewerRightVector, ViewerUpVector);
	ViewerForwardVector = TransformDirectionThroughPortal(ViewerForwardVector);
	ViewerRightVector = TransformDirectionThroughPortal(ViewerRightVector);
	ViewerUpVector = TransformDirectionThroughPortal(ViewerUpVector);
	FRotator TeleportedViewerRotation =UKismetMathLibrary::MakeRotationFromAxes(ViewerForwardVector, ViewerRightVector, ViewerUpVector);
	return TeleportedViewerRotation;
}

void APortal::UpdateSceneCapture() const
{
	UWorld* World = GetWorld();
	if (!IsValid(World) || !IsValid(LinkedPortal) || !IsValid(LinkedPortal->PortalCamera))
	{
		return;
	}

	const APlayerController* PlayerController = World->GetFirstPlayerController();
	if (PlayerController == nullptr)
	{
		return;
	}

	const APlayerCameraManager* PlayerCameraManager = PlayerController->PlayerCameraManager;

	if (!IsValid(PlayerCameraManager))
	{
		return;
	}

	const FTransform& ViewerCameraTransform = PlayerCameraManager->GetActorTransform();

	//Update location
	FTransform MirroredPortalTransform = GetInvertedPortalTransform(GetActorTransform());
	const FVector TeleportedViewerPosition = CalculateTeleportedCameraPosition(MirroredPortalTransform, ViewerCameraTransform);

	//Update rotation
	const FRotator TeleportedViewerRotation = GetPortalCameraRotation(ViewerCameraTransform);

	LinkedPortal->PortalCamera->SetWorldLocationAndRotation(TeleportedViewerPosition,
	                                                        TeleportedViewerRotation);
}

FVector APortal::MirrorVectorByNormal(const FVector& InVector, FVector InNormal)
{
	InNormal = InNormal.GetSafeNormal();

	// Mirror formula: V' = V - 2(V·N)N
	// Where V is the input vector, N is the normal, and V' is the mirrored vector
	return InVector - 2.0f * FVector::DotProduct(InVector, InNormal) * InNormal;
}

// Called when the game starts or when spawned
void APortal::BeginPlay()
{
	Super::BeginPlay();
	PlayerDetection->OnComponentBeginOverlap.AddDynamic(this, &APortal::OnBeginOverlap);
	PlayerDetection->OnComponentEndOverlap.AddDynamic(this, &APortal::OnEndOverlap);
}

void APortal::IncreaseLagTime(const float DeltaTime)
{
	LagTimeSecond += DeltaTime;
}

void APortal::ResetLagTime()
{
	LagTimeSecond = 0;
}

void APortal::HandleLag(const float DeltaTime)
{
	const float FPS = 1.0f / DeltaTime;
	const bool IsLagging = FPS < LagThresholdFPS;
	const UWorld* World = GetWorld();
	
	if (World == nullptr)
	{
		return;
	}
	
	if (IsLagging)
	{
		IncreaseLagTime(DeltaTime);
	}
	else
	{
		ResetLagTime();
	}
	const bool LagPersist = LagTimeSecond > LagTimeThresholdSecond;
	if (LagPersist)
	{
		Destroy();
	}
}

// Called every frame
void APortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	HandleLag(DeltaTime);
}

bool APortal::IsLinked() const
{
	return IsValid(LinkedPortal);
}

void APortal::Link(APortal* NewPortal)
{
	if (!(IsValid(PortalVisual) && IsValid(PortalCamera)))
	{
		return;
	}
	LinkedPortal = NewPortal;
	SetTickGroup(TG_PostUpdateWork);


	PortalMat = UMaterialInstanceDynamic::Create(
		PortalVisual->GetMaterials().Last(), this);

	PortalVisual->SetMaterial(0, PortalMat);

	FVector2D ViewportSize = FVector2D();
	GEngine->GameViewport->GetViewportSize(ViewportSize);

	PortalRenderTexture = NewObject<UTextureRenderTarget2D>();
	PortalRenderTexture->InitCustomFormat(ViewportSize.X, ViewportSize.Y, PF_FloatRGBA, false);

	PortalMat->SetTextureParameterValue("Texture", PortalRenderTexture);

	LinkedPortal->PortalCamera->TextureTarget = PortalRenderTexture;

	SetClipPlanes();
}

void APortal::SetClipPlanes() const
{
	if (!IsValid(LinkedPortal) || !IsValid(PortalCamera))
	{
		return;
	}

	PortalCamera->bEnableClipPlane = true;

	const FVector CurrentPosition = GetActorLocation();
	const FVector ForwardDirection = GetActorForwardVector();

	const FVector ClipPlaneBase = CurrentPosition + ForwardDirection * -3;


	PortalCamera->ClipPlaneBase = ClipPlaneBase;
	PortalCamera->ClipPlaneNormal = ForwardDirection;
}

void APortal::CheckViewportSize() const
{
	FVector2D ViewportSize = FVector2D();
	GEngine->GameViewport->GetViewportSize(ViewportSize);
	if (!IsValid(PortalRenderTexture) || (PortalRenderTexture->SizeX == ViewportSize.X && PortalRenderTexture->SizeY ==
		ViewportSize.Y))
	{
		return;
	}
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Yellow, TEXT("Viewport size changed"));
	PortalRenderTexture->ResizeTarget(ViewportSize.X, ViewportSize.Y);
}

bool APortal::ShouldTeleport()
{
	TArray<AActor*> DetectedActors;
	PlayerDetection->GetOverlappingActors(DetectedActors, TSubclassOf<APortalProjectCharacter>());
	const UWorld* World = GetWorld();
	const int32 Size = DetectedActors.Num();
	if (!IsValid(World) || Size == 0)
	{
		return false;
	}
	bool bIsPlayerCrossingPortal = IsPointCrossingPortal(
		UGameplayStatics::GetPlayerController(World, 0)->PlayerCameraManager->GetCameraLocation(), GetActorLocation(),
		GetActorForwardVector());
	if (bIsPlayerCrossingPortal)
	{
#if !UE_BUILD_SHIPPING
		UKismetSystemLibrary::PrintString(World, "teleport!");
#endif
		TeleportCharacter();
	}
	return false;
}

bool APortal::IsPointCrossingPortal(const FVector& Point, const FVector& PortalLocation, const FVector& PortalNormal)
{
	const bool IsInFront = FVector::DotProduct(PortalNormal, Point - PortalLocation) >= 0;
	const FPlane PortalPlane = UKismetMathLibrary::MakePlaneFromPointAndNormal(Point, PortalNormal);
	FVector LastPosition = FVector();
	float T;
	FVector Intersection;
	const bool IsIntersect = UKismetMathLibrary::LinePlaneIntersection(LastPosition, Point, PortalPlane, T,
	                                                                   Intersection);
	const bool IsCrossing = IsIntersect && !IsInFront && LastInFront;
	LastInFront = IsInFront;
	LastPosition = Point;
	return IsCrossing;
}

FVector APortal::GetRotAxis(const FVector& Axis) const
{
	FVector MirroredVector = UKismetMathLibrary::MirrorVectorByNormal(
		UKismetMathLibrary::MirrorVectorByNormal(GetActorTransform().InverseTransformVector(Axis), FVector(1, 0, 0)),
		FVector(0, 1, 0)
	);
	return LinkedPortal->GetActorTransform().TransformVector(MirroredVector);
}

FRotator APortal::GetTeleportedRotation(const FRotator& DefaultRot) const
{
	;
	FVector AxisX, AxisY, AxisZ;
	UKismetMathLibrary::GetAxes(DefaultRot, AxisX, AxisY,
	                            AxisZ);
	AxisX = GetRotAxis(AxisX);
	AxisY = GetRotAxis(AxisY);
	AxisZ = GetRotAxis(AxisZ);

	return UKismetMathLibrary::MakeRotationFromAxes(AxisX, AxisY, AxisZ);
}

void APortal::TeleportCharacter() const
{
	const UWorld* World = GetWorld();
	if (!IsValid(World) || !IsValid(LinkedPortal))
	{
		return;
	}
	FTransform NewTransform = GetActorTransform();
	const FVector ActorScale = NewTransform.GetScale3D();
	const FVector PlayerLocation = UGameplayStatics::GetPlayerCharacter(World, 0)->GetActorLocation();
	NewTransform.SetScale3D(FVector(ActorScale.X * -1, ActorScale.Y * -1, ActorScale.Z));
	const FVector NewTransformLocation = LinkedPortal->GetActorTransform().TransformPosition(
		NewTransform.InverseTransformPosition(PlayerLocation));


	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(World, 0);

	if (!IsValid(PlayerCharacter))
	{
		return;
	}

	const FRotator NewRotation = GetTeleportedRotation(
		PlayerCharacter->GetActorRotation());
	const FRotator NewControlRotation = GetTeleportedRotation(
		PlayerCharacter->GetControlRotation());

	UGameplayStatics::GetPlayerController(World, 0)->SetControlRotation(NewControlRotation);

	PlayerCharacter->GetMovementComponent()->Velocity = UpdateVelocity(
		PlayerCharacter->GetMovementComponent()->Velocity);

	PlayerCharacter->SetActorLocation(NewTransformLocation);
	PlayerCharacter->SetActorRotation(NewRotation);

	APortalProjectCharacter* PortalCharacter = Cast<APortalProjectCharacter>(PlayerCharacter);
	APlayerCameraManager* PlayerCameraManager = UGameplayStatics::GetPlayerCameraManager(World, 0);
	if (!IsValid(PortalCharacter) || !IsValid(PlayerCameraManager))
	{
		return;
	}
	PortalCharacter->InitOrientation();
	PortalCharacter->SmoothOrientation();
	PlayerCameraManager->SetGameCameraCutThisFrame();
}

FVector APortal::UpdateVelocity(const FVector& Velocity) const
{
	if (!IsValid(LinkedPortal))
	{
		return FVector();
	}

	FVector LocalVelocity = GetActorTransform().InverseTransformVector(Velocity);

	FVector MirroredLocalVelocityX = UKismetMathLibrary::MirrorVectorByNormal(LocalVelocity, FVector(1, 0, 0));
	FVector MirroredLocalVelocity = UKismetMathLibrary::MirrorVectorByNormal(MirroredLocalVelocityX, FVector(0, 1, 0));


	FVector TransformedVelocity = LinkedPortal->GetActorTransform().TransformVector(MirroredLocalVelocity);
	TransformedVelocity.Normalize();
	return TransformedVelocity * Velocity.Length();
}

void APortal::UpdateCollision(const AActor* TouchedActor, const bool bIsOverlapping)
{
	const UWorld* World = GetWorld();
	if (!IsValid(TouchedActor) || !IsValid(World))
	{
		return;
	}
	UActorComponent* TouchedActorComponent = TouchedActor->GetComponentByClass(UStaticMeshComponent::StaticClass());
	if (!IsValid(TouchedActorComponent))
	{
		return;
	}
	UStaticMeshComponent* TouchedActorMesh = Cast<UStaticMeshComponent>(TouchedActorComponent);
	if (!IsValid(TouchedActorMesh))
	{
		return;
	}
	FName CollisionProfile = bIsOverlapping ? TEXT("OverlapOnlyPawn") : TEXT("BlockAll");
	TouchedActorMesh->SetCollisionProfileName(CollisionProfile);
	UKismetSystemLibrary::PrintString(World, "Collision Updated");
}

void APortal::SetSpawnedOnActor(AActor* NewActor)
{
	SpawnedOnActor = NewActor;
}

void APortal::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                             const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor) || !IsLinked())
	{
		return;
	}
	ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor);
	if (!IsValid(PlayerCharacter))
	{
		return;
	}
	UpdateCollision(SpawnedOnActor, true);
}

void APortal::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                           int32 OtherBodyIndex)
{
	if (!IsValid(OtherActor) || !IsLinked())
	{
		return;
	}
	ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor);
	if (!IsValid(PlayerCharacter))
	{
		return;
	}
	UpdateCollision(SpawnedOnActor, false);
}
