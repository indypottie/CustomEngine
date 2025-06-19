#pragma once
#include <vec2.hpp>

#include "Singleton.h"

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------


class TextComponent;
//-----------------------------------------------------
// HudManager Class									 
//-----------------------------------------------------

class HudManager final : public dae::Singleton<HudManager>
{
public:

	//-------------------------------------------------
	// Member functions						
	//-------------------------------------------------
	void SetTimerElement	(TextComponent& timerElement)	{ m_TimerElement = &timerElement;			}
	void SetScoreElement	(TextComponent& scoreElement)	{ m_ScoreElement = &scoreElement;			}
	void SetEnemiesElement	(TextComponent& enemiesElement) { m_EnemiesLeftElement = &enemiesElement;	}

	TextComponent* GetScoreElement()	const { return m_ScoreElement;			}
	TextComponent* GetTimerElement()	const { return m_TimerElement;			}
	TextComponent* GetEnemiesElement()	const { return m_EnemiesLeftElement;	}


	void UpdateTime		(int time)		const;
	void UpdateScore	(int score)		const;
	void UpdateEnemies	(int enemies)	const;

	bool IsInitialized() const;


	static void ShowFloatingPoints(int amount, const glm::vec2& pos);

private:
	//-------------------------------------------------
	// Private member functions								
	//-------------------------------------------------

	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
	TextComponent* m_TimerElement		{ nullptr };
	TextComponent* m_ScoreElement		{ nullptr };
	TextComponent* m_EnemiesLeftElement	{ nullptr };
};
