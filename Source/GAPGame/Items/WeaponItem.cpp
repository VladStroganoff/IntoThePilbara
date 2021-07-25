// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponItem.h"
#include "SurvyvalPlayerController.h"
#include "PlayerManager.h"


UWeaponItem::UWeaponItem()
{

}

bool UWeaponItem::Equip(APlayerManager * player)
{
	bool bEquipSuccessful = Super::Equip(player);

	if (bEquipSuccessful && player)
	{
		player->EquipWeapon(this);
	}
	return bEquipSuccessful;
}

bool UWeaponItem::UnEquip(APlayerManager * player)
{
	bool bEquipSuccessful = Super::Equip(player);

	if (bEquipSuccessful && player)
	{
		player->UnEquipWeapon();
	}
	return bEquipSuccessful;
}
