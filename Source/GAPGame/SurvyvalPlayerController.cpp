// Fill out your copyright notice in the Description page of Project Settings.


#include "SurvyvalPlayerController.h"
#include "PlayerManager.h"

ASurvyvalPlayerController::ASurvyvalPlayerController()
{

}


void ASurvyvalPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAxis("Turn", this, &ASurvyvalPlayerController::Turn);
	InputComponent->BindAxis("LookUp", this, &ASurvyvalPlayerController::LookUp);

	InputComponent->BindAction("Reload", IE_Pressed, this, &ASurvyvalPlayerController::StartReload);
}


void ASurvyvalPlayerController::ClientShowNotification_Implementation(const FText& message)
{
	ShowNotification(message);
}

void ASurvyvalPlayerController::Respawn()
{
	UnPossess();
	ChangeState(NAME_Inactive);
	
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerRespawn();
	}
	else
	{
		ServerRestartPlayer();
	}
}

void ASurvyvalPlayerController::ServerRespawn_Implementation()
{
	Respawn();
}

bool ASurvyvalPlayerController::ServerRespawn_Validate()
{
	return true;
}

void ASurvyvalPlayerController::ApplyRecoil(const FVector2D & recoilAmount, const float recoilSpeed, const float recoilResetSpeed, TSubclassOf<class UMatineeCameraShake> shake)
{
	if (IsLocalPlayerController())
	{
		if (PlayerCameraManager)
		{
			PlayerCameraManager->PlayCameraShake(shake); // FPS Shake? maybe remove this for third person
		}

		RecoilBumpAmount += recoilAmount;
		RecoilResetAmount += -recoilAmount;

		CurrentRecoilSpeed = recoilSpeed;
		CurrentRecoilResetSpeed = recoilResetSpeed;

		LastRecoilTime = GetWorld()->GetTimeSeconds();
	}
}

void ASurvyvalPlayerController::Turn(float rate)
{
	if (!FMath::IsNearlyZero(RecoilResetAmount.X, 0.01f))
	{
		if (RecoilResetAmount.X > 0.0f && rate > 0.0f)
		{
			RecoilResetAmount.X = FMath::Max(0.0f, RecoilResetAmount.X - rate);
		}
		else if (RecoilResetAmount.X < 0.0f && rate < 0.0f)
		{
			RecoilResetAmount.X = FMath::Min(0.0f, RecoilResetAmount.X - rate);
		}
	}

	if (!FMath::IsNearlyZero(RecoilBumpAmount.X, 0.01f))
	{
		FVector2D lastCurrentRecoil = RecoilBumpAmount;
		RecoilBumpAmount.X = FMath::FInterpTo(RecoilBumpAmount.X, 0.0f, GetWorld()->GetDeltaSeconds(), CurrentRecoilSpeed);

		AddPitchInput(lastCurrentRecoil.X - RecoilBumpAmount.X);
	}

	FVector2D lastRecoilResetAmount = RecoilResetAmount;
	RecoilResetAmount.X = FMath::FInterpTo(RecoilResetAmount.X, 0.0f, GetWorld()->GetDeltaSeconds(), CurrentRecoilResetSpeed);
	AddPitchInput(lastRecoilResetAmount.X - RecoilResetAmount.X);

	AddYawInput(rate);
}

void ASurvyvalPlayerController::LookUp(float rate)
{
	if (!FMath::IsNearlyZero(RecoilResetAmount.Y, 0.01f))
	{
		if (RecoilResetAmount.Y > 0.0f && rate > 0.0f)
		{
			RecoilResetAmount.Y = FMath::Max(0.0f, RecoilResetAmount.Y - rate);
		}
		else if (RecoilResetAmount.Y < 0.0f && rate < 0.0f)
		{
			RecoilResetAmount.Y = FMath::Min(0.0f, RecoilResetAmount.Y - rate);
		}
	}

	if (!FMath::IsNearlyZero(RecoilBumpAmount.Y, 0.01f))
	{
		FVector2D lastCurrentRecoil = RecoilBumpAmount;
		RecoilBumpAmount.Y = FMath::FInterpTo(RecoilBumpAmount.Y, 0.0f, GetWorld()->GetDeltaSeconds(), CurrentRecoilSpeed);

		AddPitchInput(lastCurrentRecoil.Y - RecoilBumpAmount.Y);
	}

	FVector2D lastRecoilResetAmount = RecoilResetAmount;
	RecoilResetAmount.Y = FMath::FInterpTo(RecoilResetAmount.Y, 0.0f, GetWorld()->GetDeltaSeconds(), CurrentRecoilResetSpeed);
	AddPitchInput(lastRecoilResetAmount.Y - RecoilResetAmount.Y);

	AddPitchInput(rate);
}

void ASurvyvalPlayerController::StartReload()
{
	if (APlayerManager* playerManager = Cast<APlayerManager>(GetPawn()))
	{
		if (playerManager->IsAlive())
		{
			playerManager->StartReload();
		}
		else
		{
			Respawn();
		}
	}
}
