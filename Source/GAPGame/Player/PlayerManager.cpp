// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerManager.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"
#include "Player/SurvyvalPlayerController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/InteractionComponent.h"																								 
#include "Items/EquippableItem.h"																											 
#include "Items/WearItem.h"				
#include "Items/WeaponItem.h"
#include "Items/ThrowableItem.h"
#include "Materials/MaterialInstance.h"																										 
#include "World/Pickup.h"																													 
#include "GameFramework/PlayerState.h"																										 
#include "GameFramework/SpringArmComponent.h"																								 
#include "GameFramework/CharacterMovementComponent.h"																						 
#include "GameFramework/DamageType.h"																										 
#include "Weapons/MeleeDamage.h"
#include "Weapons/Weapon.h"
#include "Weapons/ThrowableWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "GameLogic/GAPGame.h"

#define LOCTEXT_NAMESPACE "SurvivalCharacter"
static FName NAME_AimDownSightsSocket("aimDownSightsSocket");

// Sets default values
APlayerManager::APlayerManager()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>("SpringArmComponent");
	SpringArmComponent->SetupAttachment(GetMesh(), FName("CameraSocket"));
	SpringArmComponent->TargetArmLength = 0.0f;


	CameraComponent = CreateDefaultSubobject<UCameraComponent>("CameraCOmponent");
	CameraComponent->SetupAttachment(SpringArmComponent);
	CameraComponent->bUsePawnControlRotation = true;

	PlayerMovement = CreateDefaultSubobject<UPlayerMovement>("Movement");
	PlayerMovement->Inject(this);
	UE_LOG(LogTemp, Warning, TEXT("DID INJECT PLAYER MOVEMENT, %d"), PlayerMovement->PlayerManager);


	HelmetMesh = PlayerMeshes.Add(EEquippableSlot::EIS_Sensor, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SensorMesh")));
	TorsoMesh = PlayerMeshes.Add(EEquippableSlot::EIS_Torso, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TorsoMesh")));
	RightArmMesh = PlayerMeshes.Add(EEquippableSlot::EIS_RightArm, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RightArmMesh")));
	LeftArmMesh = PlayerMeshes.Add(EEquippableSlot::EIS_LeftArm, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LeftArmMesh")));
	LegsMesh = PlayerMeshes.Add(EEquippableSlot::EIS_Legs, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LegsMesh")));
	JacketMesh = PlayerMeshes.Add(EEquippableSlot::EIS_Jacket, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("JacketMesh")));
	PantsMesh = PlayerMeshes.Add(EEquippableSlot::EIS_Pants, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PantsMesh")));
	BackpackMesh = PlayerMeshes.Add(EEquippableSlot::EIS_Backpack, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BackpackMesh")));

	for (auto& playerMesh : PlayerMeshes)
	{
		USkeletalMeshComponent* meshComponent = playerMesh.Value;
		meshComponent->SetupAttachment(GetMesh());
		meshComponent->SetMasterPoseComponent(GetMesh());
	}

	PlayerMeshes.Add(EEquippableSlot::EIS_Head, GetMesh());

	PlayerInventory = CreateDefaultSubobject<UInventoryComponent>("PlayerInventory");
	PlayerInventory->SetCapacity(20);
	PlayerInventory->SetWeightCapacity(60.0f);

	LootPlayerInteraction = CreateDefaultSubobject<UInteractionComponent>("PlayeLootInteraction");
	LootPlayerInteraction->InteractionActionText = LOCTEXT("LootPlayerText", "Loot");
	LootPlayerInteraction->InteractionNameText = LOCTEXT("LootPlayerText", "Player");
	LootPlayerInteraction->SetupAttachment(GetRootComponent());
	LootPlayerInteraction->SetActive(false, true);
	LootPlayerInteraction->bAutoActivate = false;

	InteractionCheckFrequency = 0.01f;
	InteractionCheckDistence = 1000.0f;

	MeleeAttackDistance = 150.f;
	MeleeAttackDamage = 20.f;

	//SprintSpeed = GetCharacterMovement()->MaxWalkSpeed * 2.f;
	//WalkSpeed = GetCharacterMovement()->MaxWalkSpeed;

	MaxHealth = 100.0f;
	Heath = MaxHealth;

	//bIsAiming = false;
	GetMesh()->SetOwnerNoSee(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

}



void APlayerManager::BeginPlay()
{
	Super::BeginPlay();
	InteractionData.LastInteractionCheckTime = GetWorld()->GetTimeSeconds();

	LootPlayerInteraction->OnInteract.AddDynamic(this, &APlayerManager::BeginLootingPlayer);

	if (APlayerState* playerState = GetPlayerState())
	{
		LootPlayerInteraction->SetInteractableNameText(FText::FromString(playerState->GetPlayerName()));
	}

	for (auto& playerMesh : PlayerMeshes) // TODO: THIS NEEDS TO BE CHANGED, WE DO NOT USE NAKED MESHS!
	{
		//EquipWear(playerMesh.Value->SkeletalMesh, playerMesh.Key);
		NakedMeshes.Add(playerMesh.Key, playerMesh.Value->SkeletalMesh);
	}
}

//void APlayerManager::ServerUseThrowable_Implementation()
//{
//	UseThrowable();
//}

void APlayerManager::MulticastPlayThrowableTossFX_Implementation(UAnimMontage* montageToPlay)
{
	if (GetNetMode() != NM_DedicatedServer && !IsLocallyControlled())
	{
		PlayAnimMontage(montageToPlay);
	}
}

UThrowableItem * APlayerManager::GetThrowable() const
{
	UThrowableItem* equippedThrowable = nullptr;
	if (EquippedItems.Contains(EEquippableSlot::EIS_ThrowbleItem))
	{
		equippedThrowable = Cast<UThrowableItem>(*EquippedItems.Find(EEquippableSlot::EIS_ThrowbleItem));
	}

	return equippedThrowable;
}
//
//void APlayerManager::UseThrowable()
//{
//	if (CanUseThrowable())
//	{
//		if (UThrowableItem* throwable = GetThrowable())
//		{
//			if (HasAuthority())
//			{
//				SpawnThrowable();
//					if (PlayerInventory)
//					{
//						PlayerInventory->ConsumeItem(throwable, 1);
//					}
//			}
//			else
//			{
//				if (throwable->GetQuantity() <= 1)
//				{
//					EquippedItems.Remove(EEquippableSlot::EIS_ThrowbleItem);
//					OnEquippedItemsChanged.Broadcast(EEquippableSlot::EIS_ThrowbleItem, nullptr);
//				}
//
//				PlayAnimMontage(throwable->ThrowableTossAnimation);
//				ServerUseThrowable();
//			}
//		}
//	}
//}

UWorld* APlayerManager::GetWorld() const
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		return GetOuter()->GetWorld();
	}
	else
	{
		return nullptr;
	}
}

void APlayerManager::SpawnThrowable()
{
	if (HasAuthority())
	{
		if (UThrowableItem* currentThrowable = GetThrowable())
		{
			if (currentThrowable->ThrowableClass)
			{
				FActorSpawnParameters spawnParams;
				spawnParams.Owner = spawnParams.Instigator = this;
				spawnParams.bNoFail = true;
				spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

				FVector camLocation;
				FRotator camRotation;

				GetController()->GetPlayerViewPoint(camLocation, camRotation);

				camLocation = (camRotation.Vector()* 20.0f) + camLocation;

				if (AThrowableWeapon* throwableWeapon = GetWorld()->SpawnActor<AThrowableWeapon>(currentThrowable->ThrowableClass, FTransform(camRotation, camLocation)))
				{
					MulticastPlayThrowableTossFX(currentThrowable->ThrowableTossAnimation);
				}
			}
		}
	}
}

bool APlayerManager::CanUseThrowable() const
{
	return GetThrowable() != nullptr && GetThrowable()->ThrowableClass != nullptr;
}

bool APlayerManager::IsInteracting()
{
	return GetWorldTimerManager().IsTimerActive(_timerHandleInteract);
}

float APlayerManager::GetRemainingInteractTime()
{
	return GetWorldTimerManager().GetTimerRemaining(_timerHandleInteract);
}

void APlayerManager::UseItem(UItem * item)
{
	if (GetLocalRole() < ROLE_Authority && item) // the same as HasAuthority()
	{
		ServerUseItem(item);
	}
	if (HasAuthority())
	{
		if (PlayerInventory && !PlayerInventory->FindItem(item))
		{
			return;
		}
	}
	if (item)
	{
		item->OnUse(this);
		item->Use(this);
	}

}



void APlayerManager::ServerUseItem_Implementation(UItem * item)
{
	UseItem(item);
}

bool APlayerManager::ServerUseItem_Validate(UItem * item)
{
	return true;
}

void APlayerManager::DropItem(UItem * item, const int32 quantity)
{

	if (PlayerInventory && item && PlayerInventory->FindItem(item))
	{
		if (GetLocalRole() < ROLE_Authority)
		{
			ServerDropItem(item, quantity);
			return;
		}
		if (HasAuthority())
		{
			const int32 itemQuantity = item->GetQuantity();
			const int32 droppedQuantty = PlayerInventory->ConsumeItem(item, quantity);
			FActorSpawnParameters spawnParams;
			spawnParams.Owner = this;
			spawnParams.bNoFail = true;
			spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			FVector spawnLocation = GetActorLocation();
			spawnLocation.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

			FTransform spawnTransfrom(GetActorRotation(), spawnLocation);

			ensure(PickupClass);

			APickup* pickup = GetWorld()->SpawnActor<APickup>(PickupClass, spawnTransfrom, spawnParams);
			pickup->InitializePickup(item->GetClass(), droppedQuantty);
		}
	}

}

bool APlayerManager::EquipItem(UEquippableItem * item)
{
	EquippedItems.Add(item->Slot, item);
	OnEquippedItemsChanged.Broadcast(item->Slot, item);
	return true;
}

bool APlayerManager::UnEquipItem(UEquippableItem * item)
{
	if (item)
	{
		if (EquippedItems.Contains(item->Slot))
		{
			if (item == *EquippedItems.Find(item->Slot))
			{
				EquippedItems.Remove(item->Slot);
				OnEquippedItemsChanged.Broadcast(item->Slot, nullptr);
				return true;
			}
		}
	}
	return false;
}

void APlayerManager::EquipWear(UWearItem * wear)
{
	if (USkeletalMeshComponent* wearMesh = *PlayerMeshes.Find(wear->Slot))
	{
		wearMesh->SetSkeletalMesh(wear->Mesh);
		wearMesh->SetMaterial(wearMesh->GetMaterials().Num() - 1, wear->MaterialInstance);
	}
}

void APlayerManager::EquipWeapon(UWeaponItem * weaponItem)
{
	if (weaponItem && weaponItem->WeaponClass && HasAuthority())
	{
		if (EquippedWeapon)
		{
			UnEquipWeapon();
		}

		FActorSpawnParameters spawnParams; 
		spawnParams.bNoFail = true;
		spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		spawnParams.Owner = spawnParams.Instigator = this;

		if (AWeapon* weapon = GetWorld()->SpawnActor<AWeapon>(weaponItem->WeaponClass, spawnParams))
		{
			weapon->Item = weaponItem;

			EquippedWeapon = weapon;
			OnRep_EquippedWeapon();

			weapon->OnEquip();
		}
	}
}

void APlayerManager::UnEquipWeapon()
{
	if (HasAuthority() && EquippedWeapon)
	{
		EquippedWeapon->OnUnEquip();
		EquippedWeapon->Destroy();
		EquippedWeapon = nullptr;
		OnRep_EquippedWeapon();
	}
}

void APlayerManager::EquipWear(USkeletalMesh* wear, EEquippableSlot slot)
{
	if (USkeletalMeshComponent* wearMesh = *PlayerMeshes.Find(slot))
	{
		wearMesh->SetSkeletalMesh(wear);
		wearMesh->SetMaterial(wearMesh->GetMaterials().Num() - 1, wear->Materials[0].MaterialInterface);
	}
}

void APlayerManager::UnEquipWear(const EEquippableSlot slot)
{

	if (USkeletalMeshComponent* EquippableMesh = *PlayerMeshes.Find(slot)) // TODO: DO NOT HIDE THE OROGINAL BODY FROM INVENTORY
	{
		if (USkeletalMesh* bodyMesh = *NakedMeshes.Find(slot))
		{
			for (int32 i = 0; i < bodyMesh->Materials.Num(); i++)
			{
				EquippableMesh->SetMaterial(i, bodyMesh->Materials[i].MaterialInterface);
				EquippableMesh->SetSkeletalMesh(bodyMesh);
			}
		}
	}
	else
	{
		EquippableMesh->SetSkeletalMesh(nullptr);
	}
}

void APlayerManager::ServerDropItem_Implementation(UItem * item, int32 quantity)
{
	DropItem(item, quantity);
}

bool APlayerManager::ServerDropItem_Validate(UItem * item, int32 quantity)
{
	return true;
}

USkeletalMeshComponent * APlayerManager::GetSlotSkeletalMeshComponent(const EEquippableSlot Slot)
{
	if (PlayerMeshes.Contains(Slot))
	{
		return *PlayerMeshes.Find(Slot);
	}
	return nullptr;
}

float APlayerManager::ModifyHealth(const float delta)
{
	const float oldHealth = Heath;

	Heath = FMath::Clamp<float>(Heath + delta, 0.0f, MaxHealth);

	return Heath - oldHealth;
}

void APlayerManager::OnRep_Health(float oldHealth)
{
	OnHealthModified(Heath - oldHealth);
}

void APlayerManager::OnRep_EquippedWeapon()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->OnEquip();
	}
}

//void APlayerManager::StartFire()
//{
//	if (EquippedWeapon)
//	{
//		EquippedWeapon->StartFire();
//	}
//	else
//	{
//		BeginMeleeAttack();
//	}
//}
//
//void APlayerManager::StopFire()
//{
//	if (EquippedWeapon)
//	{
//		EquippedWeapon->StopFire();
//	}
//}

void APlayerManager::BeginMeleeAttack()
{
	float asd = GetWorld()->TimeSince(LastMeleeAttackTime);
	float asd1 = MeleeAttackMontage->GetPlayLength();


	if (GetWorld()->TimeSince(LastMeleeAttackTime) > MeleeAttackMontage->GetPlayLength())
	{
		FHitResult hit;
		FCollisionShape shape = FCollisionShape::MakeSphere(15.0f);

		FVector startTrace = CameraComponent->GetComponentLocation();
		FVector endTrace = (CameraComponent->GetComponentRotation().Vector() * MeleeAttackDistance) + startTrace;

		FCollisionQueryParams querryParams = FCollisionQueryParams("MeleeSweep", false, this);

		PlayAnimMontage(MeleeAttackMontage);

		if (GetWorld()->SweepSingleByChannel(hit, startTrace, endTrace, FQuat(), COLLISION_WEAPON, shape, querryParams))
		{
			UE_LOG(LogTemp, Warning, TEXT("Punch!"));

			if (APlayerManager* hitPlayer = Cast<APlayerManager>(hit.GetActor()))
			{
				if (ASurvyvalPlayerController* playerController = Cast<ASurvyvalPlayerController>(GetController()))
				{
					playerController->OnHitPlayer();
				}
			}
		}
		ServerProcessMeleeHit(hit);
		LastMeleeAttackTime = GetWorld()->GetTimeSeconds();
	}
}

void APlayerManager::ServerProcessMeleeHit_Implementation(const FHitResult & meleeHit)
{
	if (GetWorld()->TimeSince(LastMeleeAttackTime) > MeleeAttackMontage->GetPlayLength() && (GetActorLocation() - meleeHit.ImpactPoint).Size() <= MeleeAttackDistance)
	{
		MulticastPlayerMeleeFX();

		UGameplayStatics::ApplyPointDamage(meleeHit.GetActor(), MeleeAttackDamage, (meleeHit.TraceStart - meleeHit.TraceEnd).GetSafeNormal(), meleeHit, GetController(), this, UMeleeDamage::StaticClass());
	}
	LastMeleeAttackTime = GetWorld()->GetTimeSeconds();
}

void APlayerManager::MulticastPlayerMeleeFX_Implementation()
{
	if (!IsLocallyControlled())
	{
		PlayAnimMontage(MeleeAttackMontage);
	}
}

void APlayerManager::Suicide(FDamageEvent const & damageEvent, const AActor * damageCauser)
{
	Killer = this;
	OnRep_Killer();
}

void APlayerManager::KilledByPlayer(FDamageEvent const & DamageEvent, APlayerManager* character, const AActor * damageCauser)
{
	Killer = character;
	OnRep_Killer();
}

void APlayerManager::OnRep_Killer()
{
	SetLifeSpan(20.0f);

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	GetMesh()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	GetMesh()->SetOwnerNoSee(false);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
	SetReplicatingMovement(false);

	LootPlayerInteraction->Activate();

	if (HasAuthority())
	{
		TArray<UEquippableItem*> equippables;
		EquippedItems.GenerateValueArray(equippables);

		for (auto& equippedItem : equippables)
		{
			equippedItem->SetEquipped(false);
		}
	}
	if (IsLocallyControlled())
	{
		SpringArmComponent->TargetArmLength = 500.0f;
		SpringArmComponent->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
		bUseControllerRotationPitch = true;

		

		if (ASurvyvalPlayerController* playerController = Cast<ASurvyvalPlayerController>(GetController()))
		{
			playerController->ShowDeathScreen(Killer);
		}
	}
}
//
//bool APlayerManager::CanSprint()
//{
//	return !IsAiming();
//}

//void APlayerManager::StartSprinting()
//{
//	SetSprinting(true);
//}
//
//void APlayerManager::StopSprinting()
//{
//	SetSprinting(false);
//}
//
//void APlayerManager::SetSprinting(const bool bNewSprinting)
//{
//	if ((bNewSprinting && !CanSprint()) || bNewSprinting == bSprinting)
//	{
//		return;
//	}
//
//	if (GetLocalRole() < ROLE_Authority)
//	{
//		ServerSetSprinting(bNewSprinting);
//	}
//
//	bSprinting = bNewSprinting;
//
//	GetCharacterMovement()->MaxWalkSpeed = bSprinting ? SprintSpeed : WalkSpeed;
//}
//
//void APlayerManager::ServerSetSprinting_Implementation(const bool bNewSprinting)
//{
//	SetSprinting(bNewSprinting);
//}
//
//bool APlayerManager::ServerSetSprinting_Validate(const bool bNewSprinting)
//{
//	return true;
//}

//void APlayerManager::StartCrouching()
//{
//	Crouch();
//}
//
//void APlayerManager::StopCrouching()
//{
//	UnCrouch();
//}
//
//void APlayerManager::MoveForward(float val)
//{
//	if (val == 0)
//		return;
//
//	AddMovementInput(GetActorForwardVector(), val);
//
//}
//
//void APlayerManager::MoveRight(float val)
//{
//	if (val == 0)
//		return;
//
//	AddMovementInput(GetActorRightVector(), val);
//}
//
//void APlayerManager::LookUp(float val)
//{
//	if (val == 0)
//		return;
//
//	AddControllerPitchInput(val);
//}
//
//void APlayerManager::Turn(float val)
//{
//	if (val == 0)
//		return;
//	AddControllerYawInput(val);
//}

//bool APlayerManager::CanAim()
//{
//	return EquippedWeapon != nullptr;
//}

//void APlayerManager::StartAiming()
//{
//	if (CanAim())
//	{
//		SetAiming(true);
//	}
//}
//
//void APlayerManager::StopAiming()
//{
//	SetAiming(false);
//}
//
//void APlayerManager::SetAiming(const bool bNewAiming)
//{
//	if (bNewAiming && !CanAim() || bNewAiming == bIsAiming)
//		return;
//
//	if (GetLocalRole() < ROLE_Authority)
//	{
//		ServerSetAiming(bNewAiming);
//	}
//
//	bIsAiming = bNewAiming;
//}
//
//void APlayerManager::ServerSetAiming_Implementation(const bool bNewAiming)
//{
//	SetAiming(bNewAiming);
//}

void APlayerManager::StartReload()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->StartReload();
	}
}

void APlayerManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const bool bIsInteractingOnServer = (HasAuthority() && IsInteracting());


	if ((!HasAuthority() || bIsInteractingOnServer) && GetWorld()->TimeSince(InteractionData.LastInteractionCheckTime) > InteractionCheckFrequency)
	{
		PreformInteractionCheck();
	}

	if (IsLocallyControlled())
	{
		const float desieredFOV = IsAiming() ? 70.0f : 100.0f; // move these values to to the weapon class?
		CameraComponent->SetFieldOfView(FMath::FInterpTo(CameraComponent->FieldOfView, desieredFOV, DeltaTime, 10.0f));

		if (EquippedWeapon)
		{
			const FVector aimDownSightLocation = EquippedWeapon->GetWeaponMesh()->GetSocketLocation(NAME_AimDownSightsSocket);
			const FVector defaultCameraLocation = GetMesh()->GetSocketLocation(FName("CameraSocket"));

			FVector cameraLocation = bIsAiming ? aimDownSightLocation : defaultCameraLocation;

			const float interpSpeed = FVector::Dist(aimDownSightLocation, defaultCameraLocation) / EquippedWeapon->AimDownSightTime;
			CameraComponent->SetWorldLocation(FMath::VInterpTo(CameraComponent->GetComponentLocation(), cameraLocation, DeltaTime, interpSpeed));
		}
	}
}

void APlayerManager::Restart()
{
	Super::Restart();

	if(ASurvyvalPlayerController* playerContoller = Cast<ASurvyvalPlayerController>(GetController()))
	{
		playerContoller->ShowUI();
	}
}

float APlayerManager::TakeDamage(float Damage, FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamagaeCauser)
{
	Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamagaeCauser);

	const float damageDealt = ModifyHealth(-Damage);

	if (Heath <= 0.0f)
	{
		if (APlayerManager* killerCharacter = Cast<APlayerManager>(DamagaeCauser->GetOwner()))
		{
			KilledByPlayer(DamageEvent, killerCharacter, DamagaeCauser);
		}
		else
		{
			Suicide(DamageEvent, DamagaeCauser);
		}
	}
	return damageDealt;
}

void APlayerManager::SetActorHiddenInGame(bool bNewHidden)
{
	Super::SetActorHiddenInGame(bNewHidden);

	if (EquippedWeapon)
	{
		EquippedWeapon->SetActorHiddenInGame(bNewHidden);
	}
}

void APlayerManager::PreformInteractionCheck()
{
	if (GetController() == nullptr)
		return;

	InteractionData.LastInteractionCheckTime = GetWorld()->GetTimeSeconds();

	FVector EyesLocation;
	FRotator EyesRotation;

	GetController()->GetPlayerViewPoint(EyesLocation, EyesRotation);

	FVector TraceStart = EyesLocation;
	FVector TraceEnd = (EyesRotation.Vector() * InteractionCheckDistence) + TraceStart;

	FHitResult TraceHit;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(TraceHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
	{
		if (TraceHit.GetActor())
		{
			if (UInteractionComponent* interactionComponent = Cast<UInteractionComponent>(TraceHit.GetActor()->GetComponentByClass(UInteractionComponent::StaticClass())))
			{
				float distance = (TraceStart - TraceHit.ImpactPoint).Size();
				if (interactionComponent != GetInteractable() && distance <= interactionComponent->InteractionDistance)
				{
					FoundInteractable(interactionComponent);
				}
				else if (distance > interactionComponent->InteractionDistance && GetInteractable())
				{
					CouldNotFindInteractable();
				}
				return;
			}
		}
	}

	CouldNotFindInteractable();


}

void APlayerManager::CouldNotFindInteractable()
{
	if (GetWorldTimerManager().IsTimerActive(_timerHandleInteract))
	{
		GetWorldTimerManager().ClearTimer(_timerHandleInteract);
	}

	if (UInteractionComponent* interactable = GetInteractable())
	{
		interactable->EndFocus(this);

		if (InteractionData.bInteractHeld)
		{
			PlayerMovement->EndInteract();
		}
	}

	InteractionData.ViewedInteractionComponent = nullptr;

}

void APlayerManager::FoundInteractable(UInteractionComponent * interactable)
{
	PlayerMovement->EndInteract();
	if (UInteractionComponent* OldInteractable = GetInteractable())
	{
		OldInteractable->EndFocus(this);
	}

	InteractionData.ViewedInteractionComponent = interactable;
	interactable->BeginFocus(this);
}

//void APlayerManager::BeginInteract()
//{
//	if (!HasAuthority())
//	{
//		ServerBeginInteract();
//	}
//
//	if(HasAuthority())
//	{
//		PreformInteractionCheck();
//	}
//
//	InteractionData.bInteractHeld = true;
//
//	if (UInteractionComponent* Interactable = GetInteractable())
//	{
//		Interactable->BeginInteract(this);
//		if (FMath::IsNearlyZero(Interactable->InteractionTime))
//		{
//			Interact();
//		}
//		else
//		{
//			GetWorldTimerManager().SetTimer(_timerHandleInteract, this, &APlayerManager::Interact, Interactable->InteractionTime, false);
//		}
//	}
//}
//
//void APlayerManager::EndInteract()
//{
//	if (!HasAuthority())
//		ServerEndInteract();
//
//	InteractionData.bInteractHeld = false;
//
//	GetWorldTimerManager().ClearTimer(_timerHandleInteract);
//
//	if (UInteractionComponent* Interactable = GetInteractable())
//	{
//		Interactable->EndInteract(this);
//	}
//
//}

bool APlayerManager::CheckAuthority()
{
	return HasAuthority();
}

int APlayerManager::CheckRole()
{
	return GetLocalRole();
}

void APlayerManager::Interact()
{
	GetWorldTimerManager().ClearTimer(_timerHandleInteract);

	if (UInteractionComponent* Interactable = GetInteractable())
		Interactable->Interact(this);

}

//void APlayerManager::ServerBeginInteract_Implementation()
//{
//	BeginInteract();
//}
//
//bool APlayerManager::ServerBeginInteract_Validate()
//{
//	return true;
//}

//void APlayerManager::ServerEndInteract_Implementation()
//{
//	PlayerMovement->EndInteract();
//}
//
//bool APlayerManager::ServerEndInteract_Validate()
//{
//	return true;
//}

// Called to bind functionality to input
void APlayerManager::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	PlayerMovement->SetuInput(PlayerInputComponent);

	/*Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &APlayerManager::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &APlayerManager::StopFire);

	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &APlayerManager::StartAiming);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &APlayerManager::StopAiming);
	PlayerInputComponent->BindAction("Throw", IE_Released, this, &APlayerManager::UseThrowable);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &APlayerManager::BeginInteract);
	PlayerInputComponent->BindAction("Interact", IE_Released, this, &APlayerManager::EndInteract);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &APlayerManager::StartCrouching);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &APlayerManager::StopCrouching);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &APlayerManager::StartSprinting);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &APlayerManager::StopSprinting);

	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerManager::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerManager::MoveRight);*/
}

void APlayerManager::OnLootSourceOwnerDestroyed(AActor * destroyedActor)
{
	if (HasAuthority() && LootSource && destroyedActor == LootSource->GetOwner())
	{
		ServerSetLootSource(nullptr);
	}
}

void APlayerManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlayerManager, bSprinting);
	DOREPLIFETIME(APlayerManager, LootSource);
	DOREPLIFETIME(APlayerManager, EquippedWeapon);
	DOREPLIFETIME(APlayerManager, Killer);

	DOREPLIFETIME_CONDITION(APlayerManager, Heath, COND_OwnerOnly); // will only replicate to the same client
	DOREPLIFETIME_CONDITION(APlayerManager, bIsAiming, COND_SkipOwner);
}

void APlayerManager::OnRep_LootSource()
{
	if (ASurvyvalPlayerController* playerController = Cast<ASurvyvalPlayerController>(GetController()))
	{
		if (playerController->IsLocalController())
		{
			if (LootSource)
			{
				playerController->ShowLootMenu(LootSource);
			}
			else
			{
				playerController->HideLootMenu();
			}
		}
	}
}


void APlayerManager::SetLootSource(UInventoryComponent * newLootSource)
{

	if (newLootSource && newLootSource->GetOwner())
	{
		newLootSource->GetOwner()->OnDestroyed.AddUniqueDynamic(this, &APlayerManager::OnLootSourceOwnerDestroyed);
	}


	if (HasAuthority())
	{
		if (newLootSource)
		{
			if (APlayerManager* player = Cast<APlayerManager>(newLootSource->GetOwner()))
			{
				player->SetLifeSpan(120.f);
			}
		}

		LootSource = newLootSource;
		OnRep_LootSource();
	}
	else
	{
		ServerSetLootSource(newLootSource);
	}
}

bool APlayerManager::IsLooting() const
{
	return LootSource != nullptr;
}

void APlayerManager::LootItem(UItem * itemToGive)
{
	if (HasAuthority())
	{
		if (PlayerInventory && LootSource && itemToGive && LootSource->HasItem(itemToGive->GetClass(), itemToGive->GetQuantity()))
		{
			const FItemAddResult addResult = PlayerInventory->TryAddItem(itemToGive);

			if (addResult.ActualAmountGiven > 0)
			{
				LootSource->ConsumeItem(itemToGive, addResult.ActualAmountGiven);
			}
			else
			{
				if (ASurvyvalPlayerController* playerController = Cast<ASurvyvalPlayerController>(GetController()))
				{
					playerController->ClientShowNotification(addResult.ErrorText);
				}
			}
		}
	}
	else
	{
		ServerLootItem(itemToGive);
	}
}

void APlayerManager::ServerLootItem_Implementation(UItem * itemToLoot)
{
	LootItem(itemToLoot);
}

bool APlayerManager::ServerLootItem_Validate(UItem * itemToLoot)
{
	return true;
}

void APlayerManager::BeginLootingPlayer(APlayerManager * player)
{
	if (player)
	{
		player->SetLootSource(PlayerInventory);
	}
}

void APlayerManager::ServerSetLootSource_Implementation(UInventoryComponent * newLootSource)
{
	SetLootSource(newLootSource);
}

bool APlayerManager::ServerSetLootSource_Validate(UInventoryComponent * newLootSource)
{
	return true;
}


#undef LOCTEXT_NAMESPACE