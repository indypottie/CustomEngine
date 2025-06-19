#include "GameOptions.h"

#include "LevelManager.h"
#include "ServiceLocator.h"


void GameOptions::ToggleMute()
{
	m_IsMuted = !m_IsMuted;

	ServiceLocator::GetSoundSystem().Mute(m_IsMuted);
}

void GameOptions::SkipLevel()
{
	LevelManager::GetInstance().SkipLevel();
}
