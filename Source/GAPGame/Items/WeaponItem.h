// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/EquippableItem.h"
#include "WeaponItem.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class GAPGAME_API UWeaponItem : public UEquippableItem
{
	GENERATED_BODY()

	UWeaponItem();

	virtual bool Equip(class APlayerManager* player) override;
	virtual bool UnEquip(class APlayerManager* player) override;
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<class AWeapon> WeaponClass;

};
