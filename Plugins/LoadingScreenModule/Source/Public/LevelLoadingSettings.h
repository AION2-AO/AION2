// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "LevelLoadingSettings.generated.h"

// Loading Scene 처리 정보를 담는 구조체.
USTRUCT(BlueprintType)
struct FLevelLoadingScreenEntry
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere, Category = "Loading Screen", meta = (AllowedClass = "World"))
	FSoftObjectPath Map;

	UPROPERTY(Config, EditAnywhere, Category = "Loading Screen", meta = (AllowedClass = "Texture"))
	FSoftObjectPath BackgroundImage;

	// Content/Movies 안의 파일명. 확장자 없이 입력: IntroLoading
	UPROPERTY(Config, EditAnywhere, Category = "Loading Screen")
	FString MoviePath;
};

/**
 * 
 */
// Loading screen settings for the game project
UCLASS(Config = "Game", DefaultConfig, meta = (DisplayName = "Loading Screen"))
class LOADINGSCREENMODULE_API ULevelLoadingSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	// Array of soft references to map assets that should display loading screens
	// Config - Property will be saved in config file.
	// EditAnywhere - Property can be edited in any property window
	// AllowedClasses = "World" - Only World assets can be selected.
	UPROPERTY(Config, EditAnywhere, Category = "Loading Screen", meta = (AllowedClass = "World"))
	TArray<FLevelLoadingScreenEntry> LoadingScreens;

	// Soft reference to the background image asset for the loading screen
	// Config - Property will be saved in config file
	// EditAnywhere - Property can be edited in any property window
	// AllowedClasses = "Texture" - Only Texture assets can be selected.
	UPROPERTY(Config, EditAnywhere, Category = "Loading Screen", meta = (AllowedClass = "Texture"))
	FSoftObjectPath DefaultBackgroundImage;

	// Minimum duration (in seconds) that the loading screen will be displayed
	// Default value of 2.0 seconds
	UPROPERTY(Config, EditAnywhere, Category = "Loading Screen")
	float MinimumLoadingScreenDisplayTime = 5.0f;
};
