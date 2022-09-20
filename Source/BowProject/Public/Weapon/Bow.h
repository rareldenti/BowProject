// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Bow.generated.h"

UCLASS()
class BOWPROJECT_API ABow : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABow();

	USkeletalMeshComponent* GetBowMesh() { return BowMesh; };

	float GetBaseVelocity() { return BaseVelocity; };
	float GetMaxVelocity() { return MaxVelocity; };
	float GetChargingStep() { return ChargingStep; };
	float GetReloadTime() { return ReloadTime; };

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	USkeletalMeshComponent* BowMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float BaseVelocity = 1000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float MaxVelocity = 5000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float ChargingStep = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float ReloadTime = 1.0f;
};
