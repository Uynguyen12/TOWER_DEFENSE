#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
using namespace std;

#ifndef ENTITY_H	
#define ENTITY_H

class Entity : public sf::Drawable
{
public:
    // ==================== PHYSICS DATA STRUCTURE ====================
    struct PhysicsData {
        PhysicsData() {
            m_vImpulse = sf::Vector2f(0.0f, 0.0f);
        }

        enum Layer {
            Enemy = 1,      // 0b0001
            Tower = 2,      // 0b0010
            Projectile = 4  // 0b0100
        };

        enum class Shape {
            Circle,
            Rectangle
        };

        enum class Type {
            Static,
            Dynamic
        };

        Shape m_eShape;
        Type m_eType;

        // Layer management
        void setLayers(int layers) { m_iMyLayer = layers; }
        void setLayersToIgnore(int layers) { m_iLayersToIgnore = layers; }
        int getLayersToIgnore() const { return m_iLayersToIgnore; }
        bool IsInAnyLayer(int layer) const { return (m_iMyLayer & layer) != 0; }

        // Collision management
        void ClearCollisions() { m_EntitiesThatCollidedWithAlready.clear(); }
        bool HasCollidedThisUpdate(Entity* pOtherEntity) const;
        void AddEntityCollision(Entity* pOtherEntity) { m_EntitiesThatCollidedWithAlready.push_back(pOtherEntity); }

        // Physics properties
        void ClearImpulse() { m_vImpulse = sf::Vector2f(0.0f, 0.0f); }
        void AddImpulse(const sf::Vector2f& impulse) { m_vImpulse += impulse; }

        // Member variables
        int m_iMyLayer;
        int m_iLayersToIgnore;
        float m_fRadius;    // For Circle shape
        float m_fWidth;     // For Rectangle shape
        float m_fHeight;    // For Rectangle shape
        sf::Vector2f m_vVelocity;
        sf::Vector2f m_vImpulse;
        vector<Entity*> m_IgnoredEntities;
        vector<Entity*> m_EntitiesToIgnore;
        vector<Entity*> m_EntitiesThatCollidedWithAlready;
    };

    // ==================== HEALTH BAR STRUCTURE ====================
    struct HealthBarData {
        HealthBarData()
            : m_iMaxHealth(100)
            , m_iCurrentHealth(100)
            , m_bShowHealthBar(false)
            , m_fHealthBarWidth(50.0f)
            , m_fHealthBarHeight(6.0f)
            , m_HealthBarOffset(0.0f, -45.0f)
            , m_HealthBarBackgroundColor(sf::Color::Red)
            , m_HealthBarForegroundColor(sf::Color::Green)
            , m_HealthBarBorderColor(sf::Color::Black)
        {
        }

        int m_iMaxHealth = 100;
        int m_iCurrentHealth = 100;
        bool m_bShowHealthBar = false;
        float m_fHealthBarWidth = 50.f;
        float m_fHealthBarHeight = 6.0f;
        sf::Vector2f m_HealthBarOffset;

        // Health bar visual components
        sf::RectangleShape m_HealthBarBackground;
        sf::RectangleShape m_HealthBarForeground;
        sf::RectangleShape m_HealthBarBorder;

        // Health bar colors
        sf::Color m_HealthBarBackgroundColor;
        sf::Color m_HealthBarForegroundColor;
        sf::Color m_HealthBarBorderColor;

        // Health bar methods
        void InitializeHealthBar();
        void UpdateHealthBarVisuals();
        void UpdateHealthBarPosition(const sf::Vector2f& entityPosition, const sf::Vector2f& offset);
        void DrawHealthBar(sf::RenderTarget& target) const;
        bool ShouldShowHealthBar() const;
    };

    // ==================== CONSTRUCTORS & DESTRUCTOR ====================
    Entity() : m_fAttackTimer(1.0f), m_bDeletionRequested(false), m_fBulletTimer(3.0f), m_iType(0), m_iGoldReward(0), m_iDamage(1) {
        m_PhysicsData.m_eType = PhysicsData::Type::Dynamic;
    }
    Entity(PhysicsData::Type ePhysicsType, int type);
    ~Entity() {}

    // ==================== PHYSICS SETUP ====================
    void setCirclePhysics(float radius);
    void setRectanglePhysics(float width, float height);
    void addIgnoredEntity(Entity* entity);
    bool shouldIgnoreEntityForPhysics(Entity* entity) const;

    // ==================== SPRITE & TRANSFORM ====================
    void SetTexture(const sf::Texture& texture);
    void SetScale(const sf::Vector2f& scale);
    void SetOrigin(const sf::Vector2f& origin);
    void SetPosition(const sf::Vector2f& position);
    void SetColor(const sf::Color& color);
    void SetSprite(const sf::Sprite& sprite);
    void SetVelocity(const sf::Vector2f& velocity);
    sf::Vector2f GetVelocity() const {
        return m_PhysicsData.m_vVelocity;
    }
    void move(const sf::Vector2f& offset);

    // ==================== GETTERS ====================
    const sf::Sprite& GetSprite() const { return m_Sprite; }
    sf::Sprite& GetSpriteNonConst() { return m_Sprite; }
    sf::Vector2f GetPosition() const { return m_Sprite.getPosition(); }
    sf::Vector2i GetClosestGridCoordinates() const;
    PhysicsData::Type GetPhysicsShapeType() const { return m_PhysicsData.m_eType; }
    const PhysicsData& GetPhysicsData() const { return m_PhysicsData; }
    PhysicsData& GetPhysicsDataNonConst() { return m_PhysicsData; }

    // ==================== MOVEMENT ==================== // 
    void SetSpeed(float speed);
    float GetSpeed() const;

    // ==================== PATHFINDING ====================
    void SetPathIndex(int index) { m_iPathIndex = index; }
    int GetPathIndex() const { return m_iPathIndex; }

    // ==================== HEALTH SYSTEM ====================
    void SetHealth(int health) { m_iHealth = health; }
    int getHealth() const { return m_iHealth; }
    void DealDamage(int damage);

    // New health bar methods
    void SetMaxHealth(int maxHealth);
    void SetCurrentHealth(int currentHealth);
    void TakeDamage(int damage);
    void ShowHealthBar(bool show);
    void SetHealthBarSize(float width, float height);
    void SetHealthBarOffset(const sf::Vector2f& offset);
    void UpdateHealthBarPosition();
    void DrawHealthBar(sf::RenderTarget& target) const;

    // Health bar getters
    int GetCurrentHealth() const { return m_HealthBarData.m_iCurrentHealth; }
    int GetMaxHealth() const { return m_HealthBarData.m_iMaxHealth; }
    bool ShouldShowHealthBar() const { return m_HealthBarData.ShouldShowHealthBar(); }

    // ==================== COLLISION & DELETION ====================
    void OnCollision(Entity& pOtherEntity);
    bool IsDeletionRequested() const { return m_bDeletionRequested; }
    void RequestDeletion() { m_bDeletionRequested = true; }

    // ==================== COLLISION & DELETION ====================
    int GetType() const;
    void SetType(int type);
    int GetGoldReward() const;
    void SetGoldReward(int gold);

    // ==================== Deal damage ====================
    int GetDamage() const { return m_iDamage; }
    void SetDamage(int damage);


    // ==================== RENDERING ====================
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    // ==================== PUBLIC TIMERS ====================
    float m_fBulletTimer;
    float m_fAttackTimer;

private:
    // ==================== CORE COMPONENTS ====================
    sf::Sprite m_Sprite;
    PhysicsData m_PhysicsData;
    HealthBarData m_HealthBarData;
    bool m_bDeletionRequested;

    // ==================== GAME DATA ====================
    int m_iPathIndex;
    int m_iHealth; // Legacy health system - keeping for compatibility
    int m_iType; // For tower/enemy type (1-4)
    int m_iGoldReward; // Gold reward for killing enemies
    int m_iDamage; // Damage amount for projectiles
    float m_fSpeed;
};

#endif