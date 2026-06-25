// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/MyNavExporter.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "Misc//FileHelper.h"
#include "Detour/DetourNavMesh.h"
#include "Engine/World.h"

UMyNavExporter::UMyNavExporter()
{
}

UMyNavExporter::~UMyNavExporter()
{
}

void UMyNavExporter::ExportNavMesh(UWorld* World)
{
	if (!World) return;

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	if (!NavSys) return;

	ARecastNavMesh* RecastNavData = Cast<ARecastNavMesh>(NavSys->GetMainNavData());
	if (!RecastNavData) return;

	const dtNavMesh* DetourMesh = RecastNavData->GetRecastMesh();
	if (!DetourMesh) return;

	TArray<uint8> BinaryData;
	FMemoryWriter Writer(BinaryData);

	// 헤더 정보 저장 (서버가 타일 개수 등을 미리 알 수 있도록)
	int32 MaxTiles = DetourMesh->getMaxTiles();
	Writer << MaxTiles;

	// dtNavMeshParams 저장 (서버 dtNavMesh 초기화에 필수)
	const dtNavMeshParams* Params = DetourMesh->getParams();
	// 임시 변수에 대입하여 L-value로 변환 후 직렬화
	float OrigX = static_cast<float>(Params->orig[0]);
	float OrigY = static_cast<float>(Params->orig[1]);
	float OrigZ = static_cast<float>(Params->orig[2]);
	Writer << OrigX;
	Writer << OrigY;
	Writer << OrigZ;

	float TileWidth = static_cast<float>(Params->tileWidth);
	float TileHeight = static_cast<float>(Params->tileHeight);
	Writer << TileWidth;
	Writer << TileHeight;

	int32 MaxParamsTiles = static_cast<int32>(Params->maxTiles);
	int32 MaxPolys = static_cast<int32>(Params->maxPolys);
	Writer << MaxParamsTiles;
	Writer << MaxPolys;



	// 각 타일의 바이너리 데이터 순회 및 저장
	int32 ZeroSize = 0;
	for (int i = 0; i < MaxTiles; ++i)
	{
		const dtMeshTile* Tile = DetourMesh->getTile(i);
		if (!Tile || !Tile->header || Tile->dataSize <= 0)
		{
			Writer << ZeroSize;
			continue;
		}
		int32 DataSize = static_cast<int32>(Tile->dataSize);
		Writer << DataSize;

		// 첫 번째 타일(index 0)의 매직넘버 강제 검증 로그 출력하여 기록 확인
		if (i == 0 && DataSize >= 4)
		{
			int32* RawMagic = (int32*)Tile->data;
			UE_LOG(LogTemp, Log, TEXT("Export Tile 0 Magic Data: 0x%X"), *RawMagic);
		}

		Writer.Serialize(const_cast<unsigned char*>(Tile->data), DataSize);
	}
	 FString SavePath = FPaths::ProjectDir()/TEXT("Common/Nav/NavMesh.nav");
		// 외부 파일로 저장 
		 bool bSuccess = FFileHelper::SaveArrayToFile(BinaryData, *SavePath);

	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("NavMesh Binary Export Success: %s"), *SavePath);
	}
}
