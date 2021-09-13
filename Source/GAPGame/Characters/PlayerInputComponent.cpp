// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/InventoryComponent.h"
#include "Items/ThrowableItem.h"
#include "Items/EquippableItem.h"
#include "Weapons/ThrowableWeapon.h"
#include "Characters/PlayerInputComponent.h"

// Sets default values for this component's properties
UPlayerInputComponent::UPlayerInputComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPlayerInputComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UPlayerInputComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


void UPlayerInputComponent::ServerUseThrowable_Implementation()
{
	UseThrowable();
}

void UPlayerInputComponent::MulticastPlayThrowableTossFX_Implementation(UAnimMontage* montageToPlay)
{
	if (GetNetMode() != NM_DedicatedServer && !_playerManager->IsLocallyControlled())
	{
		_playerManager->PlayAnimMontage(montageToPlay);
	}
}



void UPlayerInputComponent::UseThrowable()
{
	if (_playerManager->CanUseThrowable())
	{
		if (UThrowableItem* throwable = _playerManager->GetThrowable())
		{
			if (_playerManager->HasAuthority())
			{
				SpawnThrowable();
				if (_playerManager->PlayerInventory)
				{
					_playerManager->PlayerInventory->ConsumeItem(throwable, 1);
				}
			}
			else
			{
				if (throwable->GetQuantity() <= 1)
				{
					EquippedItems.Remove(EEquippableSlot::EIS_ThrowbleItem);
					_playerManager->OnEquippedItemsChanged.Broadcast(EEquippableSlot::EIS_ThrowbleItem, nullptr);
				}

				_playerManager->PlayAnimMontage(throwable->ThrowableTossAnimation);
				ServerUseThrowable();
			}
		}
	}
}

void UPlayerInputComponent::SpawnThrowable()
{
	if (_playerManager->HasAuthority())
	{
		if (UThrowableItem* currentThrowable = _playerManager->GetThrowable())
		{
			if (currentThrowable->ThrowableClass)
			{
				FActorSpawnParameters spawnParams;
				spawnParams.Owner = spawnParams.Instigator = _playerManager;
				spawnParams.bNoFail = true;
				spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

				FVector camLocation;
				FRotator camRotation;

				_playerManager->GetController()->GetPlayerViewPoint(camLocation, camRotation);

				camLocation = (camRotation.Vector()* 20.0f) + camLocation;

				if (AThrowableWeapon* throwableWeapon = GetWorld()->SpawnActor<AThrowableWeapon>(currentThrowable->ThrowableClass, FTransform(camRotation, camLocation)))
				{
					MulticastPlayThrowableTossFX(currentThrowable->ThrowableTossAnimation);
				}
			}
		}
	}
}


