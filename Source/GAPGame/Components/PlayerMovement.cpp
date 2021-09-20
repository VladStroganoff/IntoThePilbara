
#include "Net/UnrealNetwork.h"
#include "Components/InteractionComponent.h"	
#include "Weapons/Weapon.h"
#include "Components/InventoryComponent.h"
#include "Items/ThrowableItem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"	
#include "Components/PlayerMovement.h"
#include "PlayerMovement.h"

UPlayerMovement::UPlayerMovement()
{
	PrimaryComponentTick.bCanEverTick = true;

}


void UPlayerMovement::BeginPlay()
{
	Super::BeginPlay();
}



void UPlayerMovement::SetuInput(UInputComponent* PlayerInputComponent)
{
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &UPlayerMovement::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &UPlayerMovement::StopFire);

	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &UPlayerMovement::StartAiming);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &UPlayerMovement::StopAiming);
	PlayerInputComponent->BindAction("Throw", IE_Released, this, &UPlayerMovement::UseThrowable);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &UPlayerMovement::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &UPlayerMovement::StopJumping);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &UPlayerMovement::BeginInteract);
	PlayerInputComponent->BindAction("Interact", IE_Released, this, &UPlayerMovement::EndInteract);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &UPlayerMovement::StartCrouching);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &UPlayerMovement::StopCrouching);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &UPlayerMovement::StartSprinting);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &UPlayerMovement::StopSprinting);

	PlayerInputComponent->BindAxis("MoveForward", this, &UPlayerMovement::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &UPlayerMovement::MoveRight);
}


void UPlayerMovement::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPlayerMovement, bSprinting);
	DOREPLIFETIME_CONDITION(UPlayerMovement, bIsAiming, COND_SkipOwner);
}

void UPlayerMovement::Inject(APlayerManager* playerManager)
{
	PlayerManager = playerManager;
	SprintSpeed = PlayerManager->GetCharacterMovement()->MaxWalkSpeed * 2.f;
	WalkSpeed = PlayerManager->GetCharacterMovement()->MaxWalkSpeed;
}

void UPlayerMovement::StartCrouching()
{
	PlayerManager->Crouch();
}

void UPlayerMovement::StopCrouching()
{
	PlayerManager->UnCrouch();
}

void UPlayerMovement::MoveForward(float val)
{
	if (val == 0)
		return;

	PlayerManager->AddMovementInput(PlayerManager->GetActorForwardVector(), val);

}

void UPlayerMovement::MoveRight(float val)
{
	if (val == 0)
		return;

	PlayerManager->AddMovementInput(PlayerManager->GetActorRightVector(), val);
}

void UPlayerMovement::LookUp(float val)
{
	if (val == 0)
		return;

	PlayerManager->AddControllerPitchInput(val);
}

void UPlayerMovement::Turn(float val)
{
	if (val == 0)
		return;
	PlayerManager->AddControllerYawInput(val);
}

void UPlayerMovement::StartFire()
{
	if (PlayerManager->EquippedWeapon)
	{
		PlayerManager->EquippedWeapon->StartFire();
	}
	else
	{
		PlayerManager->BeginMeleeAttack();
	}
}

void UPlayerMovement::StopFire()
{
	if (PlayerManager->EquippedWeapon)
	{
		PlayerManager->EquippedWeapon->StopFire();
	}
}

void UPlayerMovement::BeginInteract()
{

	if (!PlayerManager->CheckAuthority())
	{
		ServerBeginInteract();
	}

	if (PlayerManager->HasAuthority())
	{
		PlayerManager->PreformInteractionCheck();
	}

	PlayerManager->InteractionData.bInteractHeld = true;

	if (UInteractionComponent* Interactable = PlayerManager->GetInteractable())
	{
		Interactable->BeginInteract(PlayerManager);
		if (FMath::IsNearlyZero(Interactable->InteractionTime))
		{
			PlayerManager->Interact();
		}
		else
		{
			PlayerManager->GetWorldTimerManager().SetTimer(PlayerManager->_timerHandleInteract, PlayerManager, &APlayerManager::Interact, Interactable->InteractionTime, false);
		}
	}
}

void UPlayerMovement::ServerBeginInteract_Implementation()
{
	BeginInteract();
}

bool UPlayerMovement::ServerBeginInteract_Validate()
{
	return true;
}

void UPlayerMovement::EndInteract()
{
	if (!PlayerManager->CheckAuthority())
		ServerEndInteract();

	PlayerManager->InteractionData.bInteractHeld = false;

	PlayerManager->GetWorldTimerManager().ClearTimer(PlayerManager->_timerHandleInteract);

	if (UInteractionComponent* Interactable = PlayerManager->GetInteractable())
	{
		Interactable->EndInteract(PlayerManager);
	}

}

void UPlayerMovement::ServerEndInteract_Implementation()
{
	EndInteract();
}

bool UPlayerMovement::ServerEndInteract_Validate()
{
	return true;
}

void UPlayerMovement::StartSprinting()
{
	SetSprinting(true);
}

void UPlayerMovement::StopSprinting()
{
	SetSprinting(false);
}

void UPlayerMovement::Jump()
{
	PlayerManager->Jump();
}

void UPlayerMovement::StopJumping()
{
	PlayerManager->StopJumping();
}

void UPlayerMovement::SetSprinting(const bool bNewSprinting)
{
	if ((bNewSprinting && !CanSprint()) || bNewSprinting == bSprinting)
	{
		return;
	}

	if (PlayerManager->CheckRole() < ROLE_Authority)
	{
		ServerSetSprinting(bNewSprinting);
	}

	bSprinting = bNewSprinting;

	PlayerManager->GetCharacterMovement()->MaxWalkSpeed = bSprinting ? SprintSpeed : WalkSpeed;
}

void UPlayerMovement::ServerSetSprinting_Implementation(const bool bNewSprinting)
{
	SetSprinting(bNewSprinting);
}

bool UPlayerMovement::ServerSetSprinting_Validate(const bool bNewSprinting)
{
	return true;
}

void UPlayerMovement::StartAiming()
{
	if (CanAim())
	{
		SetAiming(true);
	}
}

void UPlayerMovement::StopAiming()
{
	SetAiming(false);
}

void UPlayerMovement::SetAiming(const bool bNewAiming)
{
	if (bNewAiming && !CanAim() || bNewAiming == bIsAiming)
		return;

	if (PlayerManager->GetLocalRole() < ROLE_Authority)
	{
		ServerSetAiming(bNewAiming);
	}

	bIsAiming = bNewAiming;
}

void UPlayerMovement::ServerSetAiming_Implementation(const bool bNewAiming)
{
	SetAiming(bNewAiming);
}

bool UPlayerMovement::CanAim()
{
	return PlayerManager->EquippedWeapon != nullptr;
}


bool UPlayerMovement::CanSprint()
{
	return !IsAiming();
}

void UPlayerMovement::UseThrowable()
{
	if (PlayerManager->CanUseThrowable())
	{
		if (UThrowableItem * throwable = PlayerManager->GetThrowable())
		{
			if (PlayerManager->HasAuthority())
			{
				PlayerManager->SpawnThrowable();
				if (PlayerManager->PlayerInventory)
				{
					//UItem* pb = dynamic_cast<UItem*>(&throwable);
					PlayerManager->PlayerInventory->ConsumeItem(throwable, 1);
				}
			}
			else
			{
				if (throwable->GetQuantity() <= 1)
				{
					PlayerManager->EquippedItems.Remove(EEquippableSlot::EIS_ThrowbleItem);
					PlayerManager->OnEquippedItemsChanged.Broadcast(EEquippableSlot::EIS_ThrowbleItem, nullptr);
				}

				PlayerManager->PlayAnimMontage(throwable->ThrowableTossAnimation);
				ServerUseThrowable();
			}
		}
	}
}



void UPlayerMovement::ServerUseThrowable_Implementation()
{
	UseThrowable();
}