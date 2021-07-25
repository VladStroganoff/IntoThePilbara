
#include "Weapon.h"

#include "GAPGame.h"

#include "SurvyvalPlayerController.h"
#include "PlayerManager.h"
#include "Components/AudioComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Curves/CurveVector.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

#include "Net/UnrealNetwork.h"
#include "Items/EquippableItem.h"
#include "Items/AmmoItem.h"
#include "DrawDebugHelpers.h"



AWeapon::AWeapon()
{
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->bReceivesDecals = false;
	WeaponMesh->SetCollisionObjectType(ECC_WorldDynamic);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	RootComponent = WeaponMesh;

	bLoopMuzzleFX = false;
	bLoopFireAnim = false;
	bPlayingFireAnim = false;
	bIsEquipped = false;
	bWansToFire = false;
	bPendingReload = false;
	bPendingEquip = false;
	CurrentState = EWeaponState::Idle;
	AttachSocketFirstPerson = FName("GripPoint");
	AttachSocketThirdPerson = FName("GripPoint");

	CurrentAmmoInClip = 0;
	BurstCounter = 0;
	LastFireTime = 0.0f;

	AimDownSightTime = 0.5f;
	RecoilResetSpeed = 5.0f;
	RecoilSpeed = 10.0f;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	bReplicates = true;
	bNetUseOwnerRelevancy = true; 

}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, PawnOwner);

	DOREPLIFETIME_CONDITION(AWeapon, CurrentAmmoInClip, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AWeapon, BurstCounter, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AWeapon, bPendingReload, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AWeapon, Item, COND_InitialOnly);
}


void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		PawnOwner = Cast<APlayerManager>(GetOwner());
	}
}

void AWeapon::Destroyed()
{
	Super::Destroyed();
	StopSimulateWeaponFire();
}

void AWeapon::UseClipAmmo()
{
	if (HasAuthority())
	{
		--CurrentAmmoInClip;
	}
}

void AWeapon::ConsumeAmmo(const int32 amount)
{
	if (HasAuthority() && PawnOwner)
	{
		if (UInventoryComponent* inventory = PawnOwner->PlayerInventory)
		{
			if (UItem* ammoItem = inventory->FindItemByClass(WeaponConfig.AmmoClass))
			{
				inventory->ConsumeItem(ammoItem, amount);
			}

		}
	}
}

void AWeapon::ReturnAmmoToInventory()
{
	if (HasAuthority())
	{
		if (PawnOwner && CurrentAmmoInClip > 0)
		{
			if (UInventoryComponent* inventory = PawnOwner->PlayerInventory)
			{
				inventory->TryAddItemFromClass(WeaponConfig.AmmoClass, CurrentAmmoInClip);
			}
		}
	}
}

void AWeapon::OnEquip()
{
	AttachMeshToPawn();
	bPendingEquip = true;
	DetermineWeaponState();
	OnEquippedFinished();
	if (PawnOwner && PawnOwner->IsLocallyControlled())
	{
		PlayWeaponSound(EquipSound);
	}
}

void AWeapon::OnEquippedFinished()
{
	AttachMeshToPawn();
	bIsEquipped = true;
	bPendingEquip = false;

	DetermineWeaponState();

	if (PawnOwner)
	{
		if (PawnOwner->IsLocallyControlled() && CanReload())
		{
			StartReload();
		}
	}
}

void AWeapon::OnUnEquip()
{
	bIsEquipped = false;
	StopFire();

	if (bPendingReload)
	{
		StopWeaponAnimation(ReloadAnim);
		bPendingReload = false;

		GetWorldTimerManager().ClearTimer(TimerHandle_StopReload);
		GetWorldTimerManager().ClearTimer(TimerHandle_ReloadWeapon);
	}

	if (bPendingEquip)
	{
		StopWeaponAnimation(EquipAnim);
		bPendingEquip = false;
		GetWorldTimerManager().ClearTimer(TimerHandle_OnEquipFinished);
	}

	ReturnAmmoToInventory();
	DetermineWeaponState();
}

bool AWeapon::IsEquipped() const
{
	return bIsEquipped;
}

bool AWeapon::IsAttachedToPawn() const
{
	return bIsEquipped || bPendingEquip;
}

void AWeapon::StartFire()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerStartFire();
	}

	if (!bWansToFire)
	{
		bWansToFire = true;
		DetermineWeaponState();
	}
}


void AWeapon::StopFire()
{
	if (GetLocalRole() < ROLE_Authority && PawnOwner && PawnOwner->IsLocallyControlled())
	{
		ServerStopFire();
	}

	if (bWansToFire)
	{
		bWansToFire = false;
		DetermineWeaponState();
	}
}

void AWeapon::StartReload(bool bFromReplication)
{
	if (!bFromReplication && GetLocalRole() < ROLE_Authority)
	{
		ServerStartReload();
	}

	if (bFromReplication || CanReload())
	{
		bPendingReload = false;
		DetermineWeaponState();

		float animDuration = PlayerWeaponAnimation(ReloadAnim);
		if (animDuration <= 0.0f)
		{
			animDuration = 0.5f;
		}

		GetWorldTimerManager().SetTimer(TimerHandle_StopReload, this, &AWeapon::StopReload, animDuration, false);
		
		if (HasAuthority())
		{
			GetWorldTimerManager().SetTimer(TimerHandle_ReloadWeapon, this, &AWeapon::ReloadWeapon, FMath::Max(0.1f, animDuration - 0.1f), false);
		}
		
		if (PawnOwner && PawnOwner->IsLocallyControlled())
		{
			PlayWeaponSound(ReloadSound);
		}
	}
}

void AWeapon::StopReload()
{
	if (CurrentState == EWeaponState::Reloading)
	{
		bPendingReload = false;
		DetermineWeaponState();
		StopWeaponAnimation(ReloadAnim);
	}
}

void AWeapon::ReloadWeapon()
{
	const int32 clidDelta = FMath::Min(WeaponConfig.AmmoClip - CurrentAmmoInClip, GetCurrentAmmo());

	if (clidDelta > 0)
	{
		CurrentAmmoInClip += clidDelta;
		ConsumeAmmo(clidDelta);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Didnt have enoughe ammo for reload :("));
	}

}

bool AWeapon::CanFire() const
{
	bool bCanFire = PawnOwner != nullptr;
	bool bStateOKToFire = ((CurrentState == EWeaponState::Idle) || (CurrentState == EWeaponState::Firing));
	return ((bCanFire == true) && (bStateOKToFire == true) && (bPendingReload == false));
}

bool AWeapon::CanReload() const
{
	bool bCanReload = PawnOwner != nullptr;
	bool bGotAmmo = ((CurrentAmmoInClip < WeaponConfig.AmmoClip) && (GetCurrentAmmo() > 0));
	bool bStateOKToReload = ((CurrentState == EWeaponState::Idle) || (CurrentState == EWeaponState::Firing));
	return ((bCanReload == true) && (bGotAmmo == true) && (bStateOKToReload == true));
}

EWeaponState AWeapon::GetCurrentState() const
{
	return CurrentState;
}

int32 AWeapon::GetCurrentAmmo() const
{
	if (PawnOwner)
	{
		if (UInventoryComponent* inventory = PawnOwner->PlayerInventory)
		{
			if (UItem* ammo = inventory->FindItemByClass(WeaponConfig.AmmoClass))
			{
				return ammo->GetQuantity();
			}
		}
	}

	return 0;
}

int32 AWeapon::GetCurrentAmmoClip() const 
{
	return CurrentAmmoInClip;
}

int32 AWeapon::GetAmmoPerClip() const
{
	return WeaponConfig.AmmoClip;
}

USkeletalMeshComponent* AWeapon::GetWeaponMesh() const
{
	return WeaponMesh;
}

class APlayerManager* AWeapon::GetPawnOwner() const
{
	return PawnOwner;
}

void AWeapon::SetPawnOwner(APlayerManager* newOwner)
{
	if (PawnOwner != newOwner)
	{
		SetInstigator(newOwner);
		PawnOwner = newOwner;
		SetOwner(newOwner);
	}
}

float AWeapon::GetEquipStartedTime() const
{
	return EquipStartedTime;
}

float AWeapon::GetEquipDuration() const
{
	return EquipDuration;
}

void AWeapon::ClientStartReload_Implementation()
{
	StartReload();
}

void AWeapon::ServerStartFire_Implementation()
{
	StartFire();
}

bool AWeapon::ServerStartFire_Validate()
{
	return true;
}

void AWeapon::ServerStopFire_Implementation()
{
	StopFire();
}

bool AWeapon::ServerStopFire_Validate()
{
	return true;
}

void AWeapon::ServerStartReload_Implementation()
{
	StartReload();
}

bool AWeapon::ServerStartReload_Validate()
{
	return true;
}

void AWeapon::ServerStopReload_Implementation()
{
	StopReload();
}

bool AWeapon::ServerStopReload_Validate()
{
	return true;
}


void AWeapon::OnRep_PawnOwner()
{

}

void AWeapon::OnRep_BurstCounter()
{
	if (BurstCounter > 0)
	{
		SimulateWeaponFire();
	}
	else
	{
		StopSimulateWeaponFire();
	}
}

void AWeapon::OnRep_Reload()
{
	if (bPendingReload)
	{
		StartReload();
	}
	else
	{
		StopReload();
	}
}

void AWeapon::SimulateWeaponFire()
{
	if (HasAuthority() && CurrentState == EWeaponState::Firing)
	{
		return;
	}

	if (!bLoopMuzzleFX || MuzzleParticleSystemComp == NULL)
	{
		if (PawnOwner != NULL && (PawnOwner->IsLocallyControlled() == true))
		{
			AController* playerController = PawnOwner->GetController();
			if (playerController != NULL)
			{
				WeaponMesh->GetSocketLocation(MuzzleAttachPoint);
				MuzzleParticleSystemComp = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, WeaponMesh, MuzzleAttachPoint);
				MuzzleParticleSystemComp->bOwnerNoSee = false;
				MuzzleParticleSystemComp->bOnlyOwnerSee = true;
			}
			else
			{
				MuzzleParticleSystemComp = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, WeaponMesh, MuzzleAttachPoint);
			}
		}
	}

	if (!bLoopFireAnim || !bPlayingFireAnim)
	{
		FWeaponAnim animToPlay = FireAnim; // PawnOwner->IsAiming()|| PawnOwner->IsLocallyControlled() ? FireAimingAnim : FireAnim;
		PlayerWeaponAnimation(FireAnim);
		bPlayingFireAnim = true;
	}

	if (bLoopFireSound)
	{
		if (FireAudioComponent == NULL)
		{
			FireAudioComponent = PlayWeaponSound(FireLoopSound);
		}
		else
		{
			PlayWeaponSound(FireSound);
		}
	}

	ASurvyvalPlayerController* playerController = (PawnOwner != NULL) ? Cast<ASurvyvalPlayerController>(PawnOwner->Controller) : NULL;

	if (playerController != NULL && playerController->IsLocalController())
	{
		if (FireCameraShake != NULL)
		{
			playerController->ClientPlayCameraShake(FireCameraShake, 1);
		}
		if (FireForceFeedback != NULL)
		{
			FForceFeedbackParameters forceParams;
			forceParams.Tag = "Weapon";
			playerController->ClientPlayForceFeedback(FireForceFeedback, forceParams);
		}
	}

}

void AWeapon::StopSimulateWeaponFire()
{
	if (bLoopMuzzleFX)
	{
		if (MuzzleParticleSystemComp != NULL)
		{
			MuzzleParticleSystemComp->DeactivateSystem();
			MuzzleParticleSystemComp = NULL;
		}
		if (MuzzleParticleSystemCompSecondary != NULL)
		{
			MuzzleParticleSystemCompSecondary->DeactivateSystem();
			MuzzleParticleSystemCompSecondary = NULL;
		}
	}

	if (bLoopFireAnim && bPlayingFireAnim)
	{
		StopWeaponAnimation(FireAimAnim);
		StopWeaponAnimation(FireAnim);
		bPlayingFireAnim = false;
	}

	if (FireAudioComponent)
	{
		FireAudioComponent->FadeOut(0.1f, 0.0f);
		FireAudioComponent = NULL;
		PlayWeaponSound(FireFinishSound);
	}
}

void AWeapon::HandleHit(const FHitResult& hit, class APlayerManager* hitPlayer)
{
	if (hit.GetActor())
	{
		UE_LOG(LogTemp, Warning, TEXT("Hit actor %s"), *hit.GetActor()->GetName());
	}

	ServerHandleHit(hit, hitPlayer);

	if (hitPlayer && PawnOwner)
	{
		if (ASurvyvalPlayerController* playerController = Cast<ASurvyvalPlayerController>(PawnOwner->GetController()))
		{
			playerController->OnHitPlayer();
		}
	}
}

void AWeapon::ServerHandleHit_Implementation(const FHitResult& hit, class APlayerManager* hitPlayer)
{
	if (PawnOwner)
	{
		float damageMultiplyer = 1.0f;

		for (auto& boneDamageMdifier : HitScanConfig.BoneDamageModifiers)
		{
			if (hit.BoneName == boneDamageMdifier.Key)
			{
				damageMultiplyer = boneDamageMdifier.Value;
				break;
			}
		}
		if (hitPlayer)
		{
			UGameplayStatics::ApplyPointDamage(hitPlayer, HitScanConfig.Damage * damageMultiplyer, (hit.TraceStart - hit.TraceEnd).GetSafeNormal(), hit, PawnOwner->GetController(), this, HitScanConfig.DamageTye);
		}
	}
}

bool AWeapon::ServerHandleHit_Validate(const FHitResult& hit, class APlayerManager* hitPlayer)
{
	return true;
}

void AWeapon::FireShot()
{
	if (PawnOwner)
	{
		if (ASurvyvalPlayerController* playerController = Cast<ASurvyvalPlayerController>(PawnOwner->GetController()))
		{
			if (RecoilCurve)
			{
				const FVector2D recoilAmount(RecoilCurve->GetVectorValue(FMath::RandRange(0.0f, 0.0f)).X, RecoilCurve->GetVectorValue(FMath::RandRange(0.0f, 0.1f)).Y);
				playerController->ApplyRecoil(recoilAmount, RecoilSpeed, RecoilResetSpeed, FireCameraShake);
			}

			FVector camLocation;
			FRotator camRotation;
			playerController->GetPlayerViewPoint(camLocation, camRotation);

			FHitResult hit;
			FCollisionQueryParams queryParams;
			queryParams.AddIgnoredActor(this);
			queryParams.AddIgnoredActor(PawnOwner);

			FVector fireDir = camRotation.Vector(); //PawnOwner->IsAiming() ? camRotation.Vector() : FMath::VRandCone(camRotation.Vector(), FMath::DegreesToRadians(PawnOwner->IsAiming() ? 0.0f : 0.5f));
			FVector traceStart = camLocation;
			FVector traceEnd = (fireDir * HitScanConfig.Distance) + camLocation;

			if (GetWorld()->LineTraceSingleByChannel(hit, traceStart, traceEnd, COLLISION_WEAPON, queryParams))
			{
				APlayerManager* hitCharacter = Cast<APlayerManager>(hit.GetActor());

				HandleHit(hit, hitCharacter);

				FColor pointColor = FColor::Red;
				DrawDebugPoint(GetWorld(), hit.ImpactPoint, 5.0f, pointColor, false, 30.0f);
			}
		}
	}
}

void AWeapon::HandleReFiring()
{
	UWorld* myWorld = GetWorld();
	float slackTimeThisFrame = FMath::Max(0.0f, (myWorld->TimeSeconds - LastFireTime) - WeaponConfig.TimeBetweenShots);

	if (bAllowAutomaticWeaponCatchup)
	{
		TimerIntervalAdjustment -= slackTimeThisFrame;
	}

	HandleFiring();
}

void AWeapon::HandleFiring()
{
	if ((CurrentAmmoInClip > 0) && CanFire())
	{
		if (GetNetMode() != NM_DedicatedServer)
		{
			SimulateWeaponFire();
		}

		if (PawnOwner && PawnOwner->IsLocallyControlled())
		{
			FireShot();
			UseClipAmmo();

			BurstCounter++;
			OnRep_BurstCounter();
		}
	}
	else if(CanReload())
	{
		StartReload();
	}
	else if (PawnOwner && PawnOwner->IsLocallyControlled())
	{
		if (GetCurrentAmmo() == 0 && !bRefiring)
		{
			PlayWeaponSound(OutOfAmmoSound);
			ASurvyvalPlayerController* playerController = Cast<ASurvyvalPlayerController>(PawnOwner->Controller);
		}

		if (BurstCounter > 0)
		{
			OnBurstFinished();
		}
	}

	if (PawnOwner && PawnOwner->IsLocallyControlled())
	{
		if (GetLocalRole() < ROLE_Authority)
		{
			ServerHandleFiring();
		}

		if (CurrentAmmoInClip <= 0 && CanReload())
		{
			StartReload();
		}

		bRefiring = (CurrentState == EWeaponState::Firing && WeaponConfig.TimeBetweenShots > 0.0f);
		if (bRefiring)
		{
			GetWorldTimerManager().SetTimer(TimerHandle_HandleFiring, this, &AWeapon::HandleReFiring, FMath::Max<float>(WeaponConfig.TimeBetweenShots + TimerIntervalAdjustment, SMALL_NUMBER), false);
			TimerIntervalAdjustment = 0.0f;
		}
	}
	LastFireTime = GetWorld()->GetTimeSeconds();
}

void AWeapon::OnBurstStarted()
{
	const float gameTime = GetWorld()->GetTimeSeconds();
	if (LastFireTime > 0 && WeaponConfig.TimeBetweenShots > 0.0f && LastFireTime + WeaponConfig.TimeBetweenShots > gameTime)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_HandleFiring, this, &AWeapon::HandleFiring, LastFireTime + WeaponConfig.TimeBetweenShots - gameTime, false);
	}
	else
	{
		HandleFiring();
	}
}

void AWeapon::OnBurstFinished()
{
	BurstCounter = 0;

	if (GetNetMode() != NM_DedicatedServer)
	{
		StopSimulateWeaponFire();
	}

	GetWorldTimerManager().ClearTimer(TimerHandle_HandleFiring);
	bRefiring = false;

	TimerIntervalAdjustment = 0.0f;
}

void AWeapon::SetWeaponState(EWeaponState newState)
{
	const EWeaponState oldState = CurrentState;

	if (oldState == EWeaponState::Firing && newState != EWeaponState::Firing)
	{
		OnBurstFinished();
	}

	CurrentState = newState;

	if (oldState != EWeaponState::Firing && newState == EWeaponState::Firing)
	{
		OnBurstStarted();
	}
}

void AWeapon::DetermineWeaponState()
{
	EWeaponState newState = EWeaponState::Idle;

	if (bIsEquipped)
	{
		if (bPendingReload)
		{
			if (CanReload() == false)
			{
				newState = CurrentState;
			}
			else
			{
				newState = EWeaponState::Reloading;
			}
		}
		else if ((bPendingReload == false) && (bWansToFire == true) && (CanFire() == true))
		{
			newState = EWeaponState::Firing;
		}
	}
	else if (bPendingEquip)
	{
		newState = EWeaponState::Equipping;
	}

	SetWeaponState(newState);
}

void AWeapon::AttachMeshToPawn()
{
	if (PawnOwner)
	{
		if (const USkeletalMeshComponent* PawnMesh = PawnOwner->GetMesh())
		{
			const FName attachPoint = PawnOwner->IsLocallyControlled() ? AttachSocketFirstPerson : AttachSocketThirdPerson;
			AttachToComponent(PawnOwner->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, attachPoint);
		}
	}
}

UAudioComponent* AWeapon::PlayWeaponSound(USoundCue* sound)
{
	UAudioComponent* audioComponent = NULL;

	if (sound && PawnOwner)
	{
		audioComponent = UGameplayStatics::SpawnSoundAttached(sound, PawnOwner->GetRootComponent());
	}

	return audioComponent;
}

float AWeapon::PlayerWeaponAnimation(const FWeaponAnim& animation)
{
	float duration = 0.0f;

	if (PawnOwner)
	{
		UAnimMontage* useAnimation = PawnOwner->IsLocallyControlled() ? animation.firstPerson : animation.thirdPerson;
		if (useAnimation)
		{
			duration = PawnOwner->PlayAnimMontage(useAnimation);
		}
	}

	return duration;
}

void AWeapon::StopWeaponAnimation(const FWeaponAnim& animation)
{
	if (PawnOwner)
	{
		UAnimMontage* useAnim = PawnOwner->IsLocallyControlled() ? animation.firstPerson : animation.thirdPerson;
		if (useAnim)
		{
			PawnOwner->StopAnimMontage(useAnim);
		}
	}
}

FVector AWeapon::GetCameraAim() const
{
	ASurvyvalPlayerController* const playerController = GetInstigator() ? Cast<ASurvyvalPlayerController>(GetInstigator()->Controller) : NULL;
	FVector finalAim = FVector::ZeroVector;

	if (playerController)
	{
		FVector camLocation;
		FRotator camRotation;
		playerController->GetPlayerViewPoint(camLocation, camRotation);
		finalAim = camRotation.Vector();
	}
	else if(GetInstigator())
	{
		finalAim = GetInstigator()->GetBaseAimRotation().Vector();
	}

	return finalAim;
}

FHitResult AWeapon::WeaponTrace(const FVector& startTrace, const FVector& endTrace) const
{
	FCollisionQueryParams traceParams(SCENE_QUERY_STAT(WeaponTrace), true, GetInstigator());
	traceParams.bReturnPhysicalMaterial = true;

	FHitResult hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(hit, startTrace, endTrace, COLLISION_WEAPON, traceParams);

	return hit;
}

void AWeapon::ServerHandleFiring_Implementation()
{
	const bool bShouldUpdateAmmo = (CurrentAmmoInClip > 0 && CanFire());

	HandleFiring();

	if (bShouldUpdateAmmo)
	{
		UseClipAmmo();

		BurstCounter++;
	}
}

bool AWeapon::ServerHandleFiring_Validate()
{
	return true;
}