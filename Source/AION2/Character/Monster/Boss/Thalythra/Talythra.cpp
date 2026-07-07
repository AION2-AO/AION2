// Fill out your copyright notice in the Description page of Project Settings.


#include "Talythra.h"
#include "Components/StateTreeAIComponent.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimMontage.h"
#include "Character/Monster/Boss/Thalythra/Projectile/TalythraProjectile.h"
#include "NiagaraFunctionLibrary.h"
#include "AI/Talythra/AITalythraAIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/AttributeSet/AOAttributeSet.h"

#include "UI/AOWidgetComponentBase.h"
#include "UI/AOMonsterHUDWidget.h"

#include "Character/Monster/Boss/Thalythra/Wave/WaveCircle.h"
#include "Components/CapsuleComponent.h"

#include "Character/Monster/Boss/Thalythra/Projectile/TalythraCollectibleOrb.h"
#include "GAS/AOGameplayTags.h"
#include "Character/Daeva/Daeva.h"
#include "Character/Monster/Boss/Thalythra/Shield/TalythraGimmickShield.h"


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

	if (LanuchMazzleEffectRef.Object != NULL)
	{
		LanchMuzzleEffect = LanuchMazzleEffectRef.Object;
	}


	static ConstructorHelpers::FObjectFinder<UAnimMontage> ChargeAttackMontageRef(TEXT("/Game/Blueprint/Monster/Boss/Talythra/Montage/AM_Thalythra_ChargeAttack.AM_Thalythra_ChargeAttack"));
	if (ChargeAttackMontageRef.Object != NULL)
	{
		ChargeAttackMontage = ChargeAttackMontageRef.Object;
	}

	// UI
	DungeonBossIndex = 3;

	OverheadStatusWidgetComponent->SetRelativeScale3D(FVector(10.0f, 10.0f, 10.0f));
	OverheadStatusWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 350.0f));
	OverheadStatusWidgetComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 90.0f));


}

void ATalythra::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

// Called when the game starts or when spawned
void ATalythra::BeginPlay()
{
	Super::BeginPlay();

	InitAttributeSet();


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

#pragma region 
	AttackWarningRangeSceneComponent->SetIsReplicated(true);
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
		if (CanMoveOnNavMesh(ChargeDirection, 750.f)) // 이 부분도 수정 
		{
			AddMovementInput(ChargeDirection, 1.0f, false);
		}

		else
		{
			GetCharacterMovement()->StopMovementImmediately();
			EndChargeMove();
		}

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


		if (AttackWarningElapsedTime > AttackWarningDuration)
		{
			// 여기서 다시 재생시키면 될듯 
			AttackWarningRangeSceneComponent->SetVisibility(false, true);
		}

	}

	if (RotationAble)
	{
		TurnToTarget();
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
	if (HasAuthority() == false)
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


	// 플레이어를 향한 방향
	FVector DirToPlayer = pPlayer->GetActorLocation() - GetActorLocation();
	DirToPlayer.Z = 0.f;

	if (DirToPlayer.IsNearlyZero())
		return;

	const FRotator DesiredRot(0.f, DirToPlayer.Rotation().Yaw, 0.f);

	// 각도 차 계산 (도착 판정용)
	const float DeltaYaw = FMath::Abs(
		FRotator::NormalizeAxis(DesiredRot.Yaw - GetActorRotation().Yaw)
	);

	if (DeltaYaw < 2.0f)
	{
		RotationAble = false;
		return;
	}

	if (RotationAble == false)
		return;

	// 목표 회전만 세팅 → 실제 회전은 CMC가 RotationRate에 맞춰 스무스하게 처리
	TalythraAIController->SetControlRotation(DesiredRot);

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
	if (HasAuthority() == false)
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
	TeleportTo(pTargetActor->GetActorLocation(),
		GetActorRotation(),
		false, // true면 실제 이동 없이 가능 여부만 테스트
		true); // true면 충돌 무시하고 강제 이동


}

void ATalythra::Teleport_To_Center()
{

	if (HasAuthority() == false)
		return;


	AAITalythraAIController* pAITalythraAIController = Cast<AAITalythraAIController>(GetController());


	// replicated는 위에 생성자에서 해줬으므로 mulit rpc 필요 x 
	TeleportTo(TeleportCenterLocation,
		GetActorRotation(),
		false, // true면 실제 이동 없이 가능 여부만 테스트
		true); // true면 충돌 무시하고 강제 이동


}

void ATalythra::Attack_RangeRender(bool _bRenderOnOff)
{
	if (HasAuthority() == false)
		return;

	if (_bRenderOnOff == true)
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

void ATalythra::InitAttributeSet()
{
	// AttributeSet설정
	AttributeSet->InitHealth(5000.f);
	AttributeSet->InitMaxHealth(5000.f);

	AttributeSet->InitStamina(100.f);
	AttributeSet->InitMaxStamina(100.f);




}



void ATalythra::SpawnWaveBlue()
{

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FVector SpawnLocation = GetActorLocation();
	SpawnLocation.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	SpawnLocation.Z += 100.f;

	AWaveCircle* WaveCircle1 = GetWorld()->SpawnActor<AWaveCircle>(
		WaveCircleBlueClass,
		SpawnLocation,
		GetActorRotation(),
		SpawnParams
	);
}

void ATalythra::SpawnWaveRed()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FVector SpawnLocation = GetActorLocation();
	SpawnLocation.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	SpawnLocation.Z += 100.f;

	AWaveCircle* WaveCircle1 = GetWorld()->SpawnActor<AWaveCircle>(
		WaveCircleRedClass,
		SpawnLocation,
		GetActorRotation(),
		SpawnParams
	);
}

void ATalythra::SpawnColorOrb()
{
	if (HasAuthority() == false)
		return;

	if (ProjectileBlueOrbClass == nullptr || ProjectilePurpleOrbClass == nullptr)
	{
		return;
	}



	const int32 OrbCount = 12;
	//const float SpawnRadius = 2000.f; // 보스로부터 스폰 거리 
	//const float SpawnHeight = 100.f; // 지면에서 띄울 높이 

	const FVector BossLocation = GetActorLocation();


	FActorSpawnParameters SpawnParms;
	SpawnParms.Owner = this; // 이 엑터를 소유하는 관리 주체 ( 소유관계)
	SpawnParms.Instigator = this;  // 누가 피해를 입혔는지 
	SpawnParms.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;



	for (int32 i = 0; i < OrbCount; i++)
	{
		// 12등분 각도 ( 360 / 12 = 30도 간격 ) 
		const float AngleDeg = (360.f / OrbCount) * i;
		const float AngleRad = FMath::DegreesToRadians(AngleDeg);


		// 원 둘레상의 스폰 위치 
		const FVector OffSet(
			FMath::Cos(AngleRad) * SpawnRadius,
			FMath::Sin(AngleRad) * SpawnRadius,
			SpawnHeight
		);


		const FVector SpawnLocation = BossLocation + OffSet;

		// 보스 (중심)를 향하는 방향 
		FVector DirToBoss = BossLocation - SpawnLocation;
		DirToBoss.Z = 0.f;
		DirToBoss = DirToBoss.GetSafeNormal();


		const FRotator SpawnRotation = DirToBoss.Rotation();

		// 짝 홀로 구분하면 될듯 보라 파랑 소환
		int number = (i + OrbColorOffset) % 2;

		switch (number)
		{
		case 0:
		{
			ATalythraCollectibleOrb* pOrb = GetWorld()->SpawnActor<ATalythraCollectibleOrb>
				(
					ProjectileBlueOrbClass,
					SpawnLocation,
					SpawnRotation,
					SpawnParms
				);

			if (pOrb)
			{
				pOrb->InitVelocityAndDirection(DirToBoss);
				pOrb->Set_OrbColor(EOrbColor::BLUE);
			}
		}
		break;
		case 1:
		{
			ATalythraCollectibleOrb* pOrb = GetWorld()->SpawnActor<ATalythraCollectibleOrb>
				(
					ProjectilePurpleOrbClass,
					SpawnLocation,
					SpawnRotation,
					SpawnParms
				);

			if (pOrb)
			{

				pOrb->InitVelocityAndDirection(DirToBoss);
				pOrb->Set_OrbColor(EOrbColor::PURPLE);
			}
		}
		break;
		}

	}

	OrbColorOffset++;

}

void ATalythra::Render_PlayerAoeOnOff(bool _bOnOff)
{
	for (auto& iter : ArrayOrbHittedDaeva)
	{
		iter->Set_AOE_RenderOnOff(_bOnOff);
	}

}

void ATalythra::Add_OrbHittedDaeva(ADaeva* pDaeva)
{

	if (ArrayOrbHittedDaeva.Find(pDaeva) == -1)
	{
		ArrayOrbHittedDaeva.Add(pDaeva);
	}


}

void ATalythra::Player_Orb_RenderOnOff(bool _bOnOff)
{
	for (auto& iter : ArrayOrbHittedDaeva)
	{
		iter->Set_BlueOrb_RenderOnOff(_bOnOff);
		iter->Set_PurpleOrb_RenderOnOff(_bOnOff);
	}
}

void ATalythra::Reset_PlayerOrbStackAndColor()
{
	for (auto& iter : ArrayOrbHittedDaeva)
	{
		iter->Reset_OrbStackAndColor();
	}
}

void ATalythra::Destroy_OrbShield()
{
	// 여기서 각종 세팅값들 초기화 해주기. 

	for (auto& OrbShield : ArrayOrbShield)
	{
		OrbShield->Destroy();
	}

	for (auto& Daeva : ArrayOrbHittedDaeva)
	{
		Daeva->Set_HasSheildColor(EOrbColor::None);
	}

	// 투사체 맞은 데바들 초기화 하기. 
	ArrayOrbHittedDaeva.Empty();


}

void ATalythra::SpawnColorSheid()
{
	if (HasAuthority() == false)
		return;

	if (ShieldBlueClass == nullptr || ShieldPurpleClass == nullptr)
	{
		return;
	}


	for (auto& iter : ArrayOrbHittedDaeva)
	{
		FVector ShieldLocation = iter->GetActorLocation();
		ShieldLocation.Z = -3258.f;

		FActorSpawnParameters SpawnParms;
		SpawnParms.Owner = this; // 이 엑터를 소유하는 관리 주체 ( 소유관계)
		SpawnParms.Instigator = this;  // 누가 피해를 입혔는지 
		SpawnParms.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		FRotator SpawnRotation = FRotator(0.f, 0.f, 0.f);

		EOrbColor OrbColor = iter->Get_LastOrbColor(); // 오브 색깔

		switch (OrbColor)
		{
		case EOrbColor::BLUE:
		{
			ATalythraGimmickShield* pShield = GetWorld()->SpawnActorDeferred<ATalythraGimmickShield>
				(
					ShieldBlueClass,
					FTransform(SpawnRotation, ShieldLocation),
					SpawnParms.Owner,
					SpawnParms.Instigator,
					ESpawnActorCollisionHandlingMethod::AlwaysSpawn
				);

			// 여기서 쉴드에다가 자신의 색상정보를 알려준다.
			if (pShield)
			{
				pShield->Set_ShieldOrbColor(EOrbColor::BLUE);
				pShield->FinishSpawning(FTransform(SpawnRotation, ShieldLocation));// 이게 있어야 스폰완료 
				ArrayOrbShield.Add(pShield);
			}


		}
		break;
		case EOrbColor::PURPLE:
		{
			ATalythraGimmickShield* pShield = GetWorld()->SpawnActorDeferred<ATalythraGimmickShield>
				(
					ShieldPurpleClass,
					FTransform(SpawnRotation, ShieldLocation),
					SpawnParms.Owner,
					SpawnParms.Instigator,
					ESpawnActorCollisionHandlingMethod::AlwaysSpawn
				);

			// 여기서 쉴드에다가 자신의 색상정보를 알려준다.
			if (pShield)
			{
				pShield->Set_ShieldOrbColor(EOrbColor::PURPLE);
				pShield->FinishSpawning(FTransform(SpawnRotation, ShieldLocation));// 이게 있어야 스폰완료 
				ArrayOrbShield.Add(pShield);
			}
		}
		break;
		}

	}

}


void ATalythra::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);


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
	if (LanchMuzzleEffect == nullptr)
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


	if (bChargeMode)
	{
		GetCharacterMovement()->MaxWalkSpeed = 5000.f;
		GetCharacterMovement()->MaxAcceleration = 50000.f;
		GetCharacterMovement()->GroundFriction = 0.f;
		GetCharacterMovement()->BrakingDecelerationWalking = 0.f;
		GetCharacterMovement()->BrakingFrictionFactor = 0.f;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = 600.f;
		GetCharacterMovement()->MaxAcceleration = 2048.f;
		GetCharacterMovement()->GroundFriction = 8.f;
		GetCharacterMovement()->BrakingDecelerationWalking = 2048.f;
		GetCharacterMovement()->BrakingFrictionFactor = 2.f;
	}
}


void ATalythra::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	Super::OnHealthChanged(Data);

	const float Max = AttributeSet->GetMaxHealth();
	const float Ratio = Max > 0.f ? Data.NewValue / Max : 0.f;


	// 기믹 관련 
	FGameplayTagContainer OwnedTags;
	ASC->GetOwnedGameplayTags(OwnedTags);

	if (Ratio <= 0.9f
		&& OwnedTags.HasTagExact(GIMMICK_MONSTER_TH_HP70_DONE) == false
		&& OwnedTags.HasTagExact(GIMMICK_MONSTER_TH_HP70_PENDING) == false)
	{
		ASC->AddLooseGameplayTag(GIMMICK_MONSTER_TH_HP70_PENDING);
	}


}
