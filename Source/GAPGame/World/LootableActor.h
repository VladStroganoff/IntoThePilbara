// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LootableActor.generated.h"

UCLASS()
class GAPGAME_API ALootableActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ALootableActor();
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* LootContainerMesh;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	class UInteractionComponent* LootInteraction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	class UInventoryComponent* Inventory;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	class UDataTable* LootTable;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	FIntPoint LootRolls;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UFUNCTION()
	void OnInteraction(class APlayerManager* player);


};
