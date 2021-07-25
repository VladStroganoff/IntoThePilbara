// Fill out your copyright notice in the Description page of Project Settings.


#include "WearItem.h"
#include "PlayerManager.h"

UWearItem::UWearItem()
{
	DamageDefenceMultiplier = 0.1f;
}


bool UWearItem::Equip(APlayerManager * player)
{
	bool bEquipSuccessful = Super::Equip(player);

	if (bEquipSuccessful && player)
	{
		player->EquipWear(this);
	}
	return bEquipSuccessful;
}

bool UWearItem::UnEquip(APlayerManager * player)
{
	bool bEquipSuccessful = Super::UnEquip(player);

	if (bEquipSuccessful && player)
	{
		player->UnEquipWear(Slot);
	}
	return bEquipSuccessful;
}
