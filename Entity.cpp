#include "Entity.h"
#include "MathHelpers.h"
#include "DamageTextManager.h"

// ==================== ENTITY CONSTRUCTOR ====================
Entity::Entity(PhysicsData::Type ePhysicsType, int type)
    : m_fAttackTimer(1.0f)
    , m_bDeletionRequested(false)
    , m_fBulletTimer(3.0f)
    , m_iPathIndex(0)
    , m_iHealth(100)
    , m_fSpeed(0.0f)
    , m_iDamage(1)
{
    m_PhysicsData.m_eType = ePhysicsType;
    m_HealthBarData.InitializeHealthBar();
}


// ==================== PHYSICS DATA METHODS ====================
bool Entity::PhysicsData::HasCollidedThisUpdate(Entity* pOtherEntity) const {
    for (Entity* pEntity : m_EntitiesThatCollidedWithAlready) {
        if (pEntity == pOtherEntity) {
            return true;
        }
    }
    return false;
}

// ==================== HEALTH BAR DATA METHODS ====================
void Entity::HealthBarData::InitializeHealthBar() {
    // Background (red part)
    m_HealthBarBackground.setSize(sf::Vector2f(m_fHealthBarWidth, m_fHealthBarHeight));
    m_HealthBarBackground.setFillColor(m_HealthBarBackgroundColor);
    m_HealthBarBackground.setOrigin(m_fHealthBarWidth / 2.0f, m_fHealthBarHeight / 2.0f);

    // Foreground (green part)
    m_HealthBarForeground.setSize(sf::Vector2f(m_fHealthBarWidth, m_fHealthBarHeight));
    m_HealthBarForeground.setFillColor(m_HealthBarForegroundColor);
    m_HealthBarForeground.setOrigin(m_fHealthBarWidth / 2.0f, m_fHealthBarHeight / 2.0f);

    // Border
    m_HealthBarBorder.setSize(sf::Vector2f(m_fHealthBarWidth + 2.0f, m_fHealthBarHeight + 2.0f));
    m_HealthBarBorder.setFillColor(sf::Color::Transparent);
    m_HealthBarBorder.setOutlineColor(m_HealthBarBorderColor);
    m_HealthBarBorder.setOutlineThickness(1.0f);
    m_HealthBarBorder.setOrigin((m_fHealthBarWidth + 2.0f) / 2.0f, (m_fHealthBarHeight + 2.0f) / 2.0f);
}

void Entity::HealthBarData::UpdateHealthBarVisuals() {
    if (m_iMaxHealth <= 0) return;

    float healthPercentage = static_cast<float>(m_iCurrentHealth) / static_cast<float>(m_iMaxHealth);
    healthPercentage = std::max(0.0f, std::min(1.0f, healthPercentage)); // Clamp between 0-1

    // Update foreground width based on current health
    float currentWidth = m_fHealthBarWidth * healthPercentage;
    m_HealthBarForeground.setSize(sf::Vector2f(currentWidth, m_fHealthBarHeight));

    // Adjust origin for proper alignment
    m_HealthBarForeground.setOrigin(m_fHealthBarWidth / 2.0f, m_fHealthBarHeight / 2.0f);

    // Update colors based on health percentage
    if (healthPercentage > 0.6f) {
        m_HealthBarForeground.setFillColor(sf::Color::Green);
    }
    else if (healthPercentage > 0.3f) {
        m_HealthBarForeground.setFillColor(sf::Color::Yellow);
    }
    else {
        m_HealthBarForeground.setFillColor(sf::Color::Red);
    }
}

void Entity::HealthBarData::UpdateHealthBarPosition(const sf::Vector2f& entityPosition, const sf::Vector2f& offset) {
    sf::Vector2f healthBarPosition = entityPosition + offset;

    m_HealthBarBackground.setPosition(healthBarPosition);
    m_HealthBarForeground.setPosition(healthBarPosition);
    m_HealthBarBorder.setPosition(healthBarPosition);
}

void Entity::HealthBarData::DrawHealthBar(sf::RenderTarget& target) const {
    if (!ShouldShowHealthBar()) return;

    target.draw(m_HealthBarBackground);
    target.draw(m_HealthBarForeground);
    target.draw(m_HealthBarBorder);
}

bool Entity::HealthBarData::ShouldShowHealthBar() const {
    return m_bShowHealthBar && m_iCurrentHealth > 0;
}

// ==================== PHYSICS SETUP ====================
void Entity::setCirclePhysics(float radius) {
    m_PhysicsData.m_eShape = PhysicsData::Shape::Circle;
    m_PhysicsData.m_fRadius = radius;
}

void Entity::setRectanglePhysics(float width, float height) {
    m_PhysicsData.m_eShape = PhysicsData::Shape::Rectangle;
    m_PhysicsData.m_fWidth = width;
    m_PhysicsData.m_fHeight = height;
}

void Entity::addIgnoredEntity(Entity* entity) {
    m_PhysicsData.m_IgnoredEntities.push_back(entity);
}

bool Entity::shouldIgnoreEntityForPhysics(Entity* entity) const {
    for (const auto& ignoredEntity : m_PhysicsData.m_IgnoredEntities) {
        if (ignoredEntity == entity) {
            return true;
        }
    }

    if (entity->GetPhysicsData().IsInAnyLayer(m_PhysicsData.getLayersToIgnore())) {
        return true;
    }
    return false;
}

// ==================== SPRITE & TRANSFORM ====================
void Entity::SetTexture(const sf::Texture& texture) {
    m_Sprite.setTexture(texture);
}

void Entity::SetScale(const sf::Vector2f& scale) {
    m_Sprite.setScale(scale);
}

void Entity::SetOrigin(const sf::Vector2f& origin) {
    m_Sprite.setOrigin(origin);
}

void Entity::SetPosition(const sf::Vector2f& position) {
    m_Sprite.setPosition(position);
    UpdateHealthBarPosition(); // Update health bar when entity moves
}

void Entity::SetColor(const sf::Color& color) {
    m_Sprite.setColor(color);
}

void Entity::SetSprite(const sf::Sprite& sprite) {
    m_Sprite = sprite;
}

void Entity::SetVelocity(const sf::Vector2f& velocity) {
    m_PhysicsData.m_vVelocity = velocity;
}

void Entity::move(const sf::Vector2f& offset) {
    m_Sprite.move(offset);
    UpdateHealthBarPosition(); // Update health bar when entity moves
}

// ==================== GETTERS ====================
sf::Vector2i Entity::GetClosestGridCoordinates() const {
    return sf::Vector2i(GetPosition().x / 64, GetPosition().y / 64);
}

// ==================== MOVEMENT ====================
void Entity::SetSpeed(float speed) {
    m_fSpeed = speed;
}

float Entity::GetSpeed() const {
    return m_fSpeed;
}
// ==================== HEALTH SYSTEM ====================
void Entity::SetMaxHealth(int maxHealth) {
    m_HealthBarData.m_iMaxHealth = maxHealth;
    m_HealthBarData.m_iCurrentHealth = std::min(m_HealthBarData.m_iCurrentHealth, maxHealth);
    m_HealthBarData.UpdateHealthBarVisuals();
}

void Entity::SetCurrentHealth(int currentHealth) {
    m_HealthBarData.m_iCurrentHealth = std::max(0, std::min(currentHealth, m_HealthBarData.m_iMaxHealth));
    m_HealthBarData.UpdateHealthBarVisuals();
}

void Entity::TakeDamage(int damage) {
    m_HealthBarData.m_iCurrentHealth = std::max(0, m_HealthBarData.m_iCurrentHealth - damage);
    m_HealthBarData.UpdateHealthBarVisuals();

    // Show damage text
    DamageTextManager::getInstanceNonConst().AddDamageText(damage, GetPosition());

    if (m_HealthBarData.m_iCurrentHealth <= 0) {
        m_bDeletionRequested = true;
    }
}

void Entity::ShowHealthBar(bool show) {
    m_HealthBarData.m_bShowHealthBar = show;
}

void Entity::SetHealthBarSize(float width, float height) {
    m_HealthBarData.m_fHealthBarWidth = width;
    m_HealthBarData.m_fHealthBarHeight = height;
    m_HealthBarData.InitializeHealthBar();
    m_HealthBarData.UpdateHealthBarVisuals();
}

void Entity::SetHealthBarOffset(const sf::Vector2f& offset) {
    m_HealthBarData.m_HealthBarOffset = offset;
    UpdateHealthBarPosition();
}

void Entity::UpdateHealthBarPosition() {
    m_HealthBarData.UpdateHealthBarPosition(GetPosition(), m_HealthBarData.m_HealthBarOffset);
}

void Entity::DrawHealthBar(sf::RenderTarget& target) const {
    m_HealthBarData.DrawHealthBar(target);
}

// Legacy health system methods (for compatibility)
void Entity::DealDamage(int damage) {
    // Update both legacy and new health systems
    m_iHealth -= damage;
    TakeDamage(damage);

    if (m_iHealth <= 0) {
        m_bDeletionRequested = true;
    }
}

// ==================== COLLISION ====================
void Entity::OnCollision(Entity& pOtherEntity) {
    if (pOtherEntity.GetPhysicsData().IsInAnyLayer(PhysicsData::Layer::Enemy)) {
        //If we are a projectile
        if (GetPhysicsData().IsInAnyLayer(PhysicsData::Layer::Projectile)) {
            sf::Vector2f direction = pOtherEntity.GetPosition() - GetPosition();
            direction = MathHelpers::normalize(direction);
            pOtherEntity.GetPhysicsDataNonConst().AddImpulse(direction * 64.0f);

            //Projectile hit the enemy
            pOtherEntity.TakeDamage(m_iDamage); // Use new health system
            m_bDeletionRequested = true;
        }
    }
}

// ==================== Add gold/type ====================

int Entity::GetType() const {
    return m_iType;
}

void Entity::SetType(int type) {
    m_iType = type;
}

int Entity::GetGoldReward() const {
    return m_iGoldReward;
}

void Entity::SetGoldReward(int gold) {
    m_iGoldReward = gold;
}

// ==================== Damage====================

void Entity::SetDamage(int damage) {
    m_iDamage = damage;
}

// ==================== RENDERING ====================
void Entity::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    // Draw the main sprite
    target.draw(m_Sprite, states);


    // Draw health bar if it should be shown
    DrawHealthBar(target);
}