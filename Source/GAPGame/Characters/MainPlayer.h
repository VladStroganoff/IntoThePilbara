
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Items/EquippableItem.h"
#include "PlayerManager.h"
#include "MainPlayer.generated.h"

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquippedItemsChanged, const EEquippableSlot, Slot, const UEquippableItem*, Item);

UCLASS()
class GAPGAME_API AMainPlayer : public APawn
{
	GENERATED_BODY()



public:
	AMainPlayer();
	

};
