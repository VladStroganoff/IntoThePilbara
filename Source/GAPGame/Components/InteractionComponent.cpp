// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionComponent.h"
#include "Player/PlayerManager.h"
#include "Widgets/InteractionWidget.h"


UInteractionComponent::UInteractionComponent()
{
	SetComponentTickEnabled(false);
	InteractionTime = 0.0f;
	InteractionDistance = 200.0f;
	InteractionNameText = FText::FromString("InteractableObject");
	InteractionActionText = FText::FromString("Interact");
	bAllowMutiplayerInteraction = true;


	Space = EWidgetSpace::Screen;
	DrawSize = FIntPoint(600, 100);
	bDrawAtDesiredSize = true;


	SetActive(true);
	SetHiddenInGame(true);
}

void UInteractionComponent::SetInteractableNameText(const FText & NewNameText)
{
	InteractionNameText = NewNameText;
	RefreshWidget();
}

void UInteractionComponent::SetInteractableActionText(const FText & NewActionText)
{
	InteractionActionText = NewActionText;
	RefreshWidget();
}

void UInteractionComponent::Deactivate()
{
	Super::Deactivate();
	for (int32 i = Interactors.Num() - 1; i >= 0; i--)
	{
		if (APlayerManager* interactor = Interactors[i])
		{
			EndFocus(interactor);
			EndInteract(interactor);
		}
	}

	Interactors.Empty();
}

bool UInteractionComponent::CanInteract(APlayerManager * Character) const
{
	const bool bPlayerAlreadyInteracting = !bAllowMutiplayerInteraction && Interactors.Num() >= 1;
	return !bPlayerAlreadyInteracting && IsActive() && GetOwner() != nullptr && Character != nullptr;
}

void UInteractionComponent::RefreshWidget()
{
	if (!bHiddenInGame && GetOwner()->GetNetMode() != NM_DedicatedServer)
	{
		if (UInteractionWidget* interactionWidget = Cast<UInteractionWidget>(GetUserWidgetObject()))
		{
			interactionWidget->UpdateInteractionWidget(this);
		}
	}
}

void UInteractionComponent::BeginFocus(class APlayerManager* Character)
{
	if (!IsActive() || !GetOwner() || !Character)
		return;

	OnBeginFocus.Broadcast(Character); 

	SetHiddenInGame(false);

	if (!GetOwner()->HasAuthority())
	{
		for (auto& VisualComp : GetOwner()->GetComponentsByClass(UPrimitiveComponent::StaticClass()))
		{
			if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(VisualComp))
			{
				Prim->SetRenderCustomDepth(true);
			}
		}
	}
	RefreshWidget();
}

void UInteractionComponent::EndFocus(APlayerManager * character)
{
	OnEndFocus.Broadcast(character);

	SetHiddenInGame(true);

	if (!GetOwner()->HasAuthority())
	{
		for (auto& VisualComp : GetOwner()->GetComponentsByClass(UPrimitiveComponent::StaticClass()))
		{
			if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(VisualComp))
			{
				Prim->SetRenderCustomDepth(false);
			}
		}
	}
}

void UInteractionComponent::BeginInteract(APlayerManager * character)
{
	if (CanInteract(character))
	{
		Interactors.AddUnique(character);
		OnBeginInteract.Broadcast(character);
	}
}

void UInteractionComponent::EndInteract(APlayerManager * character)
{
	Interactors.RemoveSingle(character);
	OnEndInteract.Broadcast(character);
}

void UInteractionComponent::Interact(APlayerManager * character)
{

	if (CanInteract(character))
		OnInteract.Broadcast(character);
}

float UInteractionComponent::GetInteractPercentidge()
{
	if (Interactors.IsValidIndex(0))
	{
		if (APlayerManager* interactor = Interactors[0])
		{
			if (interactor && interactor->IsInteracting())
			{
				return 1.0f - FMath::Abs(interactor->GetRemainingInteractTime()/InteractionTime);
			}
		}
	}
	return 0.0f;
}
