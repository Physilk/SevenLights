// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Usable.generated.h"


// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UUsable : public UInterface
{
	GENERATED_BODY()
};


class ASevenLigthsCharacter;
/**
 * 
 */
class SEVENLIGTHS_API IUsable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
    virtual void OnUsed(ASevenLigthsCharacter * user);
	
};
