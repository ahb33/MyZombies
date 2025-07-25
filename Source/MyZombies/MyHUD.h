// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MyPlayerController.h"
#include "CharacterStats.h"
#include "MyHUD.generated.h"


// MyHUD doesnt know which textures to draw to screen - we can make STRCUT to hold crosshairs to draw each frame
USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY() ; // add generated so reflection system can work with this struct
public:
	class UTexture2D* CrosshairsCenter;
	UTexture2D* CrosshairsLeft;
	UTexture2D* CrosshairsRight;
	UTexture2D* CrosshairsTop;
	UTexture2D* CrosshairsBottom;
	float CrosshairSpread; // this will be changing dynamically in the game
	FLinearColor CrosshairsColor;
};
/**
 * 
 */
UCLASS()
class MYZOMBIES_API AMyHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) {HUDPackage = Package;}

	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread);

	class UCharacterStats* CharacterStats;


	// This will allow you to select the class you need in the HUD when you create the widget
	UPROPERTY(EditAnywhere, Category = "PlayerStats")
	TSubclassOf< class UUserWidget> CharacterStatsClass;
	void AddCharacterStats();


	// void AnnounceNextLevel();
	// create NextLevel class which would create sound
private:
	FHUDPackage HUDPackage;

	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 10.f;

protected:

	virtual void BeginPlay() override;

};
