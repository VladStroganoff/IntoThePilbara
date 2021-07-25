// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SurvyvalPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class GAPGAME_API ASurvyvalPlayerController : public APlayerController
{
	GENERATED_BODY()
public: 

	ASurvyvalPlayerController();

	virtual void SetupInputComponent() override;

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void ClientShowNotification(const FText& message);

	UFUNCTION(BlueprintImplementableEvent)
	void ShowNotification(const FText& message);

	UFUNCTION(BlueprintImplementableEvent)
	void ShowDeathScreen(class APlayerManager* killer);

	UFUNCTION(BlueprintImplementableEvent)
	void ShowLootMenu(const class UInventoryComponent* LootSource);

	UFUNCTION(BlueprintImplementableEvent)
	void ShowUI();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void HideLootMenu();
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnHitPlayer();
	
	UFUNCTION(BlueprintCallable, Category = "Player Controller")
	void Respawn();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRespawn();

	void ApplyRecoil(const FVector2D& recoilAmount, const float recoilSpeed, const float recoilResetSpeed, TSubclassOf<class UMatineeCameraShake> shake = nullptr);

	UPROPERTY(VisibleAnywhere, Category = "Recoil")
	FVector2D RecoilBumpAmount;

	UPROPERTY(VisibleAnywhere, Category = "Recoil")
	FVector2D RecoilResetAmount;

	UPROPERTY(VisibleAnywhere, Category = "Recoil")
	float CurrentRecoilSpeed;

	UPROPERTY(VisibleAnywhere, Category = "Recoil")
	float CurrentRecoilResetSpeed;

	UPROPERTY(VisibleAnywhere, Category = "Recoil")
	float LastRecoilTime;

	void Turn(float rate);
	void LookUp(float rate);
	void StartReload();
};
