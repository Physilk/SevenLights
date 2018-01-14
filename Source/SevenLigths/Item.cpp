// Fill out your copyright notice in the Description page of Project Settings.

#include "Item.h"
#include "Engine.h"

// Sets default values
AItem::AItem(const FObjectInitializer& ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

   
    SM_Item = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("Item Mesh"));
    SetRootComponent(SM_Item);
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AItem::OnUsed(ASevenLigthsCharacter* User)
{
    User->AddItemToInventory(ItemName);
    Destroy();
}