#include "LoadingScreenModule.h"
#include "Engine/Texture2D.h"
#include "SLoadingScreen.h"
#include "MoviePlayer.h"
#include "LevelLoadingSettings.h"

#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

#include "Engine/World.h"


void FLoadingScreenModule::StartupModule()
{
    // Load When moduel initializes
    UE_LOG(LogTemp, Warning, TEXT("FLoadingScreenModule::StartupModule()"));

    // ================= Hard Coding Settings =============================== //
    // Load the background texture, which will be used for the loading screen.
    // Asset path is relative to the Content folder.
    //BackgroundTexture = LoadObject<UTexture2D>(nullptr, TEXT("Texture2D'/Game/LoadingScenes/T_BrokenCity.T_BrokenCity'"));

    // 
    // Try to get the loading screen settings
    //if (ULevelLoadingSettings* Settings = GetMutableDefault<ULevelLoadingSettings>())
    //{
    //    // Get the background image path from settings
    //    const FSoftObjectPath& BGPath = Settings->BackgroundImage;
    //    // If a background image is specified
    //    if (!BGPath.IsNull())
    //    {
    //        // Load the texture synchronously and store it
    //        // StaticLoadObject parameters : 
    //        // 1. Class to load (UTexture2D)
    //        // 2. Outer object (nullptr = default)
    //        // 3. Path to the asset.
    //        BackgroundTexture = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, *BGPath.ToString()));
    //    }
    //}

    /*
    *  매 Scene마다 다른 resource를 사용하는 LoadingScene을 사용하게 되어,
    * 실제 로딩할 맵 이름을 아는 StartLoadingScreen에서 Asset 선택하는 방식으로 변경.
    */
    PreLoadMapHandle = FCoreUObjectDelegates::PreLoadMap.AddRaw(
        this,
        &FLoadingScreenModule::StartLoadingScreen
    );

    PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddRaw(
        this,
        &FLoadingScreenModule::EndLoadingScreen
    );

    PostEngineInitHandle = FCoreDelegates::OnPostEngineInit.AddRaw(
        this,
        &FLoadingScreenModule::PreloadLoadingScreenImages
    );

}

void FLoadingScreenModule::ShutdownModule()
{
    if (PreLoadMapHandle.IsValid())
    {
        FCoreUObjectDelegates::PreLoadMap.Remove(PreLoadMapHandle);
        PreLoadMapHandle.Reset();
    }

    if (PostLoadMapHandle.IsValid())
    {
        FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
        PostLoadMapHandle.Reset();
    }

    if (PostEngineInitHandle.IsValid())
    {
        FCoreDelegates::OnPostEngineInit.Remove(PostEngineInitHandle);
        PostEngineInitHandle.Reset();
    }

    PreloadHandles.Empty();
    StrongPreloadedTextures.Empty();
    PreloadedTextures.Empty();
    BackgroundTexture.Reset();
}

void FLoadingScreenModule::StartLoadingScreen(const FString& MapName)
{
    // Log when loading screen is requested.
	UE_LOG(LogTemp, Warning, TEXT("FLoadingScreenModule::StartLoadingScreen()"));

    // ================= Hard Coding Settings =============================== //
	////// Create a struct to hold all our loading screen settings.
    //FLoadingScreenAttributes LoadingScreen;
    //
    //// Don't automatically remove the loading screen when loading completes
    //// this gives us manual control over when to hide it.
    //LoadingScreen.bAutoCompleteWhenLoadingCompletes = false;
    ////LoadingScreen.bWaitForManualStop = true;
    //
    //// ================ Method for showing sample widget ===================== //
    //// Set minimum time the loading screen will be displayed (in seconds).
    //// This prevents "flash" loading screens for quick loads.
    //LoadingScreen.MinimumLoadingScreenDisplayTime = 2.0f;
    //
    //// Create and assign a basic test widget for the loading screen.
    //// This is Unreal's built-in test widget, useful for prototyping
    //// => Black Screen, Loading dots.
    ////LoadingScreen.WidgetLoadingScreen = FLoadingScreenAttributes::NewTestLoadingScreenWidget();
    //
    //// ================ Method for playing videos ===================== //
    //// Prevent users from skipping the loading screen movie.
    ////LoadingScreen.bMoviesAreSkippable = false;
    //
    //// Add a path to a movie file that will play during loading
    //// The movie file should be placed in the project's Content/Movies Directory.
    ////LoadingScreen.MoviePaths.Add(TEXT("Gemini_Created_Video_V1"));
    //LoadingScreen.WidgetLoadingScreen = SNew(SLoadingScreen).BackgroundTexture(BackgroundTexture);
    // 
    //// Tell MoviePlayer to display out loading screen with these settings.
    //GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);

    // Get the loading screen settings
    ULevelLoadingSettings* Settings = GetMutableDefault<ULevelLoadingSettings>();
    if (!Settings)
    {
        return;
    }

    const FLevelLoadingScreenEntry* MatchedEntry = nullptr;

    // 현재 로딩하려는 Map 이름만 추출.
    const FString CurrentMapShortName = FPackageName::GetShortName(MapName);

    // 현재 Map에 대응하는 Loading 화면 설정을 순회하면서 찾음.
    for (const FLevelLoadingScreenEntry& Entry : Settings->LoadingScreens)
    {
        const FString EntryMapShortName = Entry.Map.GetAssetName();

        if (EntryMapShortName == CurrentMapShortName || Entry.Map.GetAssetPathString().Contains(CurrentMapShortName))
        {
            MatchedEntry = &Entry;
            break;
        }
    }

    // 이 Map에 대응하는 Loading 화면 설정이 없으면 그대로 넘어가기.
    if (!MatchedEntry)
    {
        return;
    }


    // Create a struct to hold all our loading screen settings.
    FLoadingScreenAttributes LoadingScreen;

    // Don't autumatically remove the loading screen whent loading completes
    LoadingScreen.bAutoCompleteWhenLoadingCompletes = true;

    // EndLoadingScene에서 수동 Stop되도록 한다.
    //LoadingScreen.bWaitForManualStop = true;

    
    // Set minimum time the loading screen will be displayed(in seconds).
    // This prevents "Flash" loading screens for quick loads.
    LoadingScreen.MinimumLoadingScreenDisplayTime = Settings->MinimumLoadingScreenDisplayTime;

    // Set the widget to use for the loading screen.

    // Movie를 넣었으면 Movie, Image를 넣었으면 Image.
    if (!MatchedEntry->MoviePath.IsEmpty())
    {
        LoadingScreen.MoviePaths.Add(MatchedEntry->MoviePath);
        LoadingScreen.bMoviesAreSkippable = false;
    }
    else if(!MatchedEntry->BackgroundImage.IsNull())
    {
        UTexture2D* SelectedTexture = PreloadedTextures.FindRef(MatchedEntry->BackgroundImage);

        if (!SelectedTexture && !Settings->DefaultBackgroundImage.IsNull())
        {
            SelectedTexture = PreloadedTextures.FindRef(Settings->DefaultBackgroundImage);
        }

        LoadingScreen.WidgetLoadingScreen =
            SNew(SLoadingScreen).BackgroundTexture(SelectedTexture);
    }


    // 위에서 넣은 Setting을 MoviePlayer의 LoadingScene으로 적용.
    GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
}

void FLoadingScreenModule::EndLoadingScreen(UWorld* InLoadedWorld)
{
    if (!IsValid(InLoadedWorld) || !InLoadedWorld)
    {
        UE_LOG(LogTemp, Warning, TEXT("FLoadingScreenModule::EndLoadingScreen: InLoadedWorld is not valid!"));
        return;
    }

    // 특정 Mesh들을 지정한 뒤, 전부 로드된 뒤에 Stop되도록 하고 싶으면 아래 코드와 함께 여러 줄을 추가해야 함.

    // Log when End loading screen is requested.
    //UE_LOG(LogTemp, Warning, TEXT("FLoadingScreenModule::EndLoadingScreen: %s"), *InLoadedWorld->GetName());
    //
    //UE_LOG(LogTemp, Warning, TEXT("Before StopMovie"));
    //GetMoviePlayer()->StopMovie();
    //UE_LOG(LogTemp, Warning, TEXT("After StopMovie"));

    //BackgroundTexture.Reset();
}

void FLoadingScreenModule::PreloadLoadingScreenImages()
{
    const ULevelLoadingSettings* Settings = GetDefault<ULevelLoadingSettings>();
    if (!Settings)
    {
        return;
    }

    FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();

    auto RequestPreload = [this, &StreamableManager](const FSoftObjectPath& ImagePath)
        {
            if (ImagePath.IsNull() || PreloadedTextures.Contains(ImagePath))
            {
                return;
            }

            TSharedPtr<FStreamableHandle> Handle = StreamableManager.RequestAsyncLoad(
                ImagePath,
                FStreamableDelegate::CreateRaw(this, &FLoadingScreenModule::OnLoadingScreenImageLoaded, ImagePath)
            );

            if (Handle.IsValid())
            {
                PreloadHandles.Add(Handle);
            }
        };

    RequestPreload(Settings->DefaultBackgroundImage);

    for (const FLevelLoadingScreenEntry& Entry : Settings->LoadingScreens)
    {
        RequestPreload(Entry.BackgroundImage);
    }
}

void FLoadingScreenModule::OnLoadingScreenImageLoaded(FSoftObjectPath ImagePath)
{
    UTexture2D* LoadedTexture = Cast<UTexture2D>(ImagePath.ResolveObject());
    if (!LoadedTexture)
    {
        LoadedTexture = Cast<UTexture2D>(ImagePath.TryLoad());
    }

    if (!LoadedTexture)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to preload loading screen image: %s"), *ImagePath.ToString());
        return;
    }

    PreloadedTextures.Add(ImagePath, LoadedTexture);
    StrongPreloadedTextures.Add(TStrongObjectPtr<UTexture2D>(LoadedTexture));
}


// Registers this module with Unreal Engine's module system.
IMPLEMENT_MODULE(FLoadingScreenModule, LoadingScreenModule);
