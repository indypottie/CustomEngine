#pragma once

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------
#include <memory>

#include "Component.h"
#include "Observer.h"
#include "Subject.h"


//-----------------------------------------------------
// HealthComponent Class									 
//-----------------------------------------------------
class HealthComponent final : public Component
{
public:
	HealthComponent(dae::GameObject& owner, int maxHealth = 3); // Constructor
	~HealthComponent() override = default; // Destructor

	// -------------------------
	// Copy/move constructors and assignment operators
	// -------------------------    
	HealthComponent(const HealthComponent& other)					= delete;
	HealthComponent(HealthComponent&& other) noexcept				= delete;
	HealthComponent& operator=(const HealthComponent& other)		= delete;
	HealthComponent& operator=(HealthComponent&& other) noexcept	= delete;

	//-------------------------------------------------
	// Member functions						
	//-------------------------------------------------
	void AddObserver(Observer* observer);
	void RemoveObserver(Observer* observer);

	void SetMaxHealth(int newMaxHealth);
	void TakeDamage(int amount);

	int GetCurrentHealth() const { return m_CurrentHealth; }

	const std::string DAMAGE_SOUND{ "Resources/Audio/Bomberman_SFX_03.wav" };

private:
	//-------------------------------------------------
	// Private member functions								
	//-------------------------------------------------


	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------

	int m_CurrentHealth;
	int m_MaxHealth;


	std::unique_ptr<Subject> m_SubjectPtr;
};
