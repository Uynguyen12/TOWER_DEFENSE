#include "Entity.h"
#include "MathHelpers.h"
#include "DamageTextManager.h"

Entity::Entity(PhysicsData::Type ePhysicsType)
	: m_fAttackTimer(1.0f)
	, m_bDeletionRequested(false)
	, m_fAxeTimer(3.0f)
{
	m_PhysicsData.m_eType = ePhysicsType;
}

void Entity::OnCollision(Entity& pOtherEntity) {
	if (pOtherEntity.GetPhysicsData().IsInAnyLayer(PhysicsData::Layer::Enemy)) {
		//If we are a projectile
		if (GetPhysicsData().IsInAnyLayer(PhysicsData::Layer::Projectile)) {
			sf::Vector2f direction = pOtherEntity.GetPosition() - GetPosition();
			direction = MathHelpers::normalize(direction);
			pOtherEntity.GetPhysicsDataNonConst().AddImpulse(direction * 80.0f);

			//Projectile hit the enemy
			pOtherEntity.DealDamage(1);
			m_bDeletionRequested = true;
			// ADDED: Play sound effect
			SoundManager::getInstance().playSfx(SoundManager::SfxType::EnemyHit);
		}
	}
}

void Entity::DealDamage(int damage) {
	m_iHealth -= damage;
	DamageTextManager::getInstanceNonConst().AddDamageText(damage, GetPosition());
	// ADDED: Trigger a visual flash effect
	m_fFlashTimer = 0.2f; // Flash for 0.2 seconds
	if (m_iHealth <= 0) {
		m_bDeletionRequested = true;
	}
}
