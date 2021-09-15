// Fill out your copyright notice in the Description page of Project Settings.


#include "EquippableItem.h"
#include "Net/UnrealNetwork.h"
#include "Player/PlayerManager.h"
#include "Components/InventoryComponent.h"

#define LOCTEXT_NAMESPACE "EquippableItem"

UEquippableItem::UEquippableItem()
{
	bStackable = false;
	bEquipped = false;
	UseActionText = LOCTEXT("ItemUseActionText", "Equip");
}


void UEquippableItem::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UEquippableItem, bEquipped);
}

void UEquippableItem::Use(APlayerManager * player)
{
	if (player && player->HasAuthority())
	{

		if (player->GetEquippedItems().Contains(Slot) && !bEquipped)
		{
			UEquippableItem* alreadyEquippedItem = *player->GetEquippedItems().Find(Slot);
			alreadyEquippedItem->SetEquipped(false);
		}

		SetEquipped(!IsEquipped());
	}
}

bool UEquippableItem::Equip(APlayerManager * player)
{
	if (player)
	{
		return player->EquipItem(this);
	}
	return false;
}

bool UEquippableItem::UnEquip(APlayerManager * player)
{
	if (player)
	{
		return player->UnEquipItem(this);
	}
	return false;
}

bool UEquippableItem::ShouldShowInInventory() const
{
	return !bEquipped;
}

void UEquippableItem::AddedToInventory(UInventoryComponent * inventory)
{
	if (APlayerManager* player = Cast<APlayerManager>(inventory->GetOwner()))
	{
		if (player && !player->IsLooting())
		{
			if (!player->GetEquippedItems().Contains(Slot))
			{
				SetEquipped(true);
			}
		}
	}
}

void UEquippableItem::SetEquipped(bool bNewEquipped)
{
	bEquipped = bNewEquipped;
	EquipmentStatusChanged();
	MarkDirtyForReplication();
}

void UEquippableItem::EquipmentStatusChanged()
{
	if (APlayerManager* player = Cast<APlayerManager>(GetOuter()))
	{
		if (bEquipped)
		{
			Equip(player);
		}
		else
		{
			UnEquip(player);
		}
	}

	OnItemModified.Broadcast();

}

#undef LOCTEXT_NAMESPACE