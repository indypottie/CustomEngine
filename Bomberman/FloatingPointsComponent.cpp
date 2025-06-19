#include "FloatingPointsComponent.h"

#include "EngineTime.h"
#include "GameObject.h"
#include "TextComponent.h"


FloatingPointsComponent::FloatingPointsComponent(dae::GameObject& owner, float duration, float speed) :
	Component(owner),
	m_Duration(duration),
	m_Speed(speed)
{
	Init();
}

void FloatingPointsComponent::Init()
{
	m_TextPtr = GetOwner()->GetComponent<TextComponent>();
	assert(m_TextPtr && "FloatingPointsComponent requires a TextComponent!");
}

void FloatingPointsComponent::Update()
{
	const float dt = EngineTime::GetInstance().GetDeltaTime();
	m_Elapsed += dt;

	// Float upward
	const auto currentPos = GetOwner()->GetPosition();
	GetOwner()->SetPosition(currentPos.x, currentPos.y - m_Speed * dt);

	// Fade out
	if (m_TextPtr)
	{
		float alpha = std::max(0.f, 1.f - (m_Elapsed / m_Duration));
		SDL_Color color = m_TextPtr->GetColor();
		color.a = static_cast<Uint8>(alpha * 255.f);
		m_TextPtr->SetColor(color);
	}

	// Destroy after time is up
	if (m_Elapsed >= m_Duration)
	{
		GetOwner()->MarkForDeletion();
	}
}


