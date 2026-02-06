// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterStats.h"
#include "Components/ProgressBar.h" 
#include "Components/TextBlock.h" 

/*need to add variables for progress bar and health text*/

void UCharacterStats::SetHealthBar(UProgressBar* NewHealthBar)
{
    HealthBar = NewHealthBar;
}

