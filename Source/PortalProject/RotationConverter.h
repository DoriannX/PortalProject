#pragma once

class FRotationConverter
{
public:
	static FRotator CreateRotation(const FVector& ForwardVector, const FVector& RightVector, const FVector& UpVector);
	static FVector GenerateForward(const FVector& UpVector, const FVector& ForwardVector);
	static FVector GenerateRight(const FVector& UpVector, const FVector& ForwardVector);
	static bool IsPerpendicular(const FVector& Vector1, const FVector& Vector2);

private:
	static bool IsTooClose(const FVector& Vector1, const FVector& Vector2);
	static FVector MakeVectorPerpendicular(const FVector& ForwardVector, const FVector& UpVector);
};


