// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Item.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnItemModefied);

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	IR_Common		UMETA(DisplayName = "Common"),
	IR_Uncommon		UMETA(DisplayName = "Uncommon"),
	IR_Rare			UMETA(DisplayName = "Rare"),
	IR_VeryRare		UMETA(DisplayName = "Veryrare"),
	IR_Legendary	UMETA(DisplayName = "Legendary")

};

/**
 * 
 */
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class GAPGAME_API UItem : public UObject
{
	GENERATED_BODY()

protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> & OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override;
	virtual class UWorld* GetWorld() const override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& propertyChangedEvent) override;
#endif

public: 
	UItem();

	UPROPERTY(Transient)
	class UWorld* World;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	class UStaticMesh* PickupMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	class UTexture2D* Thumbnail; 

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	FText ItemDisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	FText ItemDescription;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	FText UseActionText;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	EItemRarity ItemRarty;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta = (ClampMin = 0.0))
	float Weight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	bool bStackable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta = (ClampMin = 2, EditCondition = bStackable))
	int32 MaxStackSize;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	TSubclassOf<class UItemToolTip> ItemTooltip;

	UPROPERTY(ReplicatedUsing = OnRep_Quantity, EditAnywhere, Category = "Item", meta = (UIMin = 1, EditCondition = bStackable))
	int32 Quantity;

	UPROPERTY()
	class UInventoryComponent* OwningInventory;

	UPROPERTY()
	int32 RepKey;

	UPROPERTY(BlueprintAssignable)
	FOnItemModefied OnItemModified;

	UFUNCTION()
	void OnRep_Quantity();

	UFUNCTION(BlueprintCallable, Category = "Item")
	void SetQuantity(const int32 newQuantity);

	UFUNCTION(BlueprintPure, Category = "Item")
	int32 GetQuantity() const { return Quantity; }

	UFUNCTION(BlueprintCallable, Category = "Item")
	FORCEINLINE float GetStackWeight() const { return Quantity * Weight; };

	UFUNCTION(BlueprintPure, Category = "Item")
	virtual bool ShouldShowInInventory() const;

	UFUNCTION(BlueprintImplementableEvent)
	void OnUse(class APlayerManager* player);

	virtual void Use(class APlayerManager* player);
	virtual void AddedToInventry(class UInventoryComponent* Inventory);

	void MarkDirtyForReplication();

};
