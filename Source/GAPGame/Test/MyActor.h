
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMyDelegate, float, myFloat);

UCLASS()
class GAPGAME_API AMyActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AMyActor();

	UPROPERTY(EditAnywhere, Category = "Kuken")
	class UMyActorComponent* MyCoponent;

	UPROPERTY(EditAnywhere, Category = "Kuken")
	class UMeshComponent* EditAnywhere;

	UPROPERTY(BlueprintReadOnly, Category = "Kuken")
	class UMeshComponent* BlueprintReadOnly;

	UPROPERTY(BlueprintAssignable, Category = "Kuken")
	FMyDelegate BlueprintAssignable;

	UFUNCTION(BlueprintCallable, Category = "Kuken")
	class UMeshComponent* BlueprintCallable();

//protected:
//	virtual void BeginPlay() override;
//
//public:	
//	virtual void Tick(float DeltaTime) override;

};
