
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

class UAnimMontage;
class APlayerManager;
class UAudioComponent;
class UParticleSystemComponent;
class UMatineeCameraShake;
class UForceFeedbackEffect;
class USoundCue;


UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	Idle, 
	Firing,
	Reloading, 
	Equipping
};

USTRUCT(BlueprintType)
struct FWeaponData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Ammo)
	int32 AmmoClip;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Ammo)
	TSubclassOf<class UAmmoItem> AmmoClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = WeaponStat)
	float TimeBetweenShots;

	FWeaponData()
	{
		AmmoClip = 20;
		TimeBetweenShots = 0.2f;
	}
};

USTRUCT()
struct FWeaponAnim
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* firstPerson;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* thirdPerson;

};

USTRUCT(BlueprintType)
struct FHitScanConfiguration
{
	GENERATED_BODY()

	FHitScanConfiguration()
	{
		Distance = 10000.0f;
		Damage = 25.0f;
		Radious = 0.0f;
		DamageTye = UDamageType::StaticClass();
	}

	UPROPERTY(EditDefaultsOnly, Category = "Trace Info")
	TMap<FName, float> BoneDamageModifiers; // for extra damage if you hit specific bone

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace Info")
	float Distance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace Info")
	float Damage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace Info")
	float Radious;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace Info")
	TSubclassOf<UDamageType> DamageTye;

};

UCLASS()
class GAPGAME_API AWeapon : public AActor
{
	GENERATED_BODY()

	friend class APlayerManager;

public:	
	AWeapon();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	UPROPERTY(EditAnywhere, Category = Components)
	USkeletalMeshComponent* WeaponMesh;


protected:

	void UseClipAmmo();
	void ConsumeAmmo(const int32 amount);
	void ReturnAmmoToInventory();

	virtual void OnEquip();
	virtual void OnEquippedFinished();
	virtual void OnUnEquip();
	bool IsEquipped() const;
	bool IsAttachedToPawn() const;

	virtual void StartFire();
	virtual void StopFire();

	virtual void StartReload( bool bFromReplication = false);
	virtual void StopReload();

	virtual void ReloadWeapon();

	UFUNCTION(reliable, client)
	void ClientStartReload();

	bool CanFire() const;
	bool CanReload() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	EWeaponState GetCurrentState() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	int32 GetCurrentAmmo() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	int32 GetCurrentAmmoClip() const;

	int32 GetAmmoPerClip() const;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	USkeletalMeshComponent* GetWeaponMesh() const;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	class APlayerManager* GetPawnOwner() const;

	void SetPawnOwner(APlayerManager* newOwner);

	float GetEquipStartedTime() const;

	float GetEquipDuration() const;

	UPROPERTY(Replicated, BlueprintReadOnly, Transient)
	class UWeaponItem* Item;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_PawnOwner)
	class APlayerManager* PawnOwner;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Config)
	FWeaponData WeaponConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Config)
	FHitScanConfiguration HitScanConfig;

	UPROPERTY(Transient)
	float TimerIntervalAdjustment;

	UPROPERTY(Config)
	bool bAllowAutomaticWeaponCatchup = true;

	UPROPERTY(Transient)
	UAudioComponent* FireAudioComponent;

	UPROPERTY(EditDefaultsOnly, Category = Effects)
	FName MuzzleAttachPoint;

	UPROPERTY(EditDefaultsOnly, Category = Effects)
	FName AttachSocketFirstPerson;

	UPROPERTY(EditDefaultsOnly, Category = Effects)
	FName AttachSocketThirdPerson;

	UPROPERTY(EditDefaultsOnly, Category = Effects)
	UParticleSystem* MuzzleFX;

	UPROPERTY(Transient)
	UParticleSystemComponent* MuzzleParticleSystemComp;

	UPROPERTY(Transient)
	UParticleSystemComponent* MuzzleParticleSystemCompSecondary;

	UPROPERTY(EditDefaultsOnly, Category = Effects)
	TSubclassOf<UMatineeCameraShake> FireCameraShake;

	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	float AimDownSightTime;

	UPROPERTY(EditDefaultsOnly, Category = Recoil)
	class UCurveVector* RecoilCurve;

	UPROPERTY(EditDefaultsOnly, Category = Recoil)
	float RecoilSpeed;

	UPROPERTY(EditDefaultsOnly, Category = Recoil)
	float RecoilResetSpeed;

	UPROPERTY(EditDefaultsOnly, Category = Effects)
	UForceFeedbackEffect *FireForceFeedback;

	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* FireSound;

	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* FireLoopSound;

	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* FireFinishSound;

	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* OutOfAmmoSound;

	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* ReloadSound;


	UPROPERTY(EditDefaultsOnly, Category = Animation)
	FWeaponAnim ReloadAnim;


	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* EquipSound;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	FWeaponAnim EquipAnim;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	FWeaponAnim FireAnim;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	FWeaponAnim FireAimAnim;

	UPROPERTY(EditDefaultsOnly, Category = Effects)
	uint32 bLoopMuzzleFX : 1;

	UPROPERTY(EditDefaultsOnly, Category = Sound)
	uint32 bLoopFireSound : 1;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	uint32 bLoopFireAnim : 1;

	uint32 bPlayingFireAnim : 1;

	uint32 bIsEquipped : 1;

	uint32 bWansToFire : 1;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_Reload)
	uint32 bPendingReload : 1;

	uint32 bPendingEquip : 1;

	uint32 bRefiring : 1;

	EWeaponState CurrentState;

	float LastFireTime;

	float EquipStartedTime;

	float EquipDuration;

	UPROPERTY(Transient, Replicated)
	int32 CurrentAmmoInClip;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_BurstCounter)
	int32 BurstCounter;

	FTimerHandle TimerHandle_OnEquipFinished;

	FTimerHandle TimerHandle_StopReload;

	FTimerHandle TimerHandle_ReloadWeapon;

	FTimerHandle TimerHandle_HandleFiring;

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStartFire();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStopFire();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStartReload();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStopReload();

	UFUNCTION()
	void OnRep_PawnOwner();

	UFUNCTION()
	void OnRep_BurstCounter();

	UFUNCTION()
	void OnRep_Reload();

	virtual void SimulateWeaponFire();

	virtual void StopSimulateWeaponFire();

	void HandleHit(const FHitResult& hit, class APlayerManager* hitPlayer = nullptr);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerHandleHit(const FHitResult& hit, class APlayerManager* hitPlayer = nullptr);

	virtual void FireShot();

	UFUNCTION(reliable, server, WithValidation)
	void ServerHandleFiring();

	void HandleReFiring();

	void HandleFiring();

	virtual void OnBurstStarted();

	virtual void OnBurstFinished();

	void SetWeaponState(EWeaponState newState);

	void DetermineWeaponState();

	void AttachMeshToPawn();

	UAudioComponent* PlayWeaponSound(USoundCue* sound);

	float PlayerWeaponAnimation(const FWeaponAnim& animation);

	void StopWeaponAnimation(const FWeaponAnim& animation);

	FVector GetCameraAim() const;

	FHitResult WeaponTrace(const FVector& startTrace, const FVector& endTrace) const;
};
