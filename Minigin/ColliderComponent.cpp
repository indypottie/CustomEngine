//---------------------------
// Include Files
//---------------------------
#include "ColliderComponent.h"

#include <iostream>

#include "CollisionSystem.h"
#include "GameObject.h"
#include "Renderer.h"
#include "ServiceLocator.h"
#include "../Bomberman/PlayerComponent.h"


//---------------------------
// Constructor & Destructor
//---------------------------

ColliderComponent::ColliderComponent(dae::GameObject& owner, SDL_FRect collisionRect) : Component(owner), m_CollisionRect(collisionRect)
{
	// Update collision rect position to owner's current position
	const auto& ownerPos = GetOwner()->GetPosition();
	m_CollisionRect.x = ownerPos.x;
	m_CollisionRect.y = ownerPos.y;
}

ColliderComponent::~ColliderComponent()
{
	ServiceLocator::GetCollisionSystem().UnregisterCollider(*this);
}

//---------------------------
// Member functions
//---------------------------

void ColliderComponent::Update()
{
	if (!m_Registered)
	{
		ServiceLocator::GetCollisionSystem().RegisterCollider(*this);
		m_Registered = true;
	}

	const auto& ownerTransform = GetOwner()->GetPosition();

	m_CollisionRect.x = ownerTransform.x;
	m_CollisionRect.y = ownerTransform.y;
}

void ColliderComponent::Render() const
{
#ifdef _DEBUG
	SDL_FRect rect = m_CollisionRect;

	const float outlineOffset = 2.f;

	// Offset the box outward by 2 pixels on all sides
	rect.x -= outlineOffset;
	rect.y -= outlineOffset;
	rect.w += outlineOffset * 2;
	rect.h += outlineOffset * 2;

	SDL_Renderer* renderer = dae::Renderer::GetInstance().GetSDLRenderer();
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red outline
	SDL_RenderDrawRectF(renderer, &rect);
#endif
}

void ColliderComponent::Notify(const Event& event, dae::GameObject* actor)
{
	if (event.id == make_sdbm_hash("Collision") && GetOwner() == actor)
	{
		const auto& collisionInfo = event.args[0].Get<CollisionInfo>(); // extract the collision position
		actor->HandleCollision(collisionInfo);
	}
}
