#pragma once

#include <iostream>
#include <memory>
#include <string>

#include <box2d/box2d.h>
#include <Paingine/Graphics/Sprite.hpp>

#include "Graphics/LayerManager.h"
#include "Assets/Resources.h"

class Actor {
public:
    Actor(b2WorldId worldId, const std::string &texturePath, int renderLayer = 0)
        : m_WorldId(worldId)
          , m_RenderLayer(renderLayer) {
        LoadTexture(texturePath);
        LayerManager::AddDrawable(m_RenderLayer, &m_Sprite);
    }

    virtual ~Actor() {
        LayerManager::RemoveDrawable(&m_Sprite);

        if (B2_IS_NON_NULL(m_ShapeId)) {
            b2DestroyShape(m_ShapeId, true);
            m_ShapeId = b2_nullShapeId;
        }

        if (B2_IS_NON_NULL(m_BodyId)) {
            b2DestroyBody(m_BodyId);
            m_BodyId = b2_nullBodyId;
        }
    }

    virtual void Init() {
    }

    bool LoadTexture(const std::string &texturePath) {
        auto texture = Resources::GetTexture(texturePath);
        if (!texture) {
            std::cerr << "Failed to load texture: " << texturePath << std::endl;
            return false;
        }
        m_Sprite.setTexture(*texture);
        m_Sprite.setOrigin(texture->getSize().x / 2.0f, texture->getSize().y / 2.0f);
        return true;
    }

    void CreatePhysicsBody(const b2BodyDef &bodyDef) {
        if (B2_IS_NON_NULL(m_BodyId)) {
            b2DestroyBody(m_BodyId);
        }
        m_BodyId = b2CreateBody(m_WorldId, &bodyDef);
    }

    void AddBoxShape(const b2Vec2 &size, const b2ShapeDef &shapeDef = b2DefaultShapeDef()) {
        if (B2_IS_NON_NULL(m_BodyId)) {
            if (B2_IS_NON_NULL(m_ShapeId)) {
                b2DestroyShape(m_ShapeId, true);
            }

            b2Polygon box = b2MakeBox(size.x, size.y);
            m_ShapeId = b2CreatePolygonShape(m_BodyId, &shapeDef, &box);
        }
    }

    void AddCircleShape(float radius, const b2ShapeDef &shapeDef = b2DefaultShapeDef()) {
        if (B2_IS_NON_NULL(m_BodyId)) {
            if (B2_IS_NON_NULL(m_ShapeId)) {
                b2DestroyShape(m_ShapeId, true);
            }

            b2Circle circle = {0};
            circle.radius = radius;
            m_ShapeId = b2CreateCircleShape(m_BodyId, &shapeDef, &circle);
        }
    }

    void SetPosition(const b2Vec2 &position) {
        if (B2_IS_NON_NULL(m_BodyId)) {
            b2Body_SetTransform(m_BodyId, position, b2Body_GetRotation(m_BodyId));
        }
        UpdateGraphics();
    }

    void SetRotation(b2Rot angle) {
        if (B2_IS_NON_NULL(m_BodyId)) {
            b2Body_SetTransform(m_BodyId, b2Body_GetPosition(m_BodyId), angle);
        }
        UpdateGraphics();
    }

    void SetScale(const Paingine2D::Vector2f &scale) {
        m_Sprite.setScale(scale);
    }

    void SetRenderLayer(int newLayer) {
        if (m_RenderLayer != newLayer) {
            LayerManager::ChangeLayer(&m_Sprite, newLayer);
            m_RenderLayer = newLayer;
        }
    }

    void SetLinearVelocity(const b2Vec2 &velocity) {
        if (B2_IS_NON_NULL(m_BodyId)) {
            b2Body_SetLinearVelocity(m_BodyId, velocity);
        }
    }

    void SetAngularVelocity(float omega) {
        if (B2_IS_NON_NULL(m_BodyId)) {
            b2Body_SetAngularVelocity(m_BodyId, omega);
        }
    }

    virtual void Update() {
        UpdateGraphics();
    }

    b2BodyId GetBodyId() const { return m_BodyId; }
    b2ShapeId GetShapeId() const { return m_ShapeId; }
    const Paingine2D::Sprite &GetSprite() const { return m_Sprite; }
    Paingine2D::Sprite &GetSprite() { return m_Sprite; }
    int GetRenderLayer() const { return m_RenderLayer; }

    b2Vec2 GetPosition() const {
        if (B2_IS_NON_NULL(m_BodyId)) {
            return b2Body_GetPosition(m_BodyId);
        }
        return b2Vec2{0.0f, 0.0f};
    }

    float GetRotation() const {
        if (B2_IS_NON_NULL(m_BodyId)) {
            b2Rot rotation = b2Body_GetRotation(m_BodyId);
            return b2Rot_GetAngle(rotation);
        }
        return 0.0f;
    }

    b2Vec2 GetLinearVelocity() const {
        if (B2_IS_NON_NULL(m_BodyId)) {
            return b2Body_GetLinearVelocity(m_BodyId);
        }
        return b2Vec2{0.0f, 0.0f};
    }

    float GetAngularVelocity() const {
        if (B2_IS_NON_NULL(m_BodyId)) {
            return b2Body_GetAngularVelocity(m_BodyId);
        }
        return 0.0f;
    }

protected:
    virtual void UpdateGraphics() {
        if (B2_IS_NON_NULL(m_BodyId)) {
            b2Vec2 position = b2Body_GetPosition(m_BodyId);
            b2Rot rotation = b2Body_GetRotation(m_BodyId);
            float angle = b2Rot_GetAngle(rotation);

            constexpr float SCALE = 30.0f;
            m_Sprite.setPosition(position.x * SCALE, position.y * SCALE);
            m_Sprite.setRotation(angle * 180.0f / b2_pi);
        }
    }

    b2WorldId m_WorldId;
    b2BodyId m_BodyId = b2_nullBodyId;
    b2ShapeId m_ShapeId = b2_nullShapeId;
    Paingine2D::Sprite m_Sprite;
    int m_RenderLayer;
};
