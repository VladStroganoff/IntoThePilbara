// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup.h"
#include "Items/Item.h"
#include "Net/UnrealNetwork.h"
#include "Player/PlayerManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InteractionComponent.h"
#include "Components/InventoryComponent.h"
#include "Engine/ActorChannel.h"


// Sets default values
APickup::APickup()
{
	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>("PickupMesh");
	PickupMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	SetRootComponent(PickupMesh);
	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>("PickupInteractionComponent");
	InteractionComponent->InteractionTime = 0.5f;
	InteractionComponent->InteractionDistance = 200.0f;
	InteractionComponent->InteractionNameText = FText::FromString("Pickup");
	InteractionComponent->InteractionActionText = FText::FromString("Take");
	InteractionComponent->OnInteract.AddDynamic(this, &APickup::OnTakePickup);
	InteractionComponent->SetupAttachment(PickupMesh);
	SetReplicates(true); // turn onnetworking setup
}

void APickup::InitializePickup(const TSubclassOf<class UItem> itemClass, const int32 quanity)
{
	if (HasAuthority() && itemClass && quanity > 0)
	{
		Item = NewObject<UItem>(this, itemClass);
		Item->SetQuantity(quanity);
		OnRep_Item();
		Item->MarkDirtyForReplication();
	}
}


void APickup::OnRep_Item()
{
	if (Item)
	{
		PickupMesh->SetStaticMesh(Item->PickupMesh);
		InteractionComponent->InteractionNameText = Item->ItemDisplayName;
		Item->OnItemModified.AddDynamic(this, &APickup::OnItemModefied);
	}

	InteractionComponent->RefreshWidget();
}

void APickup::OnItemModefied()
{
	if (InteractionComponent)
	{
		InteractionComponent->RefreshWidget();
	}
}

// Called when the game starts or when spawned
void APickup::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && ItemTemplate && bNetStartup)
	{
		InitializePickup(ItemTemplate->GetClass(), ItemTemplate->GetQuantity());
	}

	if (bNetStartup) // was it spawned by world or by player?
	{
		AlignWithGround();
	}

	if (Item)
	{
		Item->MarkDirtyForReplication();
	}
}


void APickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APickup, Item);
}

bool APickup::ReplicateSubobjects(UActorChannel * channel, FOutBunch * bunch, FReplicationFlags * repFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(channel, bunch, repFlags);
	if (Item && channel->KeyNeedsToReplicate(Item->GetUniqueID(), Item->RepKey))
	{
		bWroteSomething |= channel->ReplicateSubobject(Item, *bunch, *repFlags);
	}
	return bWroteSomething;

}


#if WITH_EDITOR
void APickup::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName propertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (propertyName == GET_MEMBER_NAME_CHECKED(APickup, ItemTemplate))
	{
		if (ItemTemplate)
		{
			PickupMesh->SetStaticMesh(ItemTemplate->PickupMesh);
		}
	}

}
#endif

void APickup::OnTakePickup(APlayerManager * taker)
{
	if (!taker)
	{
		UE_LOG(LogTemp, Warning, TEXT("Pickup was taken but player was not valid"));
	}

	if (HasAuthority() && !IsPendingKillPending() && Item)
	{
		if (UInventoryComponent* PlayerInventory = taker->PlayerInventory)
		{
			const FItemAddResult addResult = PlayerInventory->TryAddItem(Item);

			if (addResult.ActualAmountGiven < Item->GetQuantity())
			{
				Item->SetQuantity(Item->GetQuantity() - addResult.ActualAmountGiven);
			}
			else if (addResult.ActualAmountGiven >= Item->GetQuantity())
			{
				Destroy();
			}

		}
	}

}


