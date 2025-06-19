//---------------------------
// Include Files
//---------------------------
#include "TransformComponent.h"

#include "GameObject.h"

//---------------------------
// Constructor & Destructor
//---------------------------
TransformComponent::TransformComponent(dae::GameObject& owner) : Component(owner), m_LocalPosition(0.0f), m_WorldPosition(0.0f)
{
}

//---------------------------
// Member functions
//---------------------------

void TransformComponent::SetLocalPosition(const glm::vec2& pos)
{
	m_LocalPosition = pos;
	MarkDirty();
}

void TransformComponent::SetWorldPosition(const glm::vec2& pos)
{
	m_WorldPosition = pos;
	m_LocalPosition = m_WorldPosition; // if no parent, local = world
	MarkDirty();
}

void TransformComponent::MarkDirty()
{
	m_IsDirty = true;

	// mark all children as dirty
	for (auto& child : GetOwner()->GetChildren())
	{
		if (auto transform = child->GetTransformComponent())
		{
			transform->MarkDirty();
		}
	}
}

const glm::vec2& TransformComponent::GetWorldPosition()
{
	if (m_IsDirty)
	{
		UpdateWorldTransform();
	}
	return m_WorldPosition;
}

void TransformComponent::UpdateWorldTransform()
{
	auto parent = GetOwner()->GetParent();
	if (parent != nullptr)
	{
		if (const auto parentTransform = parent->GetTransformComponent())
		{
			m_WorldPosition = parentTransform->GetWorldPosition() + m_LocalPosition;
		}
	}
	else
	{
		m_WorldPosition = m_LocalPosition;
	}

	m_IsDirty = false;
}
