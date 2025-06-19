#pragma once


//-----------------------------------------------------
// Include Files
//-----------------------------------------------------
#include "Component.h"
#include "Observer.h"

class Observer;
class Subject;
//-----------------------------------------------------
// BombComponent Class									 
//-----------------------------------------------------
class BombComponent final : public Component, public Observer
{
public:
	BombComponent(dae::GameObject& owner, int range = 1); // Constructor
	~BombComponent() override = default; // Destructor

	// -------------------------
	// Copy/move constructors and assignment operators
	// -------------------------    
	BombComponent(const BombComponent& other)					= delete;
	BombComponent(BombComponent&& other) noexcept				= delete;
	BombComponent& operator=(const BombComponent& other)		= delete;
	BombComponent& operator=(BombComponent&& other) noexcept	= delete;

	//-------------------------------------------------
	// Member functions						
	//-------------------------------------------------
	void Update() override;

	constexpr static float BOMB_EXPLOSION_TIME{ 2.f }; // time before bomb explodes

	void Notify(const Event& event, dae::GameObject* actor) override;

	bool HasExploded() const { return m_Exploded; }

	void Explode();

private:
	//-------------------------------------------------
	// Private member functions								
	//-------------------------------------------------



	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
	float m_Timer;
	int m_ExplosionRange;
	bool m_Exploded{ false };
};
