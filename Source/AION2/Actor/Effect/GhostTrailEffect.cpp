#include "Actor/Effect/GhostTrailEffect.h"

#include "Components/PoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

AGhostTrailEffect::AGhostTrailEffect()
{
    PrimaryActorTick.bCanEverTick = true;
    SetActorEnableCollision(false);

    USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);
}

void AGhostTrailEffect::Initialize(USkeletalMeshComponent* RootMesh, const TArray<USkeletalMeshComponent*>& SourceMeshes, UMaterialInterface* InGhostMaterial, float InLifeTime)
{
    if (!RootMesh || SourceMeshes.IsEmpty())
    {
        Destroy();
        return;
    }

    LifeTime = InLifeTime;
    ElapsedTime = 0.f;

    GhostMeshes.Empty();
    DynamicMaterials.Empty();

    for (USkeletalMeshComponent* SourceMesh : SourceMeshes)
    {
        if (!SourceMesh || !SourceMesh->GetSkeletalMeshAsset() || !SourceMesh->IsVisible())
        {
            continue;
        }

        UPoseableMeshComponent* GhostMesh =
            NewObject<UPoseableMeshComponent>(this);

        GhostMesh->RegisterComponent();
        GhostMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);

        GhostMesh->SetSkeletalMesh(SourceMesh->GetSkeletalMeshAsset());
        GhostMesh->SetWorldTransform(SourceMesh->GetComponentTransform());
        GhostMesh->CopyPoseFromSkeletalComponent(RootMesh);
        GhostMesh->SetVisibleInRayTracing(false);
        GhostMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

        const int32 MaterialCount = SourceMesh->GetNumMaterials();
        for (int32 i = 0; i < MaterialCount; ++i)
        {
            UMaterialInterface* BaseMaterial = InGhostMaterial ? InGhostMaterial : SourceMesh->GetMaterial(i);

            UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMaterial, this);

            GhostMesh->SetMaterial(i, MID);
            DynamicMaterials.Add(MID);
        }

        GhostMeshes.Add(GhostMesh);
    }

    if (GhostMeshes.IsEmpty())
    {
        Destroy();
    }
}

void AGhostTrailEffect::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    ElapsedTime += DeltaTime;

    const float Alpha = FMath::Clamp(1.f - ElapsedTime / LifeTime, 0.f, 1.f);

    for (UMaterialInstanceDynamic* MID : DynamicMaterials)
    {
        if (MID)
        {
            MID->SetScalarParameterValue(TEXT("Opacity"), Alpha);
        }
    }

    if (ElapsedTime >= LifeTime)
    {
        Destroy();
    }
}
