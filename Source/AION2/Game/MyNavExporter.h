// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MyNavExporter.generated.h"
/**
 * 
 */
UCLASS()
class AION2_API UMyNavExporter : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UMyNavExporter();
	~UMyNavExporter();

	UFUNCTION(BlueprintCallable)
	static void ExportNavMesh(UWorld* World);
};
