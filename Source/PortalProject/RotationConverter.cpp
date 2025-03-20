#include "RotationConverter.h"

bool FRotationConverter::IsTooClose(const FVector& Vector1, const FVector& Vector2)
{
	return FMath::Abs(FVector::DotProduct(Vector1, Vector2)) > 0.9f;
}

FRotator FRotationConverter::CreateRotation(const FVector& ForwardVector, const FVector& RightVector,
                                            const FVector& UpVector)
{
	const FMatrix RotationMatrix = FMatrix(ForwardVector, RightVector, UpVector, FVector::ZeroVector);
	const FRotator SpawnRotation = RotationMatrix.Rotator();
	return SpawnRotation;
}

FVector FRotationConverter::GenerateForward(const FVector& UpVector, const FVector& ForwardVector)
{
	if (!IsTooClose(UpVector, FVector::UpVector))
	{
		return FVector::UpVector;
	}

	return MakeVectorPerpendicular(ForwardVector, UpVector);
}

FVector FRotationConverter::GenerateRight(const FVector& UpVector, const FVector& ForwardVector)
{
	return FVector::CrossProduct(UpVector, ForwardVector);
}

bool FRotationConverter::IsPerpendicular(const FVector& Vector1, const FVector& Vector2)
{
	return FMath::Abs(FVector::DotProduct(Vector1, Vector2)) < 0.1f;
}

FVector FRotationConverter::MakeVectorPerpendicular(const FVector& ForwardVector, const FVector& UpVector)
{
	const float ForwardProjectionOntoUp = FVector::DotProduct(ForwardVector, UpVector);
	const FVector ForwardAlongUp = UpVector * ForwardProjectionOntoUp;
	FVector Up = ForwardVector - ForwardAlongUp;
	Up.Normalize();
	return Up;
}
