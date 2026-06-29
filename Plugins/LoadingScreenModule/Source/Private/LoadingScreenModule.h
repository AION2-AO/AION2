#pragma once
#include "Modules/ModuleInterface.h"

// Texture의 참조를 다음 Level에 넘어가도 유지
#include "UObject/StrongObjectPtr.h"
#include "ILoadingScreenModule.h"

#include "UObject/SoftObjectPath.h"

class UTexture2D;
struct FStreamableHandle;

// Loading screen module implementation.
// Handles initialization and display of loading screens during gameplay.
class LOADINGSCREENMODULE_API FLoadingScreenModule final: public ILoadingScreenModule
{
public:
	// Called when module is first loaded.
	virtual void StartupModule() override;

	// Called before module is unloaded.
	virtual void ShutdownModule() override;

	// Shows the loading screen during level transitions.
	virtual void StartLoadingScreen(const FString& MapName);

	virtual void EndLoadingScreen(UWorld* InLoadedWorld);

private:
	void PreloadLoadingScreenImages();
	void OnLoadingScreenImageLoaded(FSoftObjectPath ImagePath);

private:
	FDelegateHandle PreLoadMapHandle;
	FDelegateHandle PostLoadMapHandle;
	FDelegateHandle PostEngineInitHandle;

	TStrongObjectPtr<UTexture2D> BackgroundTexture;

	TMap<FSoftObjectPath, UTexture2D*> PreloadedTextures;
	TArray<TStrongObjectPtr<UTexture2D>> StrongPreloadedTextures;
	TArray<TSharedPtr<FStreamableHandle>> PreloadHandles;
};
