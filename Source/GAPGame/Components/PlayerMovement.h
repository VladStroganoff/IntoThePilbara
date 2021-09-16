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
	UPROPERTY(Transient, Replicated)
	bool bIsAiming;

	FORCEINLINE bool IsAiming() const { return bIsAiming; }

	void StartAiming();
	void StopAiming();

	void UseThrowable();

	UPROPERTY(Replicated, BlueprintReadOnly, Category = Movement)
	bool bSprinting;

	bool CanSprint();

	UPROPERTY(EditDefaultsOnly, Category = Movement)
	float SprintSpeed;

	UPROPERTY()
	float WalkSpeed;

	void SetSprinting(const bool bNewSprinting);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetSprinting(const bool bNewSprinting);

	void SetuInput(class UInputComponent* PlayerInputComponent);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;


		
};
