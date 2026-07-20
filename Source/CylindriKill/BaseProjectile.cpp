#include "BaseProjectile.h"

#include "BaseEnemy.h"
#include "Components/SphereComponent.h"
#include "PaperSpriteComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "CylinderPlayer.h"
#include "CylinderEnemyChar.h"
#include "CylindriKill/Component/HealthComponent.h"

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
	if (!OtherActor || OtherActor == this) return;

	// Enemy projectiles shouldn't collide with or damage other enemies - this is a friendly-fire
	// rule, a separate concern from "can this thing be damaged," so it intentionally still checks
	// the concrete enemy class rather than going through UHealthComponent.
	if (OtherActor->IsA(ACylinderEnemyChar::StaticClass()) || OtherActor->IsA(ABaseEnemy::StaticClass()))
	{
		return; // pass straight through, no damage, no destroy
	}

	if (UHealthComponent* TargetHealth = UHealthComponent::FindHealthComponent(OtherActor))
	{
		TargetHealth->ApplyDamage(ProjectileDamage, GetOwner(), nullptr);
		Destroy();
		return;
	}

	// Hit something with no health component at all (world geometry, etc.) - just disappear.
	Destroy();
}