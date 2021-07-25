// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/EquippableItem.h"
#include "ThrowableItem.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class GAPGAME_API UThrowableItem : public UEquippableItem
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, Category = "Weapons")
	class UAnimMontage* ThrowableTossAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapons")
	TSubclassOf<class AThrowableWeapon> ThrowableClass;
};
