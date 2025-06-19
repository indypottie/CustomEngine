#pragma once
//-----------------------------------------------------
// Include Files
//-----------------------------------------------------
#include <SDL_rect.h>

#include "Component.h"
#include "Observer.h"


//-----------------------------------------------------
// ColliderComponent Class									 
//-----------------------------------------------------
class ColliderComponent final : public Component, public Observer
{
public:
	ColliderComponent(dae::GameObject& owner, SDL_FRect collisionRect); // Constructor
	~ColliderComponent() override; // Destructor

	// -------------------------
	// Copy/move constructors and assignment operators
	// -------------------------    
	ColliderComponent(const ColliderComponent& other)					= delete;
	ColliderComponent(ColliderComponent&& other) noexcept				= delete;
	ColliderComponent& operator=(const ColliderComponent& other)		= delete;
	ColliderComponent& operator=(ColliderComponent&& other) noexcept	= delete;

	//-------------------------------------------------
	// Member functions						
	//-------------------------------------------------

	void Update() override;
	void Render() const override;

	const SDL_FRect& GetCollisionRect() const { return m_CollisionRect; }

	void Notify(const Event& event, dae::GameObject* actor) override;

private:
	//-------------------------------------------------
	// Private member functions								
	//-------------------------------------------------


	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
	SDL_FRect m_CollisionRect; 

	bool m_Registered{ false };
};
