
#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

UENUM(BlueprintType)
enum class EItemAddResult : uint8
{
	IAR_NoItemsAdded UMETA(DisplayName = "No items added"),
	IAR_SomeItemsAdded UMETA(DisplayName = "Some items added"),
	IAR_AllItemsAdded UMETA(DisplayName = "All items added")
};

USTRUCT(BlueprintType) // so that it works in blueprint as well
struct FItemAddResult
{
	GENERATED_BODY()

public:

	FItemAddResult() {};
	FItemAddResult(int32 inItemQuanity) : AmountToGive(inItemQuanity), ActualAmountGiven(0) {};
	FItemAddResult(int32 inItemQuanity, int32 inQuanityAdded) : AmountToGive(inItemQuanity), ActualAmountGiven(inQuanityAdded) {};

	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	int32 AmountToGive = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	int32 ActualAmountGiven = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	EItemAddResult Result;

	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	FText ErrorText;

	static FItemAddResult AddedNone(const int32 inItemQuantity, const FText& ErrorText)
	{
		FItemAddResult AddedNoneResult(inItemQuantity);
		AddedNoneResult.Result = EItemAddResult::IAR_NoItemsAdded;
		AddedNoneResult.ErrorText = ErrorText;

		return AddedNoneResult;
	}

	static FItemAddResult AddedSome(const int32 inItemQuantity, const int32 actuanItemQuantity, const FText& ErrorText)
	{
		FItemAddResult AddedSomeResult(inItemQuantity, actuanItemQuantity);
		AddedSomeResult.Result = EItemAddResult::IAR_SomeItemsAdded;
		AddedSomeResult.ErrorText = ErrorText;

		return AddedSomeResult;
	}

	static FItemAddResult AddedAll(const int32 inItemQuantity)
	{
		FItemAddResult AddedAllResult(inItemQuantity, inItemQuantity);
		AddedAllResult.Result = EItemAddResult::IAR_AllItemsAdded;

		return AddedAllResult;
	}

};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAPGAME_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()
	friend class UItem;

public:	
	UInventoryComponent();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FItemAddResult TryAddItem(class UItem* Item);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FItemAddResult TryAddItemFromClass(TSubclassOf<class UItem> itemClass, const int32 quanity);

	int32 ConsumeItem(class UItem* item);
	int32 ConsumeItem(class UItem* item, const int32 quantity);

	UFUNCTION(BlueprintCallable, Category = "Inventiry")
	bool RemoveItem(class UItem* item);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasItem(TSubclassOf<class UItem> itemClass, const int32 quanity = 1) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	UItem* FindItem(UItem* item) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	UItem* FindItemByClass(TSubclassOf<class UItem> itemClass) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	TArray<UItem*> FindItemsByClass(TSubclassOf<class UItem> itemClass) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	float GetCurrentWheight() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetWeightCapacity(const float newWeightCapacity);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetCapacity(const int32 newCapacity);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE float GetWeightCapacity() const { return WeightCapacity; };

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE int32 GetCapacity() const { return Capacity; };

	UFUNCTION(BlueprintPure, Category = "Inventiry")
	FORCEINLINE TArray<class UItem*> GetItems() const { return Items; };

	UFUNCTION(Client, Reliable)
	void ClientRefreshInventory();

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryUpdated OnInventoryUpdated;


protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	float WeightCapacity;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = 0, ClampMax = 200))
	int32 Capacity;

	UPROPERTY(ReplicatedUsing = OnRep_Items, VisibleAnywhere, Category = "Inventory")
	TArray<class UItem*> Items;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& outLifetimeProps) const override;
	virtual bool ReplicateSubobjects(class UActorChannel* channel, class FOutBunch* bunch, FReplicationFlags* repFlags) override;
		
private:

	UItem* AddItem(class UItem* item); // for replication instad of Items.Add()

	UFUNCTION()
	void OnRep_Items();

	UPROPERTY()
	int32 ReplicateItemsKey;

	FItemAddResult TryAddItem_Internal(class UItem* Item);

};
