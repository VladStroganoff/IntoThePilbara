// Fill out your copyright notice in the Description page of Project Settings.


#include "FoodItem.h"
#include "SurvyvalPlayerController.h"
#include "PlayerManager.h"
#include "Components/InventoryComponent.h"

#define LOCTEXT_NAMESPACE "FoodItem"

UFoodItem::UFoodItem()
{
	HealAmount = 20.0f;
	UseActionText = LOCTEXT("ItemUseActionText", "Consume");
}


void UFoodItem::Use(class APlayerManager* player)
{
	if (player)
	{
		const float actualHealedAmount = player->ModifyHealth(HealAmount);
		const bool bUsedFood = !FMath::IsNearlyZero(actualHealedAmount);
		if (player)
		{
			if (!player->HasAuthority())
			{
				if (ASurvyvalPlayerController* playerControl = Cast<ASurvyvalPlayerController>(player->GetController()))
				{
					if (bUsedFood)
					{
						playerControl->ShowNotification(FText::Format(LOCTEXT("AteFoodText", "Ate {FoodItem}, healed {HealAmount} health."), ItemDisplayName, actualHealedAmount));
					}
					else
					{
						playerControl->ShowNotification(FText::Format(LOCTEXT("FullHealthText", "No need to eat {FoodItem}, health is already full."), ItemDisplayName));
					}
				}
			}
		}
		if (bUsedFood)
		{
			if(UInventoryComponent* inventory = player->PlayerInventory)
			{
				inventory->ConsumeItem(this, 1);
			}
		}
	}
	

}

#undef LOCTEXT_NAMESPACE