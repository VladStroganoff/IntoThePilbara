// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Items/ThrowableItem.h"
#include "Items/EquippableItem.h"
#include "PlayerManager.h"
#include "Components/ActorComponent.h"
#include "PlayerInputComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAPGAME_API UPlayerInputComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPlayerInputComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;


public:	
	class APlayerManager* _playerManager;
	UPROPERTY(VisibleAnywhere, Category = "Items")
	TMap<EEquippableSlot, UEquippableItem*> EquippedItems;

	UFUNCTION(BlueprintPure)
	FORCEINLINE TMap<EEquippableSlot, UEquippableItem*> GetEquippedItems() const { return EquippedItems; };

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	FTimerHandle _timerHandleInteract;

	UFUNCTION(Server, Reliable)
	void ServerUseThrowable();
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayThrowableTossFX(class UAnimMontage* montageToPlay);

	void UseThrowable();
	void SpawnThrowable();
		
};
