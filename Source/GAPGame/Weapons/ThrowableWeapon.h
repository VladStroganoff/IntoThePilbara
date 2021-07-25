// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThrowableWeapon.generated.h"

UCLASS()
class GAPGAME_API AThrowableWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AThrowableWeapon();

public:	
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	class UStaticMeshComponent* ThrowableMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Components")
	class UProjectileMovementComponent* ThrowableMovement;

};
