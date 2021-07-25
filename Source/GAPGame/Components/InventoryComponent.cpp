#include "InventoryComponent.h"
#include "Components/InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"

#define LOCTEXT_NAMESPACE "Inventory"

UInventoryComponent::UInventoryComponent()
{
	SetIsReplicated(true);
}

FItemAddResult UInventoryComponent::TryAddItem(UItem * Item)
{
	return TryAddItem_Internal(Item);
}

FItemAddResult UInventoryComponent::TryAddItemFromClass(TSubclassOf<class UItem> itemClass, const int32 quanity)
{
	UItem* item = NewObject<UItem>(GetOwner(), itemClass);
	item->SetQuantity(quanity);
	return TryAddItem_Internal(item);
}

int32 UInventoryComponent::ConsumeItem(UItem * item)
{
	if (item)
	{
		ConsumeItem(item, item->GetQuantity());
	}
	return 0;
}

int32 UInventoryComponent::ConsumeItem(UItem * item, const int32 quantity)
{
	if (GetOwner() && GetOwner()->HasAuthority() && item) // server? lol
	{
		const int32 removeQuanity = FMath::Min(quantity, item->GetQuantity());

		ensure(!(item->GetQuantity() - removeQuanity < 0));

		item->SetQuantity(item->GetQuantity() - removeQuanity);

		if (item->GetQuantity() <= 0)
		{
			RemoveItem(item);
		}
		else
		{
			ClientRefreshInventory();
		}

		return removeQuanity;
	}
	return 0;
}

bool UInventoryComponent::RemoveItem(UItem * item)
{
	if (GetOwner() && GetOwner()->HasAuthority()) // em I the server?
	{
		if (item)
		{
			Items.RemoveSingle(item);
			ReplicateItemsKey++;
			return true;
		}
	}

	return false;
}

bool UInventoryComponent::HasItem(TSubclassOf<class UItem> itemClass, const int32 quanity) const
{
	if (UItem* itemToFind = FindItemByClass(itemClass))
	{
		return itemToFind->GetQuantity() >= quanity;
	}
	return false;
}

UItem * UInventoryComponent::FindItem(UItem * item) const
{
	if (item)
	{
		for (auto& inventiryItem : Items)
		{
			if (inventiryItem && inventiryItem->GetClass() == item->GetClass())
			{
				return inventiryItem;
			}
		}
	}
	return nullptr;
}

UItem * UInventoryComponent::FindItemByClass(TSubclassOf<class UItem> itemClass) const
{
	for (auto& inventiryItem : Items)
	{
		if (inventiryItem && inventiryItem->GetClass() == itemClass)
		{
			return inventiryItem;
		}
	}
	return nullptr;
}

TArray<UItem*> UInventoryComponent::FindItemsByClass(TSubclassOf<class UItem> itemClass) const
{
	TArray<UItem*> itemsOClass;

	for (auto& inventiryItem : Items)
	{
		if (inventiryItem && inventiryItem->GetClass()->IsChildOf(itemClass))
		{
			itemsOClass.Add(inventiryItem);
		}
	}
	return itemsOClass;
}

float UInventoryComponent::GetCurrentWheight() const
{
	float wheight = 0.0f;

	for (auto& item : Items)
	{
		wheight += item->GetStackWeight();
	}

	return wheight;
}

void UInventoryComponent::SetWeightCapacity(const float newWeightCapacity)
{
	WeightCapacity = newWeightCapacity;
	OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::SetCapacity(const int32 newCapacity)
{
	Capacity = newCapacity;
	OnInventoryUpdated.Broadcast();
}


void UInventoryComponent::ClientRefreshInventory_Implementation()
{
	OnInventoryUpdated.Broadcast();
}


void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryComponent, Items);
}

bool UInventoryComponent::ReplicateSubobjects(UActorChannel *channel, FOutBunch *bunch, FReplicationFlags *repFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(channel, bunch, repFlags);

	if (channel->KeyNeedsToReplicate(0, ReplicateItemsKey))
	{
		for (auto& item : Items)
		{
			if (channel->KeyNeedsToReplicate(item->GetUniqueID(), item->RepKey))
			{
				bWroteSomething |= channel->ReplicateSubobject(item, *bunch, *repFlags);
			}

		}
	}

	return bWroteSomething;
}

UItem * UInventoryComponent::AddItem(UItem * item)
{
	if (GetOwner() && GetOwner()->HasAuthority()) // copy item and assign the recieving inventory as the owner
	{
		UItem* newItem = NewObject<UItem>(GetOwner(), item->GetClass());
		newItem->World = GetWorld();
		newItem->SetQuantity(item->GetQuantity());
		newItem->OwningInventory = this;
		newItem->AddedToInventry(this);
		Items.Add(newItem);
		newItem->MarkDirtyForReplication();
		return newItem;
	}

	return nullptr;
}

void UInventoryComponent::OnRep_Items()
{
	OnInventoryUpdated.Broadcast();


	for (auto& item : Items)
	{
		if (item &&  !item->World)
		{
			item->World = GetWorld();
		}
	}
}

FItemAddResult UInventoryComponent::TryAddItem_Internal(UItem * Item)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		const int32 addAmount = Item->GetQuantity();

		if (Items.Num() + 1 > GetCapacity())
		{
			return FItemAddResult::AddedNone(addAmount, LOCTEXT("InventryCapacityFullText", "Coundn't add item to Inventory. Inventory is full"));
		}

		if (!FMath::IsNearlyZero(Item->Weight))
		{
			if (GetCurrentWheight() + Item->Weight > GetWeightCapacity())
			{
				return FItemAddResult::AddedNone(addAmount, LOCTEXT("InventoryTooMuchWheightText", "Couldn't add item to inventory. Carrying too much wheight"));
			}
		}

		if (Item->bStackable)
		{
			ensure(Item->GetQuantity() <= Item->MaxStackSize); // break for debug 

			if (UItem* existingItem = FindItem(Item))
			{
				if (existingItem->GetQuantity() < existingItem->MaxStackSize)
				{
					const int32 capacityMaxAddedAmount = existingItem->MaxStackSize - existingItem->GetQuantity();
					int32 actualAddAmount = FMath::Min(addAmount, capacityMaxAddedAmount);

					FText errorText = LOCTEXT("InventoryErrorText", "Couldn't add all of the item to your inventory");

					if (!FMath::IsNearlyZero(Item->Weight))
					{
						const int32 wheightMaxAdded = FMath::FloorToInt((WeightCapacity - GetCurrentWheight()) / Item->Weight);
						actualAddAmount = FMath::Min(actualAddAmount, wheightMaxAdded);

						if (actualAddAmount < addAmount)
						{
							errorText = FText::Format(LOCTEXT("InventoryTooMuchWheightText", "Couldn't add entire stack of {ItemName} to inventory."), Item->ItemDisplayName);
						}
					}
					else if (actualAddAmount < addAmount)
					{
						errorText = FText::Format(LOCTEXT("InventoryTooMuchWheightText", "Couldn't add entire stack of {ItemName} to inventory. Inventory was full."), Item->ItemDisplayName);
					}

					if (actualAddAmount <= 0)
					{
						return FItemAddResult::AddedNone(addAmount, LOCTEXT("InventoryErrorText", "Couldn't add item to inventory."));
					}

					existingItem->SetQuantity(existingItem->GetQuantity() + actualAddAmount);
					ensure(existingItem->GetQuantity() <= existingItem->MaxStackSize);

					if (actualAddAmount < addAmount)
					{
						return FItemAddResult::AddedSome(addAmount, actualAddAmount, errorText);
					}
					else
					{
						return FItemAddResult::AddedAll(addAmount);
					}

				}
				else
				{
					return FItemAddResult::AddedNone(addAmount, FText::Format(LOCTEXT("InventiryFullStackText", "Couldn't add {itemName}. You already have a full stack of this item"), Item->ItemDisplayName));
				}
			}
			else
			{
				AddItem(Item);

				return FItemAddResult::AddedAll(addAmount);
			}

		}
		else
		{
			ensure(Item->GetQuantity() == 1);
			AddItem(Item);
			return FItemAddResult::AddedAll(addAmount);
		}


	}
	check(false);
	return FItemAddResult::AddedNone(-1, LOCTEXT("ErrorMessage", "I'm the client?"));
}

#undef LOCTEXT_NAMESPACE