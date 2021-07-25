// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"
#include "Components/InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#define LOCTEXT_NAMESPACE "Item"

UItem::UItem()
{
	ItemDisplayName = LOCTEXT("ItemName", "Item");
	UseActionText = LOCTEXT("ItemUseActionText", "Use");
	Weight = 0;
	bStackable = true;
	Quantity = 1;
	MaxStackSize = 2;
	RepKey = 0;
}

void UItem::OnRep_Quantity()
{
	OnItemModified.Broadcast();
}

void UItem::SetQuantity(const int32 newQuantity)
{
	if (newQuantity != Quantity)
	{
		Quantity = FMath::Clamp(newQuantity, 0, bStackable ? MaxStackSize : 1);
		MarkDirtyForReplication();
	}
}

bool UItem::ShouldShowInInventory() const
{
	return true;
}

void UItem::Use(APlayerManager * player)
{
}

void UItem::AddedToInventry(UInventoryComponent * Inventory)
{
}

void UItem::MarkDirtyForReplication()
{
	++RepKey;

	if (OwningInventory)
	{
		++OwningInventory->ReplicateItemsKey;
	}
}


void UItem::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UItem, Quantity);
}

bool UItem::IsSupportedForNetworking() const
{
	return true;
}

UWorld * UItem::GetWorld() const
{
	return World;
}

#if WITH_EDITOR
void UItem::PostEditChangeProperty(FPropertyChangedEvent & propertyChangedEvent)
{
	Super::PostEditChangeProperty(propertyChangedEvent);

	FName ChangePropertyName = propertyChangedEvent.Property ? propertyChangedEvent.Property->GetFName() : NAME_None;

	if (ChangePropertyName == GET_MEMBER_NAME_CHECKED(UItem, Quantity))
	{
		Quantity = FMath::Clamp(Quantity, 1, bStackable ? MaxStackSize : 1);
	}

}
#endif
#undef LOCTEXT_NAMESPACE