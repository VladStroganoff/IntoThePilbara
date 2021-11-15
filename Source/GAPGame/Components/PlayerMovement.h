// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Player/PlayerManager.h"
#include "Components/ActorComponent.h"
#include "PlayerMovement.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAPGAME_API UPlayerMovement : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPlayerMovement();
	
	UPROPERTY(EditAnywhere, Category = "Managers")
	class APlayerManager* PlayerManager;

	void Inject(APlayerManager* playerManager);
	void SetuInput(APlayerManager* PlayerInputComponent);

	void StartCrouching();
	void StopCrouching();
	void MoveForward(float val);
	void MoveRight(float val);
	void LookUp(float val);
	void Turn(float val);

	void Jump();
	void StopJumping();

	void StartFire();
	void StopFire();

	void BeginInteract();
	void EndInteract();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerBeginInteract();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEndInteract();

	void StartSprinting();
	void StopSprinting();

	bool CanAim();
	//UPROPERTY(Transient, Replicated)
	//bool bIsAiming;

	//UFUNCTION(BlueprintPure, Category = "Weapons")
	//bool IsAiming() const;

	void StartAiming();
	void StopAiming();

	void SetAiming(const bool bNewAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(const bool bNewAiming);

	void UseThrowable();

	UFUNCTION(Server, Reliable)
	void ServerUseThrowable();

	//UPROPERTY(Replicated, BlueprintReadOnly, Category = Movement)
	//bool bSprinting;

	bool CanSprint();

	UPROPERTY(EditDefaultsOnly, Category = Movement)
	float SprintSpeed;

	UPROPERTY()
	float WalkSpeed;

	void SetSprinting(const bool bNewSprinting);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetSprinting(const bool bNewSprinting);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;


		
};
