

#include "Test/MyActor.h"
#include "Test/MyActorComponent.h"


AMyActor::AMyActor()
{
	MyCoponent = CreateDefaultSubobject<UMyActorComponent>("ComponentYall");
}


UMeshComponent* AMyActor::BlueprintCallable()
{
	return BlueprintReadOnly;
 }

