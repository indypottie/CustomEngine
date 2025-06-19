#include "StartMenuSelectorComponent.h"

#include <utility>

#include "GameObject.h"
#include "ServiceLocator.h"


StartMenuSelectorComponent::StartMenuSelectorComponent(dae::GameObject& owner) : Component(owner)
{
	Init();
}

void StartMenuSelectorComponent::Init() const
{
	UpdateArrowPosition();
}

void StartMenuSelectorComponent::Update()
{
	
}

void StartMenuSelectorComponent::AddOption(const glm::vec2& position, const std::function<void()>& onSelect)
{
	m_OptionPositions.push_back(position);
	m_OnSelectActions.push_back(onSelect);
}

void StartMenuSelectorComponent::MoveUp()
{
	if (--m_CurrentIndex < 0)
		m_CurrentIndex = static_cast<int>(m_OptionPositions.size()) - 1;
	UpdateArrowPosition();
}

void StartMenuSelectorComponent::MoveDown()
{
	m_CurrentIndex = (m_CurrentIndex + 1) % static_cast<int>(m_OptionPositions.size());
	UpdateArrowPosition();
}

void StartMenuSelectorComponent::Select() const
{
	if (m_CurrentIndex >= 0 && std::cmp_less(m_CurrentIndex, m_OnSelectActions.size()))
	{
		m_OnSelectActions[m_CurrentIndex]();
		GetOwner()->MarkForDeletion();
	}
}

void StartMenuSelectorComponent::UpdateArrowPosition() const
{
	if (!m_OptionPositions.empty())
	{
		const auto newPos = m_OptionPositions[m_CurrentIndex];

		GetOwner()->SetPosition(newPos.x, newPos.y);

		ServiceLocator::GetSoundSystem().QueueSound("Resources/Audio/MenuMove.wav");
	}
}
