// Fill out your copyright notice in the Description page of Project Settings.


#include "Talythra.h"
#include "Components/StateTreeAIComponent.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimMontage.h"
#include "Character/Monster/Boss/Thalythra/Projectile/TalythraProjectile.h"
#include "NiagaraFunctionLibrary.h"
#include "AI/AITalythraAIController.h"
#include "Components/DecalComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


// Sets default values
ATalythra::ATalythra(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	// Skeletal Mesh
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> CharacterSkeletalMesh(TEXT("/Game/Characters/Monster/Boss/Thalythra/Thalythra.Thalythra"));

	if (CharacterSkeletalMesh.Object != NULL)
	{
		GetMesh()->SetSkeletalMesh(CharacterSkeletalMesh.Object);
	}

	static ConstructorHelpers::FClassFinder<ATalythraProjectile> ProjectileClassRef(
		TEXT("/Game/Blueprint/Monster/Boss/Talythra/Projectile/BP_TalyhraProjectile.BP_TalyhraProjectile_C")
	);

	if (ProjectileClassRef.Succeeded())
	{
		ProjectileClass = ProjectileClassRef.Class;
	}


	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> LanuchMazzleEffectRef(TEXT("/Game/Asset/MagicProjectiles/Niagara/Poison_Glob/PG_Ver3/NS_PoisonGlob_Muzzle_03.NS_PoisonGlob_Muzzle_03"));

	if(LanuchMazzleEffectRef.Object != NULL)
	{
		LanchMuzzleEffect = LanuchMazzleEffectRef.Object;
	}


	bReplicates = true; 
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void ATalythra::BeginPlay()
{
	Super::BeginPlay();
	

#pragma region Decal_Line SceneComponents

	TArray<USceneComponent*> SceneComponents;
	GetComponents<USceneComponent>(SceneComponents);

	for (USceneComponent* SceneComp : SceneComponents)
	{
		if (!IsValid(SceneComp))
		{
			continue;
		}

		const FString NameString = SceneComp->GetName();


		if (NameString.Contains(TEXT("DeadAttackLine")))
		{
			ArrayDecalSceneComponent.Add(SceneComp);
			SceneComp->SetVisibility(false, true);

		}
	}

#pragma endregion 


	if (HasAuthority() == false) return;

	// 현재 설정값 로그
	UE_LOG(LogTemp, Warning, TEXT("VisibilityBasedAnimTickOption: %d"),
		(int32)GetMesh()->VisibilityBasedAnimTickOption);
	UE_LOG(LogTemp, Warning, TEXT("AnimScriptInstance: %s"),
		GetMesh()->GetAnimInstance() ? TEXT("Valid") : TEXT("Null"));
	UE_LOG(LogTemp, Warning, TEXT("bRecentlyRendered: %d"),
		GetMesh()->bRecentlyRendered);


}

// Called every frame
void ATalythra::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


}

void ATalythra::Multicast_PlayAttackMontage_Implementation(UAnimMontage* MontageToPlay)
{

	PlayAnimMontage(MontageToPlay);


}


void ATalythra::FireProjectile()
{


	//GetMesh()->RefreshBoneTransforms();

	//---
	// 서버에서만 발사체 생성
	if(HasAuthority() == false)
	{
		return;
	}


	switch (FireCount)
	{
	case 1:
	{
		// 노티파이는 애님 평가 중 → 다음 틱으로 미뤄서 안전하게 처리
		GetWorld()->GetTimerManager().SetTimerForNextTick(
			FTimerDelegate::CreateUObject(this, &ATalythra::DoFireProjectile)
		);
		
		FireCount += 1;
	}
		break;
	case 2:
	{
		// 노티파이는 애님 평가 중 → 다음 틱으로 미뤄서 안전하게 처리
		GetWorld()->GetTimerManager().SetTimerForNextTick(
			FTimerDelegate::CreateUObject(this, &ATalythra::DoFireProjectile_2)
		);

		FireCount += 1; 
	}
		break;

	case 3:
	{
		// 노티파이는 애님 평가 중 → 다음 틱으로 미뤄서 안전하게 처리
		GetWorld()->GetTimerManager().SetTimerForNextTick(
			FTimerDelegate::CreateUObject(this, &ATalythra::DoFireProjectile_3)
		);

		FireCount = 1; 
	}
		break;
	}



}

void ATalythra::TurnToTarget()
{
	AAITalythraAIController* TalythraAIController = Cast <AAITalythraAIController>(GetController());
	if (TalythraAIController == nullptr)
		return;

	AActor* pPlayer = TalythraAIController->Get_CurrentTargetPlayer();
	if (pPlayer == nullptr)
		return; 



	// 몬스터에서 플레이어로 향하는 벡터
	FVector vDirToPlayer = pPlayer->GetActorLocation() - GetActorLocation();
	vDirToPlayer.Z = 0.f;
	FVector vDirNoramlToPlayer = vDirToPlayer.GetSafeNormal();

	// 몬스터의 방향벡터 , 몬스터가 플레이어를 바라보는 방향벡터를 내적하여 사이 각 구하기.
	FVector vDirMonsterFowradVector = GetActorForwardVector();
	vDirMonsterFowradVector.Z = 0.f;
	vDirMonsterFowradVector = vDirMonsterFowradVector.GetSafeNormal();

	float Dot = FVector::DotProduct(vDirMonsterFowradVector, vDirNoramlToPlayer);
	Dot = FMath::Clamp(Dot, -1.f, 1.f); // float 오차 연산 1.0001이 나올 수 있을 수 도 있으니깐!



	// 두 벡터를 외적하여 회전 방향 구하기 
	
	// 라디안 각도
	const float Radian = FMath::Acos(Dot);

	// 도 단위 각도
	const float Degree = FMath::RadiansToDegrees(Radian);
	
	FVector Cross = FVector::CrossProduct(vDirMonsterFowradVector, vDirNoramlToPlayer);

	
	if(FMath::Abs(Degree) < 2.0f)
	{
		// 여기서 회전 끄기 미세한 떨림 여기서 발생 bRotation 을 추가해서 그만 회전하게 하자!

		bRotationAble = false; 

		return;
	}



	if (bRotationAble == false)
		return; 

	
	const float DeltaTime = GetWorld()->GetDeltaSeconds();
	const float TurnSpeed = GetCharacterMovement()->RotationRate.Yaw;


	// 이번 프레임에 회전할 각도 
	const float StepDegree = FMath::Min(abs(Degree), DeltaTime * TurnSpeed);


	if(Cross.Z > 0.f)
	{
		// 시계 방향 → 오른쪽 회전
		// 여기서 현재 포워트 벡터 기준으로 회전!
		AddActorWorldRotation(FRotator(0.0f, StepDegree, 0.f));
	}

	else
	{
		// 반시계 방향 → 왼쪽 회전
		// 여기서 현재 포워트 벡터 기준으로 회전!
		AddActorWorldRotation(FRotator(0.0f, StepDegree * -1.f, 0.f));
	}

	//if (vDirToPlayer.IsNearlyZero())
	//{
	//	return;
	//}


	////월드 기준 방향 벡터를 보고, 그 방향을 바라보는 “절대 회전값”을 만들어주는 함수야.
	//const FRotator TargetRotation = vDirToPlayer.Rotation();

	//if (TargetRotation.Yaw <= KINDA_SMALL_NUMBER)
	//	return; 


	//
	//SetActorRotation(FRotator(
	//	0.f,
	//	TargetRotation.Yaw,
	//	0.f
	//));


	//switch (FireCount)
	//{
	//case 1:
	//{
	//	Multicast_AttackLine_Pattern_1();
	//}
	//	break;
	//case 2:
	//{
	//	Multicast_AttackLine_Pattern_2();
	//}
	//	break;
	//case 3:
	//{
	//	Multicast_AttackLine_Pattern_3();
	//}
	//	break;
	//default:
	//	break;
	//}
	
}

void ATalythra::DoFireProjectile()
{
	USkeletalMeshComponent* CharacterMesh = GetMesh();
	if (CharacterMesh == nullptr || ProjectileClass == nullptr)
	{
		return;
	}


	GetMesh()->RefreshBoneTransforms();

	const FName SocketName = TEXT("WP_Center");

	const FTransform SocketTransform =
		GetMesh()->GetSocketTransform(SocketName, RTS_World);

	const FVector SpawnLocation = SocketTransform.GetLocation();

	// Z축의 반대 방향으로 발사
	const FVector BaseDirection =
		-SocketTransform.GetUnitAxis(EAxis::Y).GetSafeNormal();

	const FRotator SpawnRotation = BaseDirection.Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ATalythraProjectile* Projectile = GetWorld()->SpawnActor<ATalythraProjectile>(
		ProjectileClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	if (Projectile == nullptr)
	{
		return; 
    }


	Projectile->InitVelocityAndDirection(BaseDirection);

	/* 발사 이펙트*/
	Multicast_PlayMuzzleEffect(SpawnLocation, SpawnRotation);

	/* Decal 끄기 */
	Multicast_AttackLine_Pattern_1_Off();


}

void ATalythra::DoFireProjectile_2()
{

	USkeletalMeshComponent* CharacterMesh = GetMesh();
	if (CharacterMesh == nullptr || ProjectileClass == nullptr)
	{
		return;
	}


	GetMesh()->RefreshBoneTransforms();

	const FName SocketName = TEXT("WP_Center");

	const FTransform SocketTransform =
		GetMesh()->GetSocketTransform(SocketName, RTS_World);

	const FVector SpawnLocation = SocketTransform.GetLocation();

	// Z축의 반대 방향으로 발사
	const FVector BaseDirection =
		-SocketTransform.GetUnitAxis(EAxis::Y).GetSafeNormal();



	const FRotator SpawnRotation = BaseDirection.Rotation();


	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ATalythraProjectile* Projectile_1 = GetWorld()->SpawnActor<ATalythraProjectile>(
		ProjectileClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);


	ATalythraProjectile* Projectile_2 = GetWorld()->SpawnActor<ATalythraProjectile>(
		ProjectileClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);


	


	if (Projectile_1)
	{
		const FVector FireDirection_1 = BaseDirection.RotateAngleAxis(10.f, FVector::UpVector).GetSafeNormal();

		Projectile_1->InitVelocityAndDirection(FireDirection_1);
	}



	if (Projectile_2)
	{
		const FVector FireDirection_2 = BaseDirection.RotateAngleAxis(-10.f, FVector::UpVector).GetSafeNormal();

		Projectile_2->InitVelocityAndDirection(FireDirection_2);
	}


	/* 발사 이펙트 */
	Multicast_PlayMuzzleEffect(SpawnLocation, SpawnRotation);

	/* Decal 끄기 */
	Multicast_AttackLine_Pattern_2_Off();
}

void ATalythra::DoFireProjectile_3()
{


	USkeletalMeshComponent* CharacterMesh = GetMesh();
	if (CharacterMesh == nullptr || ProjectileClass == nullptr)
	{
		return;
	}


	GetMesh()->RefreshBoneTransforms();

	const FName SocketName = TEXT("WP_Center");

	const FTransform SocketTransform =
		GetMesh()->GetSocketTransform(SocketName, RTS_World);

	const FVector SpawnLocation = SocketTransform.GetLocation();

	// Z축의 반대 방향으로 발사
	const FVector BaseDirection =
		-SocketTransform.GetUnitAxis(EAxis::Y).GetSafeNormal();



	const FRotator SpawnRotation = BaseDirection.Rotation();


	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ATalythraProjectile* Projectile_1 = GetWorld()->SpawnActor<ATalythraProjectile>(
		ProjectileClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);


	ATalythraProjectile* Projectile_2 = GetWorld()->SpawnActor<ATalythraProjectile>(
		ProjectileClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	ATalythraProjectile* Projectile_3 = GetWorld()->SpawnActor<ATalythraProjectile>(
		ProjectileClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);




	if (Projectile_1)
	{
		const FVector FireDirection_1 = BaseDirection;

		Projectile_1->InitVelocityAndDirection(FireDirection_1);
	}




	if (Projectile_2)
	{
		const FVector FireDirection_2 = BaseDirection.RotateAngleAxis(15.f, FVector::UpVector).GetSafeNormal();

		Projectile_2->InitVelocityAndDirection(FireDirection_2);
	}



	if (Projectile_3)
	{
		const FVector FireDirection_3 = BaseDirection.RotateAngleAxis(-15.f, FVector::UpVector).GetSafeNormal();

		Projectile_3->InitVelocityAndDirection(FireDirection_3);
	}

	/* 발사 이펙트 */
	Multicast_PlayMuzzleEffect(SpawnLocation, SpawnRotation);

	/* Decal 끄기 */
	Multicast_AttackLine_Pattern_3_Off();

}

void ATalythra::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ATalythra, Phase);
	DOREPLIFETIME(ATalythra, State);

}


void ATalythra::Multicast_AttackLine_Pattern_1_Off_Implementation()
{
	ArrayDecalSceneComponent[0]->SetVisibility(false, true);
}

void ATalythra::Multicast_AttackLine_Pattern_2_Off_Implementation()
{
	ArrayDecalSceneComponent[0]->SetVisibility(false, true);
	ArrayDecalSceneComponent[1]->SetVisibility(false, true);
}

void ATalythra::Multicast_AttackLine_Pattern_3_Off_Implementation()
{
	ArrayDecalSceneComponent[0]->SetVisibility(false, true);
	ArrayDecalSceneComponent[1]->SetVisibility(false, true);
	ArrayDecalSceneComponent[2]->SetVisibility(false, true);
}

void ATalythra::Multicast_AttackLine_Pattern_1_Implementation()
{
	ArrayDecalSceneComponent[0]->SetVisibility(true, true);
}

void ATalythra::Multicast_AttackLine_Pattern_2_Implementation()
{
	ArrayDecalSceneComponent[0]->SetVisibility(true, true);
	ArrayDecalSceneComponent[1]->SetVisibility(true, true);


	const FName SocketName = TEXT("WP_Center");

	const FTransform SocketTransform =
		GetMesh()->GetSocketTransform(SocketName, RTS_World);

	const FVector SpawnLocation = SocketTransform.GetLocation();

	// Z축의 반대 방향으로 발사
	const FVector BaseDirection =
		-SocketTransform.GetUnitAxis(EAxis::Y).GetSafeNormal();

	FRotator DecalRotation;

	DecalRotation = BaseDirection.RotateAngleAxis(10.0f, FVector::UpVector).Rotation();
	ArrayDecalSceneComponent[0]->SetWorldRotation(DecalRotation);


	DecalRotation = BaseDirection.RotateAngleAxis(-10.0f, FVector::UpVector).Rotation();
	ArrayDecalSceneComponent[1]->SetWorldRotation(DecalRotation);

}

void ATalythra::Multicast_AttackLine_Pattern_3_Implementation()
{
	ArrayDecalSceneComponent[0]->SetVisibility(true, true);
	ArrayDecalSceneComponent[1]->SetVisibility(true, true);
	ArrayDecalSceneComponent[2]->SetVisibility(true, true);


	const FName SocketName = TEXT("WP_Center");

	const FTransform SocketTransform =
		GetMesh()->GetSocketTransform(SocketName, RTS_World);

	const FVector SpawnLocation = SocketTransform.GetLocation();

	// Z축의 반대 방향으로 발사
	const FVector BaseDirection =
		-SocketTransform.GetUnitAxis(EAxis::Y).GetSafeNormal();

	FRotator DecalRotation;

	DecalRotation = BaseDirection.RotateAngleAxis(0.0f, FVector::UpVector).Rotation();
	ArrayDecalSceneComponent[0]->SetWorldRotation(DecalRotation);

	DecalRotation = BaseDirection.RotateAngleAxis(-15.0f, FVector::UpVector).Rotation();
	ArrayDecalSceneComponent[1]->SetWorldRotation(DecalRotation);

	DecalRotation = BaseDirection.RotateAngleAxis(15.0f, FVector::UpVector).Rotation();
	ArrayDecalSceneComponent[2]->SetWorldRotation(DecalRotation);
}




void ATalythra::Multicast_PlayMuzzleEffect_Implementation(FVector SpawnLocation, FRotator SpawnRotation)
{
	if(LanchMuzzleEffect == nullptr)
	{
		return;
	}

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		LanchMuzzleEffect,
		SpawnLocation,
		SpawnRotation,
		FVector(4.5f, 4.5f, 4.5f)  // Scale
	); 

}






// AnimInstance 

//static ConstructorHelpers::FClassFinder<UAnimInstance> AnimInstanceRef(TEXT("/Game/Blueprint/Monster/Boss/Talythra/ABP_Talythra.ABP_Talythra_C"));
////
//if(AnimInstanceRef.Class != NULL)
//{
//	GetMesh()->SetAnimInstanceClass(AnimInstanceRef.Class);
//}


//GetMesh()->VisibilityBasedAnimTickOption =
//	EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

/*GetMesh()->VisibilityBasedAnimTickOption =
	EVisibilityBasedAnimTickOption::AlwaysTickPose;*/