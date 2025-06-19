//---------------------------
// Include Files
//---------------------------
#include "Component.h"

//---------------------------
// Constructor & Destructor
//---------------------------
Component::Component(dae::GameObject& owner)
	: m_OwnerPtr(&owner)
{
}

//---------------------------
// Member functions
//---------------------------

void Component::Update()
{
}

void Component::Render() const
{
}

dae::GameObject* Component::GetOwner() const
{
	if (m_OwnerPtr) return m_OwnerPtr;
	return nullptr;
}

