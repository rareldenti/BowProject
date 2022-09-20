// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Arrow.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"

// Sets default values
AArrow::AArrow()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(1.0f);

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// 当碰撞发生时，调用 OnHit 函数
	CollisionComp->OnComponentHit.AddDynamic(this, &AArrow::OnHit);

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;

	// 默认不进行模拟，只有我们在接受到攻击后，才进行物理模拟
	ProjectileMovement->Velocity = (FVector(0, 0, 0));
	ProjectileMovement->ProjectileGravityScale = 0;
	ProjectileMovement->bRotationFollowsVelocity = true;

	ArrowMesh = CreateDefaultSubobject<UStaticMeshComponent>("ArrowMesh");
	ArrowMesh->SetupAttachment(CollisionComp);

	SetRootComponent(CollisionComp);
}

void AArrow::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	UE_LOG(LogTemp, Error, TEXT("HIT -> %s !"), *Hit.GetActor()->GetName());
}
