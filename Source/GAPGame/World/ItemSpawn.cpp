// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemSpawn.h"
#include "World/Pickup.h"
#include "Items/Item.h"

AItemSpawn::AItemSpawn()
{
	PrimaryActorTick.bCanEverTick = false;
	bNetLoadOnClient = false;

	RespawnRange = FIntPoint(10, 30);
}

void AItemSpawn::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SpawnItem();
	}
}

void AItemSpawn::SpawnItem()
{
	if (HasAuthority() && LootTable)
	{
		TArray<FLootTableRow*> spawnItems;
		LootTable->GetAllRows("", spawnItems);

		const FLootTableRow* lootRow = spawnItems[FMath::RandRange(0, spawnItems.Num() - 1)];

		ensure(lootRow);

		float probabilityRoll = FMath::FRandRange(0.0f, 1.0f);

		while(probabilityRoll > lootRow->Probability)
		{
			lootRow = spawnItems[FMath::RandRange(0, spawnItems.Num() - 1)];
			probabilityRoll = FMath::FRandRange(0.0f, 1.0f);
		}

		if (lootRow && lootRow->Items.Num() && PickupClass)
		{
			float angle = 0.0f;

			for (auto& itemClass : lootRow->Items)
			{
				const FVector locationOffset = FVector(FMath::Cos(angle), FMath::Sin(angle), 0.0f) * 50.0f;

				FActorSpawnParameters spawnParams;
				spawnParams.bNoFail = true;
				spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

				const int32 itemQuanty = itemClass->GetDefaultObject<UItem>()->GetQuantity();

				FTransform spawnTransform = GetActorTransform();
				spawnTransform.AddToTranslation(locationOffset);

				APickup* pickup = GetWorld()->SpawnActor<APickup>(PickupClass, spawnTransform, spawnParams);
				pickup->InitializePickup(itemClass, itemQuanty);
				pickup->OnDestroyed.AddUniqueDynamic(this, &AItemSpawn::OnItemTaken);

				SpawnedPickups.Add(pickup);

				angle += (PI * 2.0f) / lootRow->Items.Num();
			}

		}


	}
}

void AItemSpawn::OnItemTaken(AActor * destroyedActor)
{
	if (HasAuthority())
	{
		SpawnedPickups.Remove(destroyedActor);

		if (SpawnedPickups.Num() <= 0)
		{
			GetWorldTimerManager().SetTimer(TimerHandle_RespawnItem, this, &AItemSpawn::SpawnItem, FMath::RandRange(RespawnRange.GetMin(), RespawnRange.GetMax()), false);
		}
	}

}
