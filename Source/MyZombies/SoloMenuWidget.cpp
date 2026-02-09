// Fill out your copyright notice in the Description page of Project Settings.


#include "SoloMenuWidget.h"
#include "BaseMenuWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"


void USoloMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (EasyButton)   { EasyButton->OnClicked.RemoveAll(this);   EasyButton->OnClicked.AddDynamic(this, &USoloMenuWidget::OnEasyButtonClicked); }
	if (MediumButton) { MediumButton->OnClicked.RemoveAll(this); MediumButton->OnClicked.AddDynamic(this, &USoloMenuWidget::OnMediumButtonClicked); }
	if (HardButton)   { HardButton->OnClicked.RemoveAll(this);   HardButton->OnClicked.AddDynamic(this, &USoloMenuWidget::OnHardButtonClicked); }
	if (BackButton)   { BackButton->OnClicked.RemoveAll(this);   BackButton->OnClicked.AddDynamic(this, &USoloMenuWidget::OnBackButtonClicked); }
}

UWidget* USoloMenuWidget::GetDefaultFocusWidget_Implementation() const
{
	// Pick the first button you want focused when menu opens
	if (HardButton) return HardButton;
	if (MediumButton)    return MediumButton;
	if (EasyButton)       return EasyButton;


	return Super::GetDefaultFocusWidget_Implementation();
}


void USoloMenuWidget::OnEasyButtonClicked()
{
    UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("/Game/GameAssets/Levels/Zombies_Level")),
    true, TEXT("Difficulty=Easy"));
}

void USoloMenuWidget::OnMediumButtonClicked()
{
    UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("/Game/GameAssets/Levels/Zombies_Level")),
    true, TEXT("Difficulty=Medium"));
}

void USoloMenuWidget::OnHardButtonClicked()
{
    UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("/Game/GameAssets/Levels/Zombies_Level")),
    true, TEXT("Difficulty=Hard"));
}

void USoloMenuWidget::OnBackButtonClicked()
{
	RequestPopMenu();
}