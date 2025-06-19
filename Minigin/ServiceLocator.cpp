#include "ServiceLocator.h"

#include <iostream>

std::unique_ptr<SoundSystem> ServiceLocator::SL_SoundSystem = nullptr;
std::unique_ptr<CollisionSystem> ServiceLocator::SL_CollisionSystem = nullptr;

void ServiceLocator::RegisterSoundSystem(std::unique_ptr<SoundSystem>&& soundSystem)
{
	if (soundSystem)
	{
		std::cout << "[ServiceLocator] Sound system registered.\n";
		SL_SoundSystem = std::move(soundSystem);
	}
}

SoundSystem& ServiceLocator::GetSoundSystem()
{
	if (!SL_SoundSystem)
	{
		static NullSoundSystem nullSoundSystem;
		return nullSoundSystem;
	}

	return *SL_SoundSystem;
}


CollisionSystem& ServiceLocator::GetCollisionSystem()
{
	if (!SL_CollisionSystem)
	{
		static NullCollisionSystem nullCollisionSystem;
		return nullCollisionSystem;
	}

	return *SL_CollisionSystem;
}

void ServiceLocator::RegisterCollisionSystem(std::unique_ptr<CollisionSystem>&& collisionSystem)
{
	if (collisionSystem)
	{
		std::cout << "[ServiceLocator] Collision system registered.\n";
		SL_CollisionSystem = std::move(collisionSystem);
	}
}