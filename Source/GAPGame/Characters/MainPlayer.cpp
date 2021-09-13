

#include "Characters/MainPlayer.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"
#include "SurvyvalPlayerController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/InteractionComponent.h"																								 
#include "Items/EquippableItem.h"																											 
#include "Items/WearItem.h"				
#include "Items/WeaponItem.h"
#include "Items/ThrowableItem.h"
#include "Materials/MaterialInstance.h"																										 
#include "World/Pickup.h"																													 
#include "GameFramework/PlayerState.h"																										 
#include "GameFramework/SpringArmComponent.h"																								 
#include "GameFramework/CharacterMovementComponent.h"																						 
#include "GameFramework/DamageType.h"																										 
#include "Weapons/MeleeDamage.h"
#include "Weapons/Weapon.h"
#include "Weapons/ThrowableWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "GAPGame.h"

#define LOCTEXT_NAMESPACE "SurvivalCharacter"
static FName NAME_AimDownSightsSocket("aimDownSightsSocket");

AMainPlayer::AMainPlayer()
{
	PrimaryActorTick.bCanEverTick = true;


}




