// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Arrow.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

UCLASS()
class BOWPROJECT_API AArrow : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AArrow();

	UStaticMeshComponent* GetArrowMesh() { return ArrowMesh; };
	USphereComponent* GetSphere() { return CollisionComp; };
	UProjectileMovementComponent* GetProjectileMovement() { return ProjectileMovement; };

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UStaticMeshComponent* ArrowMesh;

	UPROPERTY(VisibleDefaultsOnly)
		USphereComponent* CollisionComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UProjectileMovementComponent* ProjectileMovement;
};
