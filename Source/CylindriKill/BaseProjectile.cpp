#include "BaseProjectile.h"
#include "Components/SphereComponent.h"
#include "PaperSpriteComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "CylinderPlayer.h"
#include "CylinderEnemyChar.h"

ABaseProjectile::ABaseProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->InitSphereRadius(15.f);
	CollisionSphere->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Overlap);
	RootComponent = CollisionSphere;

	SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("SpriteComponent"));
	SpriteComponent->SetupAttachment(RootComponent);
	SpriteComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 1200.f;
	ProjectileMovement->MaxSpeed = 1200.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.f;
}

void ABaseProjectile::BeginPlay()
{
	Super::BeginPlay();
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ABaseProjectile::OnSphereOverlap);
}

void ABaseProjectile::OnSphereOverlap(
	UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, 
	bool bFromSweep, 
	const FHitResult& SweepResult
)
{
	UE_LOG(LogTemp, Warning, TEXT("Projectile Overlapped with"));
	if (!OtherActor || OtherActor == this) return;
	UE_LOG(LogTemp, Warning, TEXT("Projectile Overlapped with: %s"), *OtherActor->GetName());
	if (ACylinderPlayer* Player = Cast<ACylinderPlayer>(OtherActor))
	{
		Player->Destroy();
		Destroy();
	}
	else if (!OtherActor->IsA(ACylinderEnemyChar::StaticClass()))
	{
		Destroy();
	}
}