// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "EquippableItem.generated.h"

UENUM(BlueprintType)
enum class EEquippableSlot : uint8
{
	EIS_Head UMETA(DisplayName = "Head"),
	EIS_Torso UMETA(DisplayName = "Torso"),
	EIS_LeftArm UMETA(DisplayName = "Left Arm"),
	EIS_RightArm UMETA(DisplayName = "Right Arm"),
	EIS_Legs UMETA(DisplayName = "Legs"),
	EIS_Jacket UMETA(DisplayName = "Jacket"),
	EIS_Pants UMETA(DisplayName = "Pants"),
	EIS_Backpack UMETA(DisplayName = "Backpack"),
	EIS_Sensor UMETA(DisplayName = "Sensor"),
	EIS_PrimaryWeapon UMETA(DisplayName = "Primary Weapon"),
	EIS_ThrowbleItem UMETA(DisplayName = "Throwable Item")
};

/**
 * 
 */
UCLASS(Abstract, NotBlueprintable)
class GAPGAME_API UEquippableItem : public UItem
{
	GENERATED_BODY()

public:

	UEquippableItem();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equippables")
	EEquippableSlot Slot;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void Use(class APlayerManager* player);

	UFUNCTION(BlueprintCallable, Category = "Equippables")
	virtual bool Equip(class APlayerManager* player);

	UFUNCTION(BlueprintCallable, Category = "Equippables")
	virtual bool UnEquip(class APlayerManager* player);

	virtual bool ShouldShowInInventory() const override;

	virtual void AddedToInventory(class UInventoryComponent* inventory);

	UFUNCTION(BlueprintPure, Category = "Equippables")
	bool IsEquipped() { return bEquipped; };

	void SetEquipped(bool bNewEquipped);

protected:
	UPROPERTY(ReplicatedUsing = EquipmentStatusChanged)
	bool bEquipped;

	UFUNCTION()
	void EquipmentStatusChanged();
	
};
