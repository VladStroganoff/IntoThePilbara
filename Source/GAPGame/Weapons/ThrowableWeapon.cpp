// Fill out your copyright notice in the Description page of Project Settings.


#include "ThrowableWeapon.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AThrowableWeapon::AThrowableWeapon()
{
	ThrowableMesh = CreateDefaultSubobject<UStaticMeshComponent>("ThrowableMesh");
	SetRootComponent(ThrowableMesh);

	ThrowableMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ThrowableMovement");
	ThrowableMovement->InitialSpeed = 1000.0f;
	
	SetReplicates(true);
	SetReplicateMovement(true);
}

