#pragma once

class ColliderComponent;

// interface for components that respond to collisions
class ICollisionResponder
{
public:
	virtual ~ICollisionResponder() = default;
	virtual void OnCollision(const CollisionInfo& collisionInfo) = 0;
};
