#include "GameObject.h"

#include <iostream>
#include <ranges>

#include "ColliderComponent.h"
#include "ICollisionResponder.h"
#include "ResourceManager.h"
#include "TransformComponent.h"

dae::GameObject::GameObject()
{
	AddComponent<TransformComponent>();

	m_TransformComponent = GetComponent<TransformComponent>();
}

dae::GameObject::~GameObject() = default;

void dae::GameObject::Update()
{
	for (auto& component : m_Components | std::views::values)
	{
		if (!component->IsMarkedForDeletion())
		{
			component->Update();
		}
	}

	CleanupComponents();
}

void dae::GameObject::Render() const
{
	// Gather all components
	std::vector<Component*> sortedComponents;
	sortedComponents.reserve(m_Components.size());

	for (const auto& comp : m_Components | std::views::values)
		sortedComponents.push_back(comp.get());

	// sort by render priority
	std::ranges::sort(sortedComponents, [](const auto* a, const auto* b)
		{
			return a->GetRenderPriority() < b->GetRenderPriority();
		});

	// Render in sorted order
	for (const auto* comp : sortedComponents)
		comp->Render();
}

void dae::GameObject::SetPosition(float x, float y) const
{
	auto position = glm::vec3(x, y, 0.0f);

	m_TransformComponent->SetLocalPosition(position);
}

glm::vec2 dae::GameObject::GetPosition() const
{
	return m_TransformComponent->GetLocalPosition();
}

void dae::GameObject::SetParent(GameObject* parent, bool keepWorldPosition)
{
	if (parent == m_ParentPtr or parent == nullptr) return;

	if (parent == nullptr)
	{
		m_TransformComponent->SetLocalPosition(m_TransformComponent->GetWorldPosition());
	}
	else
	{
		if (keepWorldPosition)
		{
			m_TransformComponent->SetLocalPosition(m_TransformComponent->GetWorldPosition() - parent->GetTransformComponent()->GetWorldPosition());
		}
		m_TransformComponent->MarkDirty();
	}

	if (m_ParentPtr) m_ParentPtr->RemoveChild(this);
	m_ParentPtr = parent;
	if (m_ParentPtr) m_ParentPtr->AddChild(this);
}

dae::GameObject* dae::GameObject::GetParent() const
{
	return m_ParentPtr;
}

std::vector<dae::GameObject*> dae::GameObject::GetChildren() const
{
	return m_ChildrenPtrs;
}

TransformComponent* dae::GameObject::GetTransformComponent() const
{
	return m_TransformComponent;
}

int dae::GameObject::GetChildCount() const
{
	return static_cast<int>(m_ChildrenPtrs.size());
}

dae::GameObject* dae::GameObject::GetChildAt(size_t index) const
{
	// check if the index is within bounds
	if (index < m_ChildrenPtrs.size())
	{
		return m_ChildrenPtrs[index];
	}
	return nullptr; // return nullptr if the index is out of bounds
}

void dae::GameObject::HandleCollision(const CollisionInfo& collisionInfo)
{
	// for all components that implement ICollisionResponder, OnCollision will be called
	for (auto& component : m_Components | std::views::values)
	{
		if (auto* responder = dynamic_cast<ICollisionResponder*>(component.get()))
		{
			responder->OnCollision(collisionInfo);
		}
	}
}

void dae::GameObject::RemoveChild(GameObject* child)
{
	std::erase(m_ChildrenPtrs, child);
}

void dae::GameObject::AddChild(GameObject* child)
{
	m_ChildrenPtrs.emplace_back(child);
}

void dae::GameObject::MarkForDeletion()
{
	m_IsMarkedForDeletion = true;
}

bool dae::GameObject::IsMarkedForDeletion() const
{
	return m_IsMarkedForDeletion;
}

void dae::GameObject::CleanupComponents()
{
	for (auto it = m_Components.begin(); it != m_Components.end();)
	{
		if (it->second->IsMarkedForDeletion())
		{
			it = m_Components.erase(it);
		}
		else
		{
			++it;
		}
	}
}
