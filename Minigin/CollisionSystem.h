#pragma once

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------
#include <memory>
#include <vector>

#include "ColliderComponent.h"
#include "Subject.h"

struct StaticCollider
{
	SDL_FRect rect;
	glm::ivec2 coords;
	bool pickup_able;
};

//-----------------------------------------------------
// CollisionSystem Class									 
//-----------------------------------------------------
class CollisionSystem 
{
public:
	CollisionSystem(); // Constructor
	virtual ~CollisionSystem() = default; // Destructor

	// -------------------------
	// Copy/move constructors and assignment operators
	// -------------------------    
	CollisionSystem(const CollisionSystem& other)					= delete;
	CollisionSystem(CollisionSystem&& other) noexcept				= delete;
	CollisionSystem& operator=(const CollisionSystem& other)		= delete;
	CollisionSystem& operator=(CollisionSystem&& other) noexcept	= delete;

	//-------------------------------------------------
	// Member functions						
	//-------------------------------------------------
	virtual void RegisterCollider(ColliderComponent& collider);
	virtual void UnregisterCollider(ColliderComponent& collider);

	virtual void RegisterStaticCollider(const SDL_FRect& rect, glm::ivec2 coords, bool pickupAble);
	virtual void UnregisterStaticCollider(glm::ivec2 coords, bool allowPickupUnregister);

	virtual void Update();
	virtual void DebugRender();

	virtual void ClearColliders();

private:
	//-------------------------------------------------
	// Private member functions								
	//-------------------------------------------------
	void AddObserver(Observer* observer);
	void RemoveObserver(Observer* observer);

	static bool ResolveOverlap(const dae::GameObject* object, const SDL_FRect& overlap, const SDL_FRect& self, const SDL_FRect& other, bool pickUp);

	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------

	std::unique_ptr<Subject> m_SubjectPtr;

	std::vector<ColliderComponent*> m_Colliders;
	std::vector<StaticCollider> m_StaticColliders;

	// debug datamembers
	SDL_FRect m_LastCollisionRect{ -1, -1, -1, -1 };
};


// null collision system class
class NullCollisionSystem final : public CollisionSystem
{
public:
	NullCollisionSystem() = default;
	~NullCollisionSystem() override = default;

	void RegisterCollider(ColliderComponent& /*collider*/) override {}
	void UnregisterCollider(ColliderComponent& /*collider*/) override {}
	void Update() override {}
	void DebugRender() override {}
	void RegisterStaticCollider(const SDL_FRect& /*rect*/, glm::ivec2 /*coords*/, bool /*pickupAble*/) override {}
	void UnregisterStaticCollider(glm::ivec2 /*coords*/, bool /*allowPickupUnregister*/) override {}
	void ClearColliders() override {}
};