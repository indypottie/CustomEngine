//---------------------------
// Include Files
//---------------------------
#include "CollisionSystem.h"
#include <algorithm>
#include <glm.hpp>

#include "ColliderComponent.h"
#include "CollisionInfo.h"
#include "GameObject.h"
#include "Renderer.h"
#include "Subject.h"
#include "../Bomberman/PlayerComponent.h"

//---------------------------
// Constructor & Destructor
//---------------------------
CollisionSystem::CollisionSystem() : m_SubjectPtr(std::make_unique<Subject>())
{
}

//---------------------------
// Member functions
//---------------------------
void CollisionSystem::RegisterCollider(ColliderComponent& collider)
{
	m_Colliders.push_back(&collider);
	AddObserver(&collider);
}

void CollisionSystem::UnregisterCollider(ColliderComponent& collider)
{
	RemoveObserver(&collider);
	auto it = std::ranges::remove(m_Colliders, &collider);
}

void CollisionSystem::RegisterStaticCollider(const SDL_FRect& rect , glm::ivec2 coords, bool pickupAble)
{
	m_StaticColliders.push_back({ rect, coords, pickupAble });
}

void CollisionSystem::UnregisterStaticCollider(glm::ivec2 coords, bool allowPickupUnregister)
{
	const auto it = std::ranges::remove_if(m_StaticColliders, [=](const StaticCollider& collider)
		{
			if (collider.coords != coords) return false;

			// Check if it should be removed
			const bool shouldRemove = (collider.pickup_able && allowPickupUnregister) ||
				(!collider.pickup_able && !allowPickupUnregister);

			return shouldRemove;
		}).begin();

	m_StaticColliders.erase(it, m_StaticColliders.end());
}

void CollisionSystem::Update()
{
	// dynamic vs static
	for (auto* dynamic : m_Colliders)
	{
		SDL_FRect dynamicRect = dynamic->GetCollisionRect();

		for (const auto& staticCollider : m_StaticColliders)
		{
			SDL_FRect resultRect{};

			if (SDL_IntersectFRect(&dynamicRect, &staticCollider.rect, &resultRect))
			{
				m_LastCollisionRect = resultRect;

				CollisionInfo info{};
				info.other = nullptr;
				info.contactPoint = { resultRect.x, resultRect.y };
				info.overlapRect = resultRect;

				DetermineBlockedDirections(dynamicRect, staticCollider.rect, resultRect, info);

				// only send event if collision was not just sliding
				if (ResolveOverlap(dynamic->GetOwner(), resultRect, dynamicRect, staticCollider.rect, staticCollider.pickup_able))
				{
					Event event(make_sdbm_hash("Collision"), info);
					m_SubjectPtr->NotifyObservers(event, dynamic->GetOwner());
				}
			}
		}
	}

	// dynamic vs dynamic
	for (size_t index = 0; index < m_Colliders.size(); ++index)
	{
		auto* colliderA = m_Colliders[index];
		SDL_FRect rectA = colliderA->GetCollisionRect();

		for (size_t indexB = index + 1; indexB < m_Colliders.size(); ++indexB)
		{
			auto* colliderB = m_Colliders[indexB];
			SDL_FRect rectB = colliderB->GetCollisionRect();

			SDL_FRect resultRect{};

			if (SDL_IntersectFRect(&rectA, &rectB, &resultRect))
			{
				m_LastCollisionRect = resultRect;

				glm::vec2 contactPoint{ resultRect.x, resultRect.y };

				CollisionInfo infoA{};
				infoA.other = colliderB->GetOwner();
				infoA.contactPoint = contactPoint;
				infoA.overlapRect = resultRect;
				
				DetermineBlockedDirections(rectA, rectB, resultRect, infoA);

				CollisionInfo infoB{};
				infoB.other = colliderA->GetOwner();
				infoB.contactPoint = contactPoint;
				infoB.overlapRect = resultRect;

				DetermineBlockedDirections(rectB, rectA, resultRect, infoB);

				Event eventA(make_sdbm_hash("Collision"), infoA);
				Event eventB(make_sdbm_hash("Collision"), infoB);

				m_SubjectPtr->NotifyObservers(eventA, colliderA->GetOwner());
				m_SubjectPtr->NotifyObservers(eventB, colliderB->GetOwner());
			}
		}
	}
}


void CollisionSystem::DebugRender()
{
#ifdef _DEBUG

	for (const auto& collider : m_StaticColliders)
	{
		if (collider.pickup_able) SDL_SetRenderDrawColor(dae::Renderer::GetInstance().GetSDLRenderer(), 0, 255, 0, 255); // green
		else SDL_SetRenderDrawColor(dae::Renderer::GetInstance().GetSDLRenderer(), 255, 0, 0, 255); // Red outline

		SDL_RenderDrawRectF(dae::Renderer::GetInstance().GetSDLRenderer(), &collider.rect);
	}

	if (m_LastCollisionRect.w < 0 || m_LastCollisionRect.h < 0 || m_LastCollisionRect.x < 0 || m_LastCollisionRect.y < 0)
		return; // No valid collision rectangle to render

	SDL_SetRenderDrawColor(dae::Renderer::GetInstance().GetSDLRenderer(), 255, 0, 0, 255); // Red outline
	SDL_RenderDrawRectF(dae::Renderer::GetInstance().GetSDLRenderer(), &m_LastCollisionRect);

#endif
}

void CollisionSystem::ClearColliders()
{
	m_Colliders.clear();
	m_StaticColliders.clear();
}

void CollisionSystem::AddObserver(Observer* observer)
{
	if (m_SubjectPtr)
		m_SubjectPtr->AddObserver(observer);
}

void CollisionSystem::RemoveObserver(Observer* observer)
{
	if (m_SubjectPtr)
		m_SubjectPtr->RemoveObserver(observer);
}

// Returns true if it was a real collision (no slide or too small slide)
bool CollisionSystem::ResolveOverlap(const dae::GameObject* object, const SDL_FRect& overlap, const SDL_FRect& self, const SDL_FRect& other, bool pickUp)
{
	if (pickUp) return true;

	glm::vec2 resolve{ 0.f, 0.f };

	const float dx = (self.x + self.w * 0.5f) - (other.x + other.w * 0.5f);
	const float dy = (self.y + self.h * 0.5f) - (other.y + other.h * 0.5f);

	const float resolveX = (dx > 0) ? overlap.w : -overlap.w;
	const float resolveY = (dy > 0) ? overlap.h : -overlap.h;

	// === Tweakables ===
	constexpr float slideFactor = 0.15f;         
	constexpr float slideThreshold = 0.06f;
	constexpr float maxResolve = 4.0f;           // Max pixels we allow to move per frame from resolving
	constexpr float mapWidth = 992.f;          
	constexpr float mapHeight = 500.f;

	bool didSlide = false;

	if (overlap.w < overlap.h)
	{
		resolve.x = resolveX;
		const float ratio = overlap.w / overlap.h;
		if (ratio > slideThreshold)
		{
			resolve.y = resolveY * slideFactor;
			didSlide = true;
		}
	}
	else
	{
		resolve.y = resolveY;
		const float ratio = overlap.h / overlap.w;
		if (ratio > slideThreshold)
		{
			resolve.x = resolveX * slideFactor;
			didSlide = true;
		}
	}

	// Clamp the resolve to prevent large jumps
	resolve.x = std::clamp(resolve.x, -maxResolve, maxResolve);
	resolve.y = std::clamp(resolve.y, -maxResolve, maxResolve);

	// Apply resolution
	glm::vec2 newPos = object->GetPosition();
	newPos.x += resolve.x;
	newPos.y += resolve.y;

	// Prevent player from moving out of bounds
	newPos.x = std::clamp(newPos.x, 0.f, mapWidth - self.w);
	newPos.y = std::clamp(newPos.y, 0.f, mapHeight - self.h);

	object->SetPosition(newPos.x, newPos.y);

	// If sliding happened, consider it not a hard collision
	return !didSlide;
}
