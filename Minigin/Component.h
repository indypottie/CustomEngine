#pragma once

namespace dae
{
	class GameObject;
}
//-----------------------------------------------------
// Component Class									 
//-----------------------------------------------------
class Component
{
public:
	virtual ~Component() = default; // Destructor

	// -------------------------
	// Copy/move constructors and assignment operators
	// -------------------------
	Component(const Component& other)					= delete;
	Component(Component&& other)						= delete;
	Component& operator=(const Component& other)		= delete;
	Component& operator=(Component&& other) noexcept	= delete;

	//-------------------------------------------------
	// Member functions						
	//-------------------------------------------------
	virtual void Update();
	virtual void Render() const;

	void MarkForDeletion() { m_IsMarkedForDeletion = true; }
	bool IsMarkedForDeletion() const { return m_IsMarkedForDeletion; }

	dae::GameObject* GetOwner() const;

	virtual int GetRenderPriority() const { return 5; } // base layer is 5 (leaves room for components that want to be drawn at lower layers like backgrounds)

protected:
	explicit Component(dae::GameObject& owner); // owner is set at creation and cannot be changed

private:
	//-------------------------------------------------
	// Private member functions								
	//-------------------------------------------------

	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
	dae::GameObject* m_OwnerPtr{};

	bool m_IsMarkedForDeletion{ false };
};
