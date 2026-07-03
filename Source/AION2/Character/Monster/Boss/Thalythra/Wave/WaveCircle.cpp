// Fill out your copyright notice in the Description page of Project Settings.


#include "WaveCircle.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Character/Daeva/Daeva.h"
#include "Engine/OverlapResult.h"
#include "Components/CapsuleComponent.h"
#include "Character/Monster/AOMonsterBase.h"

// Sets default values
AWaveCircle::AWaveCircle()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	//Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	//ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));

	RootComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Root"));


	RingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RingMesh"));
	RingMesh->SetupAttachment(RootComponent);
	RingMesh->SetCastShadow(false);


	// Replication 설정 
	bReplicates = true;					// 이 Actor를 클라이언트에도 만들어지도록 설정
	SetReplicateMovement(false);			// 이 Actor의 위치/회전/속도를 클라이언트에 동기화 설정
	SetNetUpdateFrequency(100.f);		// 초당 최대 100번까지 클라에게 패킷을 보내라.
	SetMinNetUpdateFrequency(30.f);     // 변화가 없거나 우선순위가 낮아도 초당 최소 30번은 체크 보장해라. 

	
}

// Called when the game starts or when spawned
void AWaveCircle::BeginPlay()
{
	Super::BeginPlay();

	if(RingPulseCurve)
	{
		FOnTimelineFloat ProgressDelegate; 
		ProgressDelegate.BindUFunction(this, FName("UpdateRingScale")); 
		
		// 타임라인이 재생되는 동안 매 틱마다 커브에서 현재 시간에 해당하는 FLAOT값을 샘플링해서 그 값을 인자로 
		// 델리게이트에 바인딩된 함수를 호출한다.
		// 즉, 타임라인이 재생 중인 매 프레임(틱) 마다 호출된다. 
		RingPulseTimeline.AddInterpFloat(RingPulseCurve, ProgressDelegate); 
		RingPulseTimeline.SetPlayRate(PulsePlayRate);  // 0.5 = 2배 느리게 (1초 커브 → 2초 재생)


		FOnTimelineEvent FinishedDelegate; 
		FinishedDelegate.BindUFunction(this, FName("OnRingPulseFinished"));
		RingPulseTimeline.SetTimelineFinishedFunc(FinishedDelegate);

	}


	RingMesh->SetWorldScale3D(FVector(BaseRingScale));

	// 서버,클라 모두 BeginPlay에서 자동 재생.
	// 서버는 렌더링 안 되지만 소멸 타이밍 관리를 위해 Timeline은 돌려야 함.
	PlayRingPulse(); 


	
	const FBoxSphereBounds Bounds = RingMesh->GetStaticMesh()->GetBounds();
	BaseMeshRadius = Bounds.BoxExtent.X;  // 링 외곽까지의 반경

	UE_LOG(LogTemp, Warning, TEXT("Ring X Radius : %f"), BaseMeshRadius);
}

// Called every frame
void AWaveCircle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RingPulseTimeline.TickTimeline(DeltaTime);

	// 서버에서만 충돌로직 검사. 
	//if(HasAuthority())
	//CheckRingHit();
}


void AWaveCircle::PlayRingPulse()
{
	RingMesh->SetWorldScale3D(FVector(BaseRingScale));
	RingPulseTimeline.PlayFromStart(); // 여기가 다시 curve 곡선을 0부터 시작하게 만들어주는 구간. 

}

void AWaveCircle::UpdateRingScale(float Alpha)
{
	// Alpha: 0(기준) -> 1 (최대) -> 0 (복귀) 왕복 
	const float CurrentScale = FMath::Lerp(BaseRingScale, MaxRingScale, Alpha);
	
	RingMesh->SetWorldScale3D(FVector(CurrentScale, CurrentScale, 0.1f/*BaseRingScale*/));

	// 이거는 서버에서만 하도록 설정하기 
	if(HasAuthority())
		CheckRingHit();
}

void AWaveCircle::OnRingPulseFinished()
{
	RingMesh->SetWorldScale3D(FVector(BaseRingScale));

	// 소멸은 서버에서만. Replicate로 각 클라에서도 파괴됨.
	if (DestroyOnFinished && HasAuthority())
	{
		Destroy();
	}

}
//
void AWaveCircle::CheckRingHit()
{
	// 현재 스케일 → 실제 월드 반경
	const float CurrentScale = RingMesh->GetComponentScale().X;
	const float OuterRadius = BaseMeshRadius * CurrentScale;
	const float RingThickness = BaseRingRadius * CurrentScale; // 링 두께 (판정 폭)
	const float InnerRadius = FMath::Max(0.f, OuterRadius - RingThickness);


	// 이건 나중에 한번마 초기화 해주자. 
	FVector CurrentCenterLocation = GetInstigator()->GetActorLocation(); 

	// 몬스터가 서있는 발위치 높이 계산 및 z축 충돌 범위 설정 
	AAOMonsterBase* pMonster = Cast<AAOMonsterBase>(GetInstigator());
	CurrentCenterLocation.Z = pMonster->GetActorLocation().Z - pMonster->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + RingCollisionHeight;


	//  오버랩 판정 
	TArray<FOverlapResult> Overlaps; 
	const FCollisionShape Sphere = FCollisionShape::MakeSphere(OuterRadius);

	FCollisionQueryParams Params; 
	Params.AddIgnoredActor(this); 

	if (GetInstigator())
		Params.AddIgnoredActor(GetInstigator()); // 시전자 제외 

	GetWorld()->OverlapMultiByChannel(Overlaps, CurrentCenterLocation, FQuat::Identity, ECC_Pawn, Sphere, Params);

	for(const FOverlapResult& Result : Overlaps)
	{

		AActor* Target = Result.GetActor();
		if (Target == nullptr || AlreadyHit.Contains(Target))
			continue; 


		// 도넛 판정 inner ~ outer 사이만. 
		const float Dist = FVector::Dist2D(CurrentCenterLocation, Target->GetActorLocation());
		

		// 데바만 충돌처리하기 위해 설정 
		ADaeva* pDaeva = Cast<ADaeva>(Target);
		if (pDaeva == nullptr)
			continue;

		
		float CurrentDevaHeight = pDaeva->GetActorLocation().Z - pDaeva->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();


		if( Dist >= InnerRadius && Dist <= OuterRadius && CurrentCenterLocation.Z >= CurrentDevaHeight)
		{
			AlreadyHit.Add(Target);

		
			FHitResult HitResult;
			HitResult.HitObjectHandle = FActorInstanceHandle(Target);
			HitResult.Location = Target->GetActorLocation();
			HitResult.ImpactPoint = Target->GetActorLocation();

			// 파동 중심에서 대상 방향을 노멀로
			FVector Dir = (Target->GetActorLocation() - CurrentCenterLocation);
			Dir.Z = 0.f;
			HitResult.ImpactNormal = Dir.GetSafeNormal();
			HitResult.Normal = HitResult.ImpactNormal;

			HitResult.Component = Cast<UPrimitiveComponent>(Result.GetComponent());
			HitResult.bBlockingHit = true;
			
			
			// OnAttackSucceeded 호출하기 위해 형변환
			AAOCharacter* pMonster = Cast<AAOCharacter>(GetInstigator());

			if (pMonster == nullptr)
				continue;

			bool bCameraShake = false; 
			pMonster->OnAttackSucceeded(AttackData, pDaeva, HitResult, bCameraShake);
		}
	}

}



// 디버그 드로잉 
// XY 평면에 눕힌 원 (링이 바닥에 누워있으므로 Z축 기준 회전)
//DrawDebugCircle(GetWorld(), CurrentCenterLocation, OuterRadius, 64, FColor::Red, false, -1.f, 0, 5.f,
//	FVector(1, 0, 0), FVector(0, 1, 0), false);
//DrawDebugCircle(GetWorld(), CurrentCenterLocation, InnerRadius, 64, FColor::Green, false, -1.f, 0, 5.f,
//	FVector(1, 0, 0), FVector(0, 1, 0), false);




// 충돌 디버깅용 
// if(Cast<ADaeva>(Target))
// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, Target->GetName());
