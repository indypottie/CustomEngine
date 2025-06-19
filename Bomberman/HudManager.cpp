//---------------------------
// Include Files
//---------------------------
#include "HudManager.h"

#include "FloatingPointsComponent.h"
#include "GameObject.h"
#include "ResourceManager.h"
#include "Scene.h"
#include "SceneManager.h"
#include "TextComponent.h"


//---------------------------
// Member functions
//---------------------------

void HudManager::UpdateTime(int time) const
{
	if (!m_TimerElement) return;

	std::string newText = "TIME " + std::to_string(time);
	m_TimerElement->SetText(newText);
}

void HudManager::UpdateScore(int score) const
{
	if (!m_ScoreElement) return;

	std::string newText = std::to_string(score);
	m_ScoreElement->SetText(newText);
}

void HudManager::UpdateEnemies(int enemies) const
{
	if (!m_EnemiesLeftElement) return;

	std::string newText = "LEFT " + std::to_string(enemies);
	m_EnemiesLeftElement->SetText(newText);
}

bool HudManager::IsInitialized() const
{
	return m_ScoreElement && m_TimerElement && m_EnemiesLeftElement;
}

void HudManager::ShowFloatingPoints(int amount, const glm::vec2& pos)
{
	auto floatingText = std::make_unique<dae::GameObject>();
	floatingText->SetPosition(pos.x, pos.y);

	auto font = dae::ResourceManager::GetInstance().LoadFont("Emulogic.ttf", 12);

	floatingText->AddComponent<TextComponent>(std::to_string(amount), font);
	floatingText->AddComponent<FloatingPointsComponent>();

	dae::SceneManager::GetInstance().GetActiveScene()->Add(std::move(floatingText));
}
