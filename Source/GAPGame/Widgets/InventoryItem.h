// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryItem.generated.h"

/**
 * 
 */
UCLASS()
class GAPGAME_API UInventoryItem : public UUserWidget
{
	GENERATED_BODY()
	
public: 
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Item", meta = (ExposeOnSpawn = true))
	class UItem* Item;

};
