// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "InteractionComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeginInteract, class APlayerManager*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndInteract, class APlayerManager*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeginFocus, class APlayerManager*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndFocus, class APlayerManager*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteract, class APlayerManager*, Character);


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class GAPGAME_API UInteractionComponent : public UWidgetComponent
{
	GENERATED_BODY()
public:
	UInteractionComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	float InteractionTime;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	float InteractionDistance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	FText InteractionNameText;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	FText InteractionActionText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	bool bAllowMutiplayerInteraction;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetInteractableNameText(const FText& NewNameText);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetInteractableActionText(const FText& NewActionText);

	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnBeginInteract OnBeginInteract;
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnEndInteract OnEndInteract;
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnBeginFocus OnBeginFocus;
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnEndFocus OnEndFocus;
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnInteract OnInteract;
protected:

	virtual void Deactivate() override;

	bool CanInteract(class APlayerManager* Character) const;

	UPROPERTY()
	TArray<class APlayerManager*> Interactors;

public :

	void RefreshWidget();

	void BeginFocus(class APlayerManager* Character);
	void EndFocus(class APlayerManager* Character);

	void BeginInteract(class APlayerManager* Character);
	void EndInteract(class APlayerManager* Character);

	void Interact(class APlayerManager* Character);

	UFUNCTION(BlueprintPure, Category = "Interaction")
	float GetInteractPercentidge();

};
