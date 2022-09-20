// Copyright Epic Games, Inc. All Rights Reserved.

#include "BowProjectCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Weapon/Bow.h"
#include "Weapon/Arrow.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GamePlayStatics.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"

//////////////////////////////////////////////////////////////////////////
// ABowProjectCharacter

ABowProjectCharacter::ABowProjectCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rate for input
	TurnRateGamepad = 50.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	Spline = CreateDefaultSubobject<USplineComponent>("Spline");
}

//////////////////////////////////////////////////////////////////////////
// Input

void ABowProjectCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &ABowProjectCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &ABowProjectCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &ABowProjectCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &ABowProjectCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ABowProjectCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ABowProjectCharacter::TouchStopped);


	PlayerInputComponent->BindAction("Attack", IE_Pressed,this, &ABowProjectCharacter::Attacking);
	PlayerInputComponent->BindAction("Attack", IE_Released,this, &ABowProjectCharacter::StopAttacking);

	PlayerInputComponent->BindAction("Aiming", IE_Pressed, this, &ABowProjectCharacter::Aiming);
	PlayerInputComponent->BindAction("Aiming", IE_Released, this, &ABowProjectCharacter::StopAiming);
}

void ABowProjectCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void ABowProjectCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void ABowProjectCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void ABowProjectCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void ABowProjectCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ABowProjectCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void ABowProjectCharacter::BeginPlay()
{
	Super::BeginPlay();

	SpawnBow();
}

void ABowProjectCharacter::Tick(float DetalTime)
{
	Super::Tick(DetalTime);

	AddChargingVelocity();

	DestoryLine();
	DrawLine();
}

void ABowProjectCharacter::SpawnBow()
{
	if (!GetWorld())	return;

	Bow = GetWorld()->SpawnActor<ABow>(BowClass);
	auto BowMesh = Bow->GetBowMesh();

	FAttachmentTransformRules Rules(EAttachmentRule::SnapToTarget,false);

	BowMesh->AttachToComponent(this->GetMesh(), Rules, "SocketForBow");

	NowVelocity = Bow->GetBaseVelocity();
}

void ABowProjectCharacter::Attacking()
{
	if (!bAimingMode)	return;

	bCharging = true;
}

void ABowProjectCharacter::StopAttacking()
{
	if (bCanAttack && bArrowSpawned)
	{
		LaunchArrow();
	}

	bCharging = false;
	NowVelocity = Bow->GetBaseVelocity();
}

void ABowProjectCharacter::LaunchArrow()
{
	Arrow->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	auto Sphere = Arrow->GetSphere();
	auto Projectile = Arrow->GetProjectileMovement();

	Sphere->IgnoreActorWhenMoving(this,true);

	Projectile->ProjectileGravityScale = 1;
	Projectile->Velocity = NowFvectorVelocity;

	bArrowSpawned = false;

	bCanAttack = false;
	bDrawLine = false;

	auto BowTime = Bow->GetReloadTime();
	auto Time = DrawArrowAnim->GetTimeAtFrame(60);
	float Rate = Time / BowTime;

	DrawArrowAnim->RateScale = Rate;

	this->PlayAnimMontage(DrawArrowAnim);
}

void ABowProjectCharacter::AddChargingVelocity()
{
	if (!bCharging)	return;

	NowVelocity += Bow->GetChargingStep();

	NowVelocity = FMath::Clamp(NowVelocity,0, Bow->GetMaxVelocity());
}

void ABowProjectCharacter::Aiming()
{
	bAimingMode = true;

	auto BowTime = Bow->GetReloadTime();
	auto Time = DrawArrowAnim->GetTimeAtFrame(60);
	float Rate = Time / BowTime;

	DrawArrowAnim->RateScale = Rate;

	this->PlayAnimMontage(DrawArrowAnim);

	bUseControllerRotationYaw = true;
	Cast<UCharacterMovementComponent>(GetMovementComponent())->bOrientRotationToMovement = false;
}

void ABowProjectCharacter::StopAiming()
{
	this->StopAnimMontage();

	bAimingMode = false;
	bDrawLine = false;

	if (Arrow->IsAttachedTo(this))
		Arrow->Destroy();

	bUseControllerRotationYaw = false;
	Cast<UCharacterMovementComponent>(GetMovementComponent())->bOrientRotationToMovement = true;
}

void ABowProjectCharacter::DrawLine()
{
	if (!bDrawLine) return;

	FPredictProjectilePathParams Params;
	FPredictProjectilePathResult Result;

	NowFvectorVelocity = this->GetController()->GetControlRotation().Vector().GetSafeNormal();
	NowFvectorVelocity *= NowVelocity;

	Params.ActorsToIgnore.Add(this);
	Params.ActorsToIgnore.Add(Arrow);

	Params.StartLocation = Arrow->GetActorLocation();
	Params.MaxSimTime = 3.0f;
	Params.bTraceWithCollision = true;
	Params.LaunchVelocity = NowFvectorVelocity;
	Params.ProjectileRadius = 1.0f;
	Params.SimFrequency = 10;
	Params.DrawDebugType = EDrawDebugTrace::None;

	UGameplayStatics::PredictProjectilePath(Arrow, Params, Result);

	Spline->ClearSplinePoints();

	for (int32 i = 0; i < Result.PathData.Num(); i++)
	{
		Spline->AddSplinePoint(Result.PathData[i].Location, ESplineCoordinateSpace::Local, true);
	}

	FVector PrePointLocation;
	FVector NextPointLocation;
	FVector PrePointTangent;
	FVector NextPointTangent;

	for (size_t i = 0; i < Spline->GetNumberOfSplinePoints()-1; i++)
	{
		USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass());
		SplineMesh->CreationMethod = EComponentCreationMethod::UserConstructionScript;

		SplineMesh->SetStaticMesh(SplineStaticMesh);
		SplineMesh->SetForwardAxis(ESplineMeshAxis::X, false);

		PrePointLocation = Spline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Local);
		PrePointTangent = Spline->GetTangentAtSplinePoint(i, ESplineCoordinateSpace::Local);
		NextPointLocation = Spline->GetLocationAtSplinePoint(i + 1, ESplineCoordinateSpace::Local);
		NextPointTangent = Spline->GetTangentAtSplinePoint(i + 1, ESplineCoordinateSpace::Local);

		SplineMesh->SetStartAndEnd(PrePointLocation, PrePointTangent, NextPointLocation, NextPointTangent);

		SplineMeshs.Add(SplineMesh);
	}

	RegisterAllComponents();
}

void ABowProjectCharacter::DestoryLine()
{
	for (size_t i = 0; i < SplineMeshs.Num(); i++)
	{
		SplineMeshs[i]->DestroyComponent();
	}

	SplineMeshs.Empty();
}

void ABowProjectCharacter::WhenDrawArrowNotity()
{
	if (!GetWorld())	return;

	Arrow = GetWorld()->SpawnActor<AArrow>(ArrowClass);
	auto ArrowShpere = Arrow->GetSphere();

	FAttachmentTransformRules Rules(EAttachmentRule::SnapToTarget, false);

	ArrowShpere->AttachToComponent(this->GetMesh(), Rules, "SocketForArrow");

	bArrowSpawned = true;
}

void ABowProjectCharacter::WhenDrawArrowEndNotity()
{
	bDrawLine = true;
	bCanAttack = true;
}
