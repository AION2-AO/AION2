#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

class UWorld;

class ILoadingScreenModule : public IModuleInterface
{
public:
    static ILoadingScreenModule& Get()
    {
        return FModuleManager::LoadModuleChecked<ILoadingScreenModule>("LoadingScreenModule");
    }

    static bool IsAvailable()
    {
        return FModuleManager::Get().IsModuleLoaded("LoadingScreenModule");
    }

    virtual void StartLoadingScreen(const FString& MapName) = 0;
    virtual void EndLoadingScreen(UWorld* LoadedWorld) = 0;
};