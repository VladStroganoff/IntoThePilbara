
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Items/EquippableItem.h"
#include "PlayerManager.h"
#include "MainPlayer.generated.h"

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquippedItemsChanged, const EEquippableSlot, Slot, const UEquippableItem*, Item);

UCLASS()
class GAPGAME_API AMainPlayer : public APawn
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


public:
	AMainPlayer();
	UPROPERTY(BlueprintReadOnly, Category = Mesh)
	TMap<EEquippableSlot, USkeletalMesh*> NakedMeshes;

	UPROPERTY(BlueprintReadOnly, Category = Mesh)
	TMap<EEquippableSlot, USkeletalMeshComponent*> PlayerMeshes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
		class UInventoryComponent* PlayerInventory;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
		class UInteractionComponent* LootPlayerInteraction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* SpringArmComponent;

	UPROPERTY(EditAnywhere, Category = "Components")
		class UCameraComponent* CameraComponent;

	UPROPERTY(EditAnywhere, Category = "Components")
		class USkeletalMeshComponent* HelmetMesh;
	UPROPERTY(EditAnywhere, Category = "Components")
		class USkeletalMeshComponent* TorsoMesh;
	UPROPERTY(EditAnywhere, Category = "Components")
		class USkeletalMeshComponent* LegsMesh;
	UPROPERTY(EditAnywhere, Category = "Components")
		class USkeletalMeshComponent* LeftArmMesh;
	UPROPERTY(EditAnywhere, Category = "Components")
		class USkeletalMeshComponent* RightArmMesh;

	UPROPERTY(EditAnywhere, Category = "Components")
		class USkeletalMeshComponent* JacketMesh;
	UPROPERTY(EditAnywhere, Category = "Components")
		class USkeletalMeshComponent* PantsMesh;
	UPROPERTY(EditAnywhere, Category = "Components")
		class USkeletalMeshComponent* BackpackMesh;

	UFUNCTION(BlueprintCallable)
		void SetLootSource(class UInventoryComponent* newLootSource);

	UFUNCTION(BlueprintPure, Category = "Looting")
		bool IsLooting() const;

	UFUNCTION(BlueprintCallable, Category = "Looting")
		void LootItem(class UItem* itemToGive);

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerLootItem(class UItem* itemToLoot);


protected:

	UFUNCTION()
		void BeginLootingPlayer(class APlayerManager* player);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
		void ServerSetLootSource(class UInventoryComponent* newLootSource);

	UPROPERTY(ReplicatedUsing = OnRep_LootSource, BlueprintReadOnly)
		UInventoryComponent* LootSource;

	UFUNCTION()
		void OnLootSourceOwnerDestroyed(AActor* destroyedActor);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
		void OnRep_LootSource();

	virtual void Restart() override;
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamagaeCauser) override;
	virtual void SetActorHiddenInGame(bool bNewHidden) override;

	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
		float InteractionCheckFrequency;

	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
		float InteractionCheckDistence;


	void PreformInteractionCheck();

	void CouldNotFindInteractable();
	void FoundInteractable(UInteractionComponent* Interactable);

	void BeginInteract();
	void EndInteract();

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerBeginInteract();

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerEndInteract();

	void Interact();

	//UPROPERTY()
		FInteractionData InteractionData;


	FORCEINLINE class UInteractionComponent* GetInteractable() const { return InteractionData.ViewedInteractionComponent; }

	FTimerHandle _timerHandleInteract;

	UFUNCTION(Server, Reliable)
		void ServerUseThrowable();
	UFUNCTION(NetMulticast, Unreliable)
		void MulticastPlayThrowableTossFX(class UAnimMontage* montageToPlay);

	class UThrowableItem* GetThrowable() const;
	void UseThrowable();
	void SpawnThrowable();
	bool CanUseThrowable() const;

	public:
		bool IsInteracting();
		float GetRemainingInteractTime();

		UFUNCTION(BlueprintCallable, Category = "Items")
			void UseItem(class UItem* item);

		UFUNCTION(Server, Reliable, WithValidation)
			void ServerUseItem(class UItem* item);

		UFUNCTION(BlueprintCallable, Category = "Items")
			void DropItem(class UItem* item, const int32 quantity);

		UFUNCTION(Server, Reliable, WithValidation)
			void ServerDropItem(class UItem* item, int32 quantity);

		UPROPERTY(EditDefaultsOnly, Category = "Item")
			TSubclassOf<class APickup> PickupClass;

		bool EquipItem(class UEquippableItem* item);
		bool UnEquipItem(class UEquippableItem* item);

		void EquipWeapon(class UWeaponItem* weaponItem);
		void UnEquipWeapon();

		void EquipWear(USkeletalMesh* wear, EEquippableSlot slot);
		void EquipWear(class UWearItem* wear);
		void UnEquipWear(const EEquippableSlot slot);



		//UPROPERTY(BlueprintAssignable, Category = "Items")
			FOnEquippedItemsChanged OnEquippedItemsChanged;

		UFUNCTION(BlueprintPure)
			class USkeletalMeshComponent* GetSlotSkeletalMeshComponent(const EEquippableSlot Slot);

		UFUNCTION(BlueprintPure)
			FORCEINLINE TMap<EEquippableSlot, UEquippableItem*> GetEquippedItems() const { return EquippedItems; };

		UFUNCTION(BlueprintCallable, Category = "Weapons")
			FORCEINLINE class AWeapon* GetEquippedWeapon() const { return EquippedWeapon; }

		float ModifyHealth(const float delta);

		UFUNCTION()
			void OnRep_Health(float oldHealth);

		UFUNCTION(BlueprintImplementableEvent)
			void OnHealthModified(const float healthDelta);
		protected:

			UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_EquippedWeapon)
				class AWeapon* EquippedWeapon;

			UFUNCTION()
				void OnRep_EquippedWeapon();

			void StartFire();
			void StopFire();

			void BeginMeleeAttack();

			UFUNCTION(Server, Reliable)
				void ServerProcessMeleeHit(const FHitResult& meleeHit);

			UFUNCTION(NetMulticast, Unreliable)
				void MulticastPlayerMeleeFX();

			UPROPERTY()
				float LastMeleeAttackTime;

			UPROPERTY(EditDefaultsOnly, Category = Melee)
				float MeleeAttackDistance;

			UPROPERTY(EditDefaultsOnly, Category = Melee)
				float MeleeAttackDamage;

			UPROPERTY(EditDefaultsOnly, Category = Melee)
				TSubclassOf<class UDamageType> MeleeDamageType;

			UPROPERTY(EditDefaultsOnly, Category = Melee)
				class UAnimMontage* MeleeAttackMontage;


			void Suicide(struct FDamageEvent const& damageEvent, const AActor* damageCauser);
			void KilledByPlayer(struct FDamageEvent const& DamageEvent, class APlayerManager* character, const AActor* damageCauser);

			UPROPERTY(ReplicatedUsing = OnRep_Killer)
				class APlayerManager* Killer;

			UFUNCTION()
				void OnRep_Killer();

			UFUNCTION(BlueprintImplementableEvent)
				void OnDeath();

			UPROPERTY(VisibleAnywhere, Category = "Items")
				TMap<EEquippableSlot, UEquippableItem*> EquippedItems;

			UPROPERTY(ReplicatedUsing = OnRep_Health, BlueprintReadOnly, Category = "Health")
				float Heath;

			UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
				float MaxHealth;

			UPROPERTY(EditDefaultsOnly, Category = Movement)
				float SprintSpeed;

			UPROPERTY()
				float WalkSpeed;

			UPROPERTY(Replicated, BlueprintReadOnly, Category = Movement)
				bool bSprinting;

			bool CanSprint();

			void StartSprinting();
			void StopSprinting();

			void SetSprinting(const bool bNewSprinting);

			UFUNCTION(Server, Reliable, WithValidation)
				void ServerSetSprinting(const bool bNewSprinting);


			void StartCrouching();
			void StopCrouching();
			void MoveForward(float val);
			void MoveRight(float val);
			void LookUp(float val);
			void Turn(float val);

			bool CanAim();

			void StartAiming();
			void StopAiming();

			void SetAiming(const bool bNewAiming);

			UFUNCTION(Server, Reliable)
				void ServerSetAiming(const bool bNewAiming);

			UPROPERTY(Transient, Replicated)
				bool bIsAiming;

			public:

				void StartReload();

				UFUNCTION(BlueprintPure)
					FORCEINLINE bool IsAlive() const { return Killer == nullptr; };


				UFUNCTION(BlueprintPure, Category = "Weapons")
					FORCEINLINE bool IsAiming() const { return bIsAiming; }

				// Called to bind functionality to input
				virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
