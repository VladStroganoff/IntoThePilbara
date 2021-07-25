// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

UCLASS()
class GAPGAME_API APickup : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickup();

	void InitializePickup(const TSubclassOf<class UItem> itemClass, const int32 quanity);

	UFUNCTION(BlueprintImplementableEvent) // implemented in blueprint
	void AlignWithGround();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	class UItem* ItemTemplate;

protected:

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, ReplicatedUsing = OnRep_Item)
	class UItem* Item;
	
	UFUNCTION()
	void OnRep_Item();

	UFUNCTION()
	void OnItemModefied();


	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(class UActorChannel* channel, class FOutBunch* bunch, FReplicationFlags* repFlags) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UFUNCTION()
	void OnTakePickup(class APlayerManager* taker);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* PickupMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Components")
	class UInteractionComponent* InteractionComponent;

};
