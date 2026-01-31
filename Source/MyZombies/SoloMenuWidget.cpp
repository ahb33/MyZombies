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


void USoloMenuWidget::OnEasyButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Easy clicked"));

	UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("/Game/GameAssets/Levels/Zombies_Level")));

}

void USoloMenuWidget::OnMediumButtonClicked() {}
void USoloMenuWidget::OnHardButtonClicked() {}

void USoloMenuWidget::OnBackButtonClicked()
{
	RequestPopMenu();
}