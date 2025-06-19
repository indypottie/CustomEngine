#pragma once
//-----------------------------------------------------
// Include Files
//-----------------------------------------------------
#include "Component.h"
#include "Transform.h"

//-----------------------------------------------------
// TransformComponent Class									 
//-----------------------------------------------------
class TransformComponent final : public Component
{
public:
	TransformComponent(dae::GameObject& owner); // Constructor
	~TransformComponent() override = default; // Destructor

	// -------------------------
	// Copy/move constructors and assignment operators
	// -------------------------    
	TransformComponent(const TransformComponent& other)					= delete;
	TransformComponent(TransformComponent&& other) noexcept				= delete;
	TransformComponent& operator=(const TransformComponent& other)		= delete;
	TransformComponent& operator=(TransformComponent&& other) noexcept	= delete;

	//-------------------------------------------------
	// Member functions						
	//-------------------------------------------------

	// position setters
	void SetLocalPosition(const glm::vec2& pos);
	void SetWorldPosition(const glm::vec2& pos); // used for teleporting objects

	// position getters
	const glm::vec2& GetLocalPosition() const { return m_LocalPosition; }
	const glm::vec2& GetWorldPosition(); // Computed if dirty

	// mark transform as dirty
	void MarkDirty();

private:
	//-------------------------------------------------
	// Private member functions								
	//-------------------------------------------------
	void UpdateWorldTransform();

	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
	glm::vec2 m_LocalPosition{};
	glm::vec2 m_WorldPosition{};
	bool m_IsDirty = true; // Used to track updates
};
