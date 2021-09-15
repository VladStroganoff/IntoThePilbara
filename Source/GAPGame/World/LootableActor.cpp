// Fill out your copyright notice in the Description page of Project Settings.


#include "LootableActor.h"
#include "Components/InteractionComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DataTable.h"
#include "Items/EquippableItem.h"
#include "World/ItemSpawn.h"
#include "Items/Item.h"
#include "Player/PlayerManager.h"

#define LOCTEXT_NAMESPACE "LootableActor"

// Sets default values
ALootableActor::ALootableActor()
{
	LootContainerMesh = CreateDefaultSubobject<UStaticMeshComponent>("LootContainerMesh");
	SetRootComponent(LootContainerMesh);

	LootInteraction = CreateDefaultSubobject<UInteractionComponent>("LootInteraction");
	LootInteraction->InteractionActionText = LOCTEXT("LootActionText", "Loot");
	LootInteraction->InteractionNameText = LOCTEXT("LootActionName", "Cest");
	LootInteraction->SetupAttachment(GetRootComponent());

	Inventory = CreateDefaultSubobject<UInventoryComponent>("Inventory");
	Inventory->SetCapacity(20);
	Inventory->SetWeightCapacity(80.0f);

	LootRolls = FIntPoint(2,8);

	SetReplicates(true);
}

// Called when the game starts or when spawned
void ALootableActor::BeginPlay()
{
	Super::BeginPlay();

	LootInteraction->OnInteract.AddDynamic(this, &ALootableActor::OnInteraction);

	if (HasAuthority() && LootTable)
	{
		TArray<FLootTableRow*> SpawnItems;
		LootTable->GetAllRows("", SpawnItems);
		int32 rolls = FMath::RandRange(LootRolls.GetMin(), LootRolls.GetMax());

		for (int32 i = 0; i < rolls; i++)
		{
			const FLootTableRow* lootRow = SpawnItems[FMath::RandRange(0, SpawnItems.Num() - 1)];
			
			ensure(lootRow);

			float probablityRoll = FMath::FRandRange(0, SpawnItems.Num() - 1);

			while (probablityRoll > lootRow->Probability)
			{
				lootRow = SpawnItems[FMath::RandRange(0, SpawnItems.Num() - 1)];
				probablityRoll = FMath::FRandRange(0.0f, 1.0f);
			}

			if (lootRow && lootRow->Items.Num())
			{
				for (auto& itemClass : lootRow->Items)
				{
					if (itemClass)
					{
						const int32 quanity = Cast<UItem>(itemClass->GetDefaultObject())->GetQuantity();
						Inventory->TryAddItemFromClass(itemClass, quanity);
					}
				}
			}
		}
	}


	
}

void ALootableActor::OnInteraction(class APlayerManager* player)
{
	if (player)
	{
		player->SetLootSource(Inventory);
	}
}

#undef LOCTEXT_NAMESPACE