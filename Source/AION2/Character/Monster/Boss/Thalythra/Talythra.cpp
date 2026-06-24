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
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "AI/AITalythraAIController.h"
#include "GAS/AOGameplayTags.h"


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


	static ConstructorHelpers::FObjectFinder<UAnimMontage> ChargeAttackMontageRef(TEXT("/Game/Blueprint/Monster/Boss/Talythra/Montage/AM_Thalythra_ChargeAttack.AM_Thalythra_ChargeAttack"));
	if(ChargeAttackMontageRef.Object != NULL)
	{
		ChargeAttackMontage = ChargeAttackMontageRef.Object;
	}
	


	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	ASC->SetIsReplicated(true);
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);


	
	bReplicates = true; 
	SetReplicateMovement(true);


}

void ATalythra::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	// Owner Actor와 AvatarActor 설정 
	ASC->InitAbilityActorInfo(this, this);


	if (!ASC->HasMatchingGameplayTag(TEAM_MONSTER))
	{
		ASC->AddLooseGameplayTag(TEAM_MONSTER);
	}

	// Owner Actor란? : ASC를 논리적으로 소유한 주체 ( 해당 주체가 죽어도 유지되며, 레벨 및 스텟 보존 )
	// Avatar Actor란? : Character  (죽으면 바뀌는 물체, 실제로 데이터를 처리하지 않지만 비주얼만 수행해주는 엑터) 


}

// Called when the game starts or when spawned
void ATalythra::BeginPlay()
{
	Super::BeginPlay();
	
	
	if(HasAuthority()) // 서버인 경우 
	{
		for( const auto& Ability : HasAbilities) // 능력들 저장. 
		{
			FGameplayAbilitySpec AbilitySpec(Ability.Value);
			ASC->GiveAbility(AbilitySpec);
		}
	}

	

#pragma region Attack_Line SceneComponents

	TArray<USceneComponent*> SceneComponents;
	GetComponents<USceneComponent>(SceneComponents);

	for (USceneComponent* SceneComp : SceneComponents)
	{
		if (!IsValid(SceneComp))
		{
			continue;
		}

		const FString NameString = SceneComp->GetName();


		if (NameString.Contains(TEXT("ProjectileLine")))
		{
			ArrayProjectileLineSceneComponent.Add(SceneComp);
			SceneComp->SetVisibility(false, true);
			// 앞의 false는 root 부모 설정 , 뒤의 true는 부모 설정을 따라갈지를 선택
			// ex) root -> false 이므로 자식도 false로 설정하고 싶으면 뒤를 true로 설정.

		}


		else if (NameString.Contains(TEXT("AoeIndicator")))
		{
			AttackRangeSceneComponent = SceneComp;
			SceneComp->SetVisibility(false, true);
		}


		else if (NameString.Contains(TEXT("AOE Warning Circle")))
		{
			AttackWarningRangeSceneComponent = SceneComp; 
			SceneComp->SetVisibility(false, true);
		}


	}

#pragma endregion 

}

// Called every frame
void ATalythra::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	if (HasAuthority() == false)
		return;

	if (bChargeAttack)
	{
		AddMovementInput(ChargeDirection, 1.0f, false);
	}


	if (AttackWarningRangeSceneComponent->GetVisibleFlag() == true)
	{

		AttackWarningElapsedTime += DeltaTime;

		const float Alpha = FMath::Clamp(
			AttackWarningElapsedTime / AttackWarningDuration,
			0.0f,
			1.0f
		);
		
		const float CurrentScale = FMath::Lerp(
			0.01f,
			AttackWarningTargetScale,
			Alpha
		);


		AttackWarningRangeSceneComponent->SetRelativeScale3D(FVector(CurrentScale, CurrentScale, 1.f));


		if(AttackWarningElapsedTime > AttackWarningDuration)
		{
			// 여기서 다시 재생시키면 될듯 
			AttackWarningRangeSceneComponent->SetVisibility(false, true);
		}
	}
	

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

	if (AttackLineRenderOnOff)
	{
		switch (FireCount)
		{
		case 1:
		{
			Multicast_AttackLine_Pattern_1();
			AttackLineRenderOnOff = false;
		}
		break;
		case 2:
		{
			Multicast_AttackLine_Pattern_2();
			AttackLineRenderOnOff = false;
		}
		break;
		case 3:
		{
			Multicast_AttackLine_Pattern_3();
			AttackLineRenderOnOff = false;
		}
		break;
		default:
			break;
		}
	}



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

		RotationAble = false; 

		return;
	}



	if (RotationAble == false)
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



	
	
	
}


void ATalythra::StartChargeMove()
{

	if (HasAuthority() == false)
		return;

	ChargeDirection = GetActorForwardVector();
	ChargeDirection.Z = 0.f;
	ChargeDirection = ChargeDirection.GetSafeNormal();

	bChargeAttack = true;

	Multicast_SetChargeMovementParams(true);
}

void ATalythra::EndChargeMove()
{
	if(HasAuthority() == false)
		return;

	bChargeAttack = false;

	Multicast_SetChargeMovementParams(false);
}

void ATalythra::Teleport_To_Player()
{

	if (HasAuthority() == false)
		return; 


	AAITalythraAIController* pAITalythraAIController = Cast<AAITalythraAIController>(GetController());
	
	AActor* pTargetActor = pAITalythraAIController->Get_CurrentTargetPlayer(); 
	

	// replicated는 위에 생성자에서 해줬으므로 mulit rpc 필요 x 
	TeleportTo(pTargetActor->GetActorLocation(),GetActorRotation(),false,false);


}

void ATalythra::Attack_RangeRender(bool _bRenderOnOff)
{
	if (HasAuthority() == false)
		return; 

	if(_bRenderOnOff == true)
	{
		AttackRangeSceneComponent->SetRelativeScale3D(FVector(AttackAoeScale, AttackAoeScale, 1.f));
		AttackWarningRangeSceneComponent->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
		
	}

	Multicast_AttackRangeRender(_bRenderOnOff);


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
	DOREPLIFETIME(ATalythra, bLockPelvis);

}


void ATalythra::Multicast_AttackRangeRender_Implementation(bool _bRendrOnOff)
{
	AttackRangeSceneComponent->SetVisibility(_bRendrOnOff, true);
	AttackWarningRangeSceneComponent->SetVisibility(_bRendrOnOff, true);

	if (_bRendrOnOff == false)
	{
		AttackWarningRangeSceneComponent->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
		AttackWarningElapsedTime = 0.0f;
	}
}

void ATalythra::Multicast_AttackLine_Pattern_1_Off_Implementation()
{
	ArrayProjectileLineSceneComponent[0]->SetVisibility(false, true);
}

void ATalythra::Multicast_AttackLine_Pattern_2_Off_Implementation()
{
	ArrayProjectileLineSceneComponent[0]->SetVisibility(false, true);
	ArrayProjectileLineSceneComponent[1]->SetVisibility(false, true);
}

void ATalythra::Multicast_AttackLine_Pattern_3_Off_Implementation()
{
	ArrayProjectileLineSceneComponent[0]->SetVisibility(false, true);
	ArrayProjectileLineSceneComponent[1]->SetVisibility(false, true);
	ArrayProjectileLineSceneComponent[2]->SetVisibility(false, true);
}

void ATalythra::Multicast_AttackLine_Pattern_1_Implementation()
{
	ArrayProjectileLineSceneComponent[0]->SetVisibility(true, true);
}

void ATalythra::Multicast_AttackLine_Pattern_2_Implementation()
{
	ArrayProjectileLineSceneComponent[0]->SetVisibility(true, true);
	ArrayProjectileLineSceneComponent[1]->SetVisibility(true, true);


	const FName SocketName = TEXT("WP_Center");

	const FTransform SocketTransform =
		GetMesh()->GetSocketTransform(SocketName, RTS_World);

	const FVector SpawnLocation = SocketTransform.GetLocation();

	// Z축의 반대 방향으로 발사
	const FVector BaseDirection =
		-SocketTransform.GetUnitAxis(EAxis::Y).GetSafeNormal();

	FRotator AttackLineRotation;

	AttackLineRotation = BaseDirection.RotateAngleAxis(10.0f, FVector::UpVector).Rotation();
	ArrayProjectileLineSceneComponent[0]->SetWorldRotation(AttackLineRotation);


	AttackLineRotation = BaseDirection.RotateAngleAxis(-10.0f, FVector::UpVector).Rotation();
	ArrayProjectileLineSceneComponent[1]->SetWorldRotation(AttackLineRotation);

}

void ATalythra::Multicast_AttackLine_Pattern_3_Implementation()
{
	ArrayProjectileLineSceneComponent[0]->SetVisibility(true, true);
	ArrayProjectileLineSceneComponent[1]->SetVisibility(true, true);
	ArrayProjectileLineSceneComponent[2]->SetVisibility(true, true);


	const FName SocketName = TEXT("WP_Center");

	const FTransform SocketTransform =
		GetMesh()->GetSocketTransform(SocketName, RTS_World);

	const FVector SpawnLocation = SocketTransform.GetLocation();

	// Z축의 반대 방향으로 발사
	const FVector BaseDirection =
		-SocketTransform.GetUnitAxis(EAxis::Y).GetSafeNormal();

	FRotator AttackLineRotation;

	AttackLineRotation = BaseDirection.RotateAngleAxis(0.0f, FVector::UpVector).Rotation();
	ArrayProjectileLineSceneComponent[0]->SetWorldRotation(AttackLineRotation);

	AttackLineRotation = BaseDirection.RotateAngleAxis(-15.0f, FVector::UpVector).Rotation();
	ArrayProjectileLineSceneComponent[1]->SetWorldRotation(AttackLineRotation);

	AttackLineRotation = BaseDirection.RotateAngleAxis(15.0f, FVector::UpVector).Rotation();
	ArrayProjectileLineSceneComponent[2]->SetWorldRotation(AttackLineRotation);
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

void ATalythra::Multicast_SetChargeMovementParams_Implementation(bool bChargeMode)
{

	UCharacterMovementComponent* CMC = GetCharacterMovement();
	if (!CMC)
		return;

	if (bChargeMode)
	{
		CMC->MaxWalkSpeed = 6000.f;
		CMC->MaxAcceleration = 60000.f;
		CMC->GroundFriction = 0.f;
		CMC->BrakingDecelerationWalking = 0.f;
		CMC->BrakingFrictionFactor = 0.f;
	}
	else
	{
		CMC->MaxWalkSpeed = 600.f;
		CMC->MaxAcceleration = 2048.f;
		CMC->GroundFriction = 8.f;
		CMC->BrakingDecelerationWalking = 2048.f;
		CMC->BrakingFrictionFactor = 2.f;
	}
}




//const float ChargeSpeed = 2600.f;

	//GetCharacterMovement()->Velocity = ChargeDirection * 1800.f;

		/*FHitResult Hit;

	AddActorWorldOffset(
		ChargeDirection * ChargeSpeed * DeltaTime,
		true,
		&Hit
	);*/
