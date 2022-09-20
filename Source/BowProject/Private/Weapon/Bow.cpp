// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Bow.h"

// Sets default values
ABow::ABow()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	BowMesh = CreateDefaultSubobject<USkeletalMeshComponent>("BowMesh");
}

// Called when the game starts or when spawned
void ABow::BeginPlay()
{
	Super::BeginPlay();
	
}
