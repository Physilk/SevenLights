// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "SevenLigthsCharacter.h"
#include "SevenLigthsProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"

#include "Engine.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// ASevenLigthsCharacter

ASevenLigthsCharacter::ASevenLigthsCharacter()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.

	// Create VR Controllers.
	R_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("R_MotionController"));
	R_MotionController->Hand = EControllerHand::Right;
	R_MotionController->SetupAttachment(RootComponent);
	L_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("L_MotionController"));
	L_MotionController->SetupAttachment(RootComponent);

    UseMaxDistance = 150.0f;

	// Uncomment the following line to turn motion controllers on by default:
	//bUsingMotionControllers = true;
}

void ASevenLigthsCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    CheckForUsable();
}

void ASevenLigthsCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
}

//////////////////////////////////////////////////////////////////////////
// Input

void ASevenLigthsCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

    //Uncomment to able jumping
	//PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	//PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	//InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &ASevenLigthsCharacter::TouchStarted);
	if (EnableTouchscreenMovement(PlayerInputComponent) == false)
	{
		PlayerInputComponent->BindAction("Use", IE_Pressed, this, &ASevenLigthsCharacter::OnUse);
	}
    PlayerInputComponent->BindAction("ShowInventory", IE_Pressed, this, &ASevenLigthsCharacter::ShowInventory);

	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ASevenLigthsCharacter::OnResetVR);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASevenLigthsCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASevenLigthsCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ASevenLigthsCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ASevenLigthsCharacter::LookUpAtRate);
}

void ASevenLigthsCharacter::OnUse()
{
    if (UsableFocused != NULL)
    {
        if (Controller != NULL) // we access the controller, make sure we have one, else we will crash
        {
            UsableFocused->OnUsed(this);
        }
    }
}

void ASevenLigthsCharacter::CheckForUsable()
{
    if (Controller && Controller->IsLocalPlayerController()) // we check the controller becouse we dont want bots to grab the use object and we need a controller for the Getplayerviewpoint function
    {
        FVector CamLoc;
        FRotator CamRot;

        Controller->GetPlayerViewPoint(CamLoc, CamRot); // Get the camera position and rotation
        const FVector StartTrace = CamLoc; // trace start is the camera location
        const FVector Direction = CamRot.Vector();
        const FVector EndTrace = StartTrace + Direction * UseMaxDistance; // and trace end is the camera location + an offset in the direction you are looking, the 200 is the distance at wich it checks
        //StartTrace = StartTrace + Direction * 150.0f;

        const FName TraceTag("UsableItemTraceTag");
        // Perform trace to retrieve hit info
        FCollisionQueryParams TraceParams = FCollisionQueryParams(TraceTag);
        TraceParams.bTraceAsyncScene = true;
        TraceParams.bReturnPhysicalMaterial = true;

        //GetWorld()->DebugDrawTraceTag = TraceTag;
        TraceParams.TraceTag = TraceTag;
        TraceParams.bFindInitialOverlaps = false;
        TraceParams.AddIgnoredActor(this);

        FCollisionObjectQueryParams ObjectTraceParam = FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllObjects);
        FHitResult Hit(ForceInit);

        UsableFocused = NULL; // nothing, so we set the UseFocus pointer to NULL, so it wont give problems
        if (GetWorld()->LineTraceSingleByObjectType(Hit, StartTrace, EndTrace, ObjectTraceParam, TraceParams)) // simple trace function
        {
            IUsable* usable = dynamic_cast<IUsable *>(Hit.GetActor()); // we cast the hit actor to the IUsable interface
            if (usable) // we are looking to a usable object
            {
                UsableFocused = usable; // as the actor under crosshairs is a usable actor, we store it for the hud.
            }
        }
    }
}

void ASevenLigthsCharacter::ShowInventory()
{
    for (auto& item : Inventory)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Item: %s"), *item));
    }
}

void ASevenLigthsCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ASevenLigthsCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void ASevenLigthsCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnUse();
	}
	TouchItem.bIsPressed = false;
}

//Commenting this section out to be consistent with FPS BP template.
//This allows the user to turn without using the right virtual joystick

//void ASevenLigthsCharacter::TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location)
//{
//	if ((TouchItem.bIsPressed == true) && (TouchItem.FingerIndex == FingerIndex))
//	{
//		if (TouchItem.bIsPressed)
//		{
//			if (GetWorld() != nullptr)
//			{
//				UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
//				if (ViewportClient != nullptr)
//				{
//					FVector MoveDelta = Location - TouchItem.Location;
//					FVector2D ScreenSize;
//					ViewportClient->GetViewportSize(ScreenSize);
//					FVector2D ScaledDelta = FVector2D(MoveDelta.X, MoveDelta.Y) / ScreenSize;
//					if (FMath::Abs(ScaledDelta.X) >= 4.0 / ScreenSize.X)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.X * BaseTurnRate;
//						AddControllerYawInput(Value);
//					}
//					if (FMath::Abs(ScaledDelta.Y) >= 4.0 / ScreenSize.Y)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.Y * BaseTurnRate;
//						AddControllerPitchInput(Value);
//					}
//					TouchItem.Location = Location;
//				}
//				TouchItem.Location = Location;
//			}
//		}
//	}
//}

void ASevenLigthsCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void ASevenLigthsCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void ASevenLigthsCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ASevenLigthsCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

bool ASevenLigthsCharacter::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	bool bResult = false;
	if (FPlatformMisc::GetUseVirtualJoysticks() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		bResult = true;
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &ASevenLigthsCharacter::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &ASevenLigthsCharacter::EndTouch);

		//Commenting this out to be more consistent with FPS BP template.
		//PlayerInputComponent->BindTouch(EInputEvent::IE_Repeat, this, &ASevenLigthsCharacter::TouchUpdate);
	}
	return bResult;
}

void ASevenLigthsCharacter::AddItemToInventory(FString& item)
{
    Inventory.Add(item);
}
