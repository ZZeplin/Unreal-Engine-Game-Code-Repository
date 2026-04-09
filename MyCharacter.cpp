// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacter.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Interactable.h"


// Sets default values
AMyCharacter::AMyCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void AMyCharacter::PossessedBy(AController* NewController) {

	Super::PossessedBy(NewController);

}

// Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(AboveTraceTimerHandler, this, &AMyCharacter::CheckForObjectAbove, 0.25f, true);

	FirstPersonCameraRef = Cast<UCameraComponent>(GetDefaultSubobjectByName(TEXT("FirstPersonCamera")));

	CameraPivotRef = Cast<USceneComponent>(GetDefaultSubobjectByName(TEXT("CameraPivot")));
}

// Called every frame
void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector CurrentCameraLoc = CameraPivotRef->GetRelativeLocation();
	FVector TargetCameraLoc = FVector(0, 0, TargetCameraZ);

	FVector NewCameraLoc = FMath::VInterpTo(CurrentCameraLoc, TargetCameraLoc, DeltaTime, CameraInterpSpeed);

	CameraPivotRef->SetRelativeLocation(NewCameraLoc);

}

// Called to bind functionality to input
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	APlayerController* PC = Cast<APlayerController>(GetController());

	if (PC) {

		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());

		if (Subsystem) {

			Subsystem->AddMappingContext(IMC_Default, 0);

		}
	}

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if (EIC) {
		EIC->BindAction(IA_Walk, ETriggerEvent::Triggered, this, &AMyCharacter::Move);
		EIC->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AMyCharacter::Look);
		EIC->BindAction(IA_Crouch, ETriggerEvent::Started, this, &AMyCharacter::StartCrouch);
		EIC->BindAction(IA_Crouch, ETriggerEvent::Completed, this, &AMyCharacter::StopCrouch);
		EIC->BindAction(IA_Sprint, ETriggerEvent::Started, this, &AMyCharacter::StartSprinting);
		EIC->BindAction(IA_Sprint, ETriggerEvent::Completed, this, &AMyCharacter::StopSprinting);
		EIC->BindAction(IA_Interact, ETriggerEvent::Triggered, this, &AMyCharacter::InteractionLineTrace);
	}

}

void AMyCharacter::Move(const FInputActionValue& Value) {

	FVector2D Axis = Value.Get<FVector2D>();

	AddMovementInput(GetActorForwardVector(), Axis.Y);
	AddMovementInput(GetActorRightVector(), Axis.X);

}

void AMyCharacter::Look(const FInputActionValue& Value) {

	FVector2D Axis = Value.Get<FVector2D>();

	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);

}

void AMyCharacter::StartCrouch(const FInputActionValue& Value) {

	isCrouching = true;
	CanSprint = false;
	Crouch();

	TargetCameraZ = 30.f;
	UpdateMovementSpeed();


	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Crouched"));

}

void AMyCharacter::StopCrouch(const FInputActionValue& Value) {

	GetWorldTimerManager().SetTimer(CheckCrouchingTimerHandler, this, &AMyCharacter::CheckCrouching, 0.05f, true);

}

void AMyCharacter::CheckCrouching() {

	CheckForObjectAbove();

	if (CanStand) {

		isCrouching = false;
		CanSprint = true;
		UnCrouch();
		
		TargetCameraZ = 60.0f;
		UpdateMovementSpeed();
		GetWorldTimerManager().ClearTimer(CheckCrouchingTimerHandler);

	}
	else {

		TargetCameraZ = 30.f;

	}

}

void AMyCharacter::CheckForObjectAbove() {

	if (isCrouching) {

		FVector Start = GetActorLocation() + FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
		FVector End = Start + FVector(0, 0, 50.f);
		float Radius = 30.f;

		FHitResult HitResult;
		TArray<AActor*> ActorsToIgnore;
		ActorsToIgnore.Add(this);

		bool bHit = UKismetSystemLibrary::SphereTraceSingle(GetWorld(), Start, End, Radius, UEngineTypes::ConvertToTraceType(ECC_Visibility), false, ActorsToIgnore, EDrawDebugTrace::ForDuration, HitResult, true);

		if (bHit && HitResult.GetActor()) {

			CanStand = false;

		}
		else {

			CanStand = true;

		}

	}

}

void AMyCharacter::StartSprinting(const FInputActionValue& Value) {

	isSprinting = true;
	UpdateMovementSpeed();

}

void AMyCharacter::StopSprinting(const FInputActionValue& Value) {

	isSprinting = false;
	UpdateMovementSpeed();

}

void AMyCharacter::InteractionLineTrace(const FInputActionValue& Value) {

	FHitResult HitResult;
	FVector Start = FirstPersonCameraRef->GetComponentLocation();
	FVector ForwardVector = FirstPersonCameraRef->GetForwardVector();
	FVector End = Start + (ForwardVector * InteractionDistance);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);

	DrawDebugLine(GetWorld(), Start, End, FColor::Green, 1.f, 0.f, 2.f);

	if (bHit) {

		AActor* HitActor = HitResult.GetActor();
		UPrimitiveComponent* HitComponent = HitResult.GetComponent();

		if (!HitActor && HitComponent) {

			HitActor = HitComponent->GetOwner();

		}

		if (HitActor && HitActor->Implements<UInteractable>()) {

			IInteractable::Execute_Interact(HitActor, HitComponent);
		}

	}
}

void AMyCharacter::UpdateMovementSpeed() {

	if (isSprinting) {

		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;

	}
	else if (isCrouching) {

		GetCharacterMovement()->MaxWalkSpeed = CrouchSpeed;

	}
	else {

		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	}

}