// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BowProjectCharacter.generated.h"

class ABow;
class AArrow;
class USplineComponent;
class USplineMeshComponent;

UCLASS(config=Game)
class ABowProjectCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	ABowProjectCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Input)
	float TurnRateGamepad;

protected:

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }


public:
	virtual void BeginPlay() override;
	virtual void Tick(float DetalTime) override;

	void SpawnBow();
	
	void Attacking();
	void StopAttacking();
	void LaunchArrow();

	void AddChargingVelocity();

	void Aiming();
	void StopAiming();

	void DrawLine();
	void DestoryLine();

	UFUNCTION(BlueprintCallable)
	void WhenDrawArrowNotity();
	UFUNCTION(BlueprintCallable)
	void WhenDrawArrowEndNotity();

protected:
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TSubclassOf<ABow> BowClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AArrow> ArrowClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* DrawArrowAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USplineComponent* Spline;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UStaticMesh* SplineStaticMesh;
private:
	ABow* Bow;
	AArrow* Arrow;

	bool bAimingMode = false;
	bool bCharging = false;
	bool bCanAttack = true;
	bool bArrowSpawned = false;
	bool bDrawLine = false;

	float NowVelocity;
	FVector NowFvectorVelocity;

	TArray<USplineMeshComponent*> SplineMeshs;
};

