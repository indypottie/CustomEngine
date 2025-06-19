#pragma once
#include "Component.h"

class TextComponent;

class FloatingPointsComponent final : public Component
{
public:
    FloatingPointsComponent(dae::GameObject& owner, float duration = 1.0f, float speed = 30.0f);

    void Update() override;

private:
    void Init();

    float m_Elapsed{};
    float m_Duration;
    float m_Speed;

    TextComponent* m_TextPtr{ nullptr };
};
