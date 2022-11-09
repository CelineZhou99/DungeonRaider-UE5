// Fill out your copyright notice in the Description page of Project Settings.

#include "Grabber.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
UGrabber::UGrabber()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UGrabber::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void UGrabber::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UPhysicsHandleComponent *PhysicsHandle = GetPhysicsHandle();

	if (PhysicsHandle && PhysicsHandle->GetGrabbedComponent())
	{
		FVector TargetLocation = GetComponentLocation() + GetForwardVector() * HoldDistance;
		PhysicsHandle->SetTargetLocationAndRotation(TargetLocation, GetComponentRotation());
	}
}

UPhysicsHandleComponent *UGrabber::GetPhysicsHandle() const
{
	UPhysicsHandleComponent *Result = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();
	if (Result == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Grabber requires a UPhysicsHandleComponent"));
	}
	return Result;
}

bool UGrabber::GetGrabbableInReach(FHitResult &OutHitResult) const
{
	FVector Start = GetComponentLocation();
	FVector End = Start + GetForwardVector() * MaxGrabDistance;
	DrawDebugLine(GetWorld(), Start, End, FColor::Red);

	FCollisionShape Sphere = FCollisionShape::MakeSphere(GrabRadius);
	// Identity means no rotation
	return GetWorld()->SweepSingleByChannel(OutHitResult, Start, End, FQuat::Identity, ECC_GameTraceChannel2, Sphere);
}

void UGrabber::Grab()
{
	UE_LOG(LogTemp, Display, TEXT("Grabbed"));

	UPhysicsHandleComponent *PhysicsHandle = GetPhysicsHandle();

	if (PhysicsHandle == nullptr)
	{
		return;
	}

	FHitResult HitResult;
	bool HasHit = GetGrabbableInReach(HitResult);

	if (HasHit)
	{
		UPrimitiveComponent *HitComponent = HitResult.GetComponent();
		HitComponent->SetSimulatePhysics(true);

		HitComponent->WakeAllRigidBodies();
		AActor *HitActor = HitResult.GetActor();
		// get the tags to add a "Grabbed" tag when the actor is grabbed
		HitActor->Tags.Add("Grabbed");
		// we attached actors to the trigger component in triggercomponent.cpp
		// therefore we need to detach it first before picking it up
		HitActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		PhysicsHandle->GrabComponentAtLocationWithRotation(
			HitComponent,
			NAME_None,
			HitResult.ImpactPoint,
			GetComponentRotation());
		// DrawDebugSphere(GetWorld(), HitResult.Location, 10, 10, FColor::Blue, false, 5);
		// DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 10, 10, FColor::Purple, false, 5);
	}
}

void UGrabber::Release()
{
	UE_LOG(LogTemp, Display, TEXT("Released grabber"));

	UPhysicsHandleComponent *PhysicsHandle = GetPhysicsHandle();

	if (PhysicsHandle && PhysicsHandle->GetGrabbedComponent())
	{
		// wake the body up in case its asleep -> which then releasing won't do anything
		PhysicsHandle->GetGrabbedComponent()->WakeAllRigidBodies();
		PhysicsHandle->GetGrabbedComponent()->GetOwner()->Tags.Remove("Grabbed");
		PhysicsHandle->ReleaseComponent();
	}
}
