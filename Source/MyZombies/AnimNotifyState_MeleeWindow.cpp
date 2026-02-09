// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_MeleeWindow.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

void UAnimNotifyState_MeleeWindow::NotifyBegin(USkeletalMeshComponent* Mesh, UAnimSequenceBase*, float, const FAnimNotifyEventReference&)
{
	if (!Mesh) return;
	if(!Mesh->DoesSocketExist(StrikeSocket)) return;
	PrevLoc.Add(Mesh, Mesh->GetSocketLocation(StrikeSocket));
	AlreadyHit.FindOrAdd(Mesh).Reset();
}

void UAnimNotifyState_MeleeWindow::NotifyTick(USkeletalMeshComponent* Mesh, UAnimSequenceBase*, float Dt, const FAnimNotifyEventReference&)
{
	if (!Mesh) return;
	if (!Mesh->DoesSocketExist(StrikeSocket)) return; 

	AActor* Owner = Mesh->GetOwner();
	if (!Owner || !Owner->HasAuthority()) return; // gameplay only on server
          
	const FVector Prev = PrevLoc.FindRef(Mesh);
	const FVector Curr = Mesh->GetSocketLocation(StrikeSocket);
	PrevLoc.Add(Mesh, Curr);

	FCollisionQueryParams qp(SCENE_QUERY_STAT(MeleeSweep), false, Owner);
	FCollisionObjectQueryParams oqp; oqp.AddObjectTypesToQuery(TraceChannel);

	TArray<FHitResult> Hits;
	if (Owner->GetWorld()->SweepMultiByObjectType(
		Hits, Prev, Curr, FQuat::Identity, oqp, FCollisionShape::MakeSphere(SphereRadius), qp))
	{
		TSet<TWeakObjectPtr<AActor>>& HitSet = AlreadyHit.FindOrAdd(Mesh);

		for (const FHitResult& H : Hits)
		{
			AActor* Other = H.GetActor();
			if (!Other || Other == Owner || HitSet.Contains(Other)) continue;

			// Apply gameplay effect/damage
			UGameplayStatics::ApplyPointDamage(Other, /*Damage*/ 15.f, (Curr-Prev).GetSafeNormal(),
				H, Owner->GetInstigatorController(), Owner, nullptr);

			HitSet.Add(Other);
		}
	}
}

void UAnimNotifyState_MeleeWindow::NotifyEnd(USkeletalMeshComponent* Mesh, UAnimSequenceBase*, const FAnimNotifyEventReference&)
{
	if (!Mesh) return;
	PrevLoc.Remove(Mesh);
	AlreadyHit.Remove(Mesh);
}