// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MyHUD.generated.h"


class UCharacterStats;
class UKillDeathStats;
class UTexture2D;
class APlayerController;

// MyHUD doesnt know which textures to draw to screen - we can make STRCUT to hold crosshairs to draw each frame
USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()

public:
	UTexture2D* CrosshairsCenter = nullptr;
	UTexture2D* CrosshairsLeft   = nullptr;
	UTexture2D* CrosshairsRight  = nullptr;
	UTexture2D* CrosshairsTop    = nullptr;
	UTexture2D* CrosshairsBottom = nullptr;

	float CrosshairSpread = 0.f;
	FLinearColor CrosshairsColor = FLinearColor::White;
};
/**
 * 
 */
UCLASS()
class MYZOMBIES_API AMyHUD : public AHUD
{
	GENERATED_BODY()

public:
	// --- Core overrides ---
	virtual void DrawHUD() override;
	virtual void BeginPlay() override;

	// --- Crosshair ---
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread);

	// --- Widget management ---
	void AddCharacterStats();
	void AddKillDeathWidget();

	// --- Widget references (instances) ---
	UPROPERTY()
	UCharacterStats* CharacterStats = nullptr;

	UPROPERTY()
	UKillDeathStats* KillDeathStats = nullptr;

	// --- Widget classes (set in editor/blueprints) ---
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> CharacterStatsClass;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> KillDeathStatsClass;

private:
	// --- Crosshair package ---
	FHUDPackage HUDPackage;

	// Max spread for crosshairs
	UPROPERTY(EditAnywhere, Category = "Crosshair")
	float CrosshairSpreadMax = 10.f;
};