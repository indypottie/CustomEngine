#pragma once
#include <SDL_rect.h>
#include <vec2.hpp>


namespace dae
{
	class GameObject;
}

struct CollisionInfo
{
	dae::GameObject* other = nullptr;
	glm::vec2 contactPoint{};
	SDL_FRect overlapRect{};

	bool blockLeft = false;
	bool blockRight = false;
	bool blockUp = false;
	bool blockDown = false;
};

inline void DetermineBlockedDirections(const SDL_FRect& self, const SDL_FRect& other, const SDL_FRect& overlap, CollisionInfo& info)
{
	float dx = (self.x + self.w / 2.0f) - (other.x + other.w / 2.0f);
	float dy = (self.y + self.h / 2.0f) - (other.y + other.h / 2.0f);

	// Horizontal collision
	if (overlap.w > 0.f && overlap.w <= overlap.h)
	{
		if (dx > 0) info.blockLeft = true;  // hit something to our left
		else        info.blockRight = true; // hit something to our right
	}

	// Vertical collision
	if (overlap.h > 0.f && overlap.h <= overlap.w)
	{
		if (dy > 0) info.blockUp = true;    // hit something above
		else        info.blockDown = true;  // hit something below
	}
}