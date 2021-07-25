// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/EquippableItem.h"
#include "WearItem.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class GAPGAME_API UWearItem : public UEquippableItem
{
	GENERATED_BODY()

public:

	UWearItem();


	virtual bool Equip(class APlayerManager* player) override; // this is how an override looks in C++
	virtual bool UnEquip(class APlayerManager* player) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wear")
	class USkeletalMesh* Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wear")
	class UMaterialInstance* MaterialInstance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gear", meta = (ClampMin = 0.0, ClampMax = 1.0))
	float DamageDefenceMultiplier;

};
