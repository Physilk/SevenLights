// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SevenLigthsCharacter.h"
#include "Usable.h"
#include "Item.generated.h"

UCLASS()
class SEVENLIGTHS_API AItem : public AActor, public IUsable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere)
    UStaticMeshComponent* SM_Item;

    UPROPERTY(EditAnywhere)
    FString ItemName = FString(TEXT(""));

    virtual void OnUsed(ASevenLigthsCharacter* User) override;
};
