#include "Entity.h"
#include "MathHelpers.h"
#include "DamageTextManager.h"

Entity::Entity(PhysicsData::Type ePhysicsType, int type)
    : m_fAttackTimer(1.0f)
    , m_bDeletionRequested(false)
    , m_fAxeTimer(3.0f)
    , m_iType(type)
    , m_iGoldReward(0)
    , m_iDamage(1) // Default damage
{
    m_PhysicsData.m_eType = ePhysicsType;
}

void Entity::OnCollision(Entity& pOtherEntity) {
    if (pOtherEntity.GetPhysicsData().IsInAnyLayer(PhysicsData::Layer::Enemy)) {
        // If we are a projectile
        if (GetPhysicsData().IsInAnyLayer(PhysicsData::Layer::Projectile)) {
            // Projectile hit the enemy, use the damage from the projectile
            pOtherEntity.DealDamage(m_iDamage);
            m_bDeletionRequested = true; 
        }
    }
}

void Entity::DealDamage(int damage) {
    m_iHealth -= damage;
    DamageTextManager::getInstanceNonConst().AddDamageText(damage, GetPosition());
    if (m_iHealth <= 0) {
        m_bDeletionRequested = true;
    }
}