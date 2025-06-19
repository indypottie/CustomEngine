#pragma once
#include "Singleton.h"

class GameOptions final : public dae::Singleton<GameOptions>
{
public:

	void ToggleMute();
	void SkipLevel();

private:


	bool m_IsMuted{ false };
};
