// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "CollisionQueryParams.h"
#include "Camera/CameraComponent.h"
#include "MyCharacter.generated.h"


UCLASS()
class PROJECTNAMEFORNOW_API AMyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PossessedBy(AController* NewController) override;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_Walk = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_Look = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_Crouch = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_Sprint = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_Interact = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* IMC_Default = nullptr;

	UPROPERTY(BlueprintReadOnly)
	USceneComponent* CameraPivotRef;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FirstPersonCameraRef;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float WalkSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float SprintSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float CrouchSpeed = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float CameraInterpSpeed = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	float InteractionDistance = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool isSprinting;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool CanSprint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool isCrouching;

	bool CanStand;

	void UpdateMovementSpeed();
	void CheckForObjectAbove();
	void CheckCrouching();

	float TargetCameraZ = 60.f;

	FTimerHandle AboveTraceTimerHandler;
	FTimerHandle CheckCrouchingTimerHandler;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private: 

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartCrouch(const FInputActionValue& Value);
	void StopCrouch(const FInputActionValue& Value);
	void StartSprinting(const FInputActionValue& Value);
	void StopSprinting(const FInputActionValue& Value);
	void InteractionLineTrace(const FInputActionValue& Value);

};
