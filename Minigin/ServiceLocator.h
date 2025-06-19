#pragma once
//-----------------------------------------------------
// Include Files
//-----------------------------------------------------
#include "CollisionSystem.h"
#include "SoundSystem.h"


//-----------------------------------------------------
// ServiceLocator Class									 
//-----------------------------------------------------
class ServiceLocator final
{
public:

	ServiceLocator() = default;

	//-------------------------------------------------
	// Member functions						
	//-------------------------------------------------

	// sound system
	static SoundSystem& GetSoundSystem();
	static void RegisterSoundSystem(std::unique_ptr<SoundSystem>&& soundSystem);

	// collision system
	static CollisionSystem& GetCollisionSystem();
	static void RegisterCollisionSystem(std::unique_ptr<CollisionSystem>&& collisionSystem);

private:


	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------

	static std::unique_ptr<SoundSystem> SL_SoundSystem;
	static std::unique_ptr<CollisionSystem> SL_CollisionSystem;
};
