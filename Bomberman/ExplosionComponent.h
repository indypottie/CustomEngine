#pragma once
//-----------------------------------------------------
// Include Files
//-----------------------------------------------------
#include <SDL_rect.h>

#include "Component.h"
#include "GridComponent.h"


//-----------------------------------------------------
// ExplosionComponent Class									 
//-----------------------------------------------------

class Observer;
class Subject;

enum class ExplosionPartType : uint8_t
{
	Center,
	HorizontalMiddle,
	VerticalMiddle,
	EndLeft,
	EndRight,
	EndTop,
	EndBottom
};



struct ExplosionTile
{
	TileCoords coords; // coordinates of the tile
	float lifeTime{ 0.f }; // time the explosion has been active
	float frameTimer{ 0.f }; // timer for the current frame of the explosion animation
	int currentFrame{ 0 }; // current frame of the explosion animation
	SDL_Rect m_SourceRect{};
	bool isCenter{ false }; // whether this explosion tile is the center of the explosion

	ExplosionPartType type{};
};


class ExplosionComponent final : public Component
{
public:
	ExplosionComponent(dae::GameObject& owner); // Constructor
	~ExplosionComponent() override = default; // Destructor

	// -------------------------
	// Copy/move constructors and assignment operators
	// -------------------------    
	ExplosionComponent(const ExplosionComponent& other)						= delete;
	ExplosionComponent(ExplosionComponent&& other) noexcept					= delete;
	ExplosionComponent& operator=(const ExplosionComponent& other)			= delete;
	ExplosionComponent& operator=(ExplosionComponent&& other) noexcept		= delete;

	//-------------------------------------------------
	// Member functions						
	//-------------------------------------------------
	void Update() override;
	void Render() const override;

	void TriggerExplosion(const TileCoords& center, int range);

	void AddObserver(Observer* observer);
	void RemoveObserver(Observer* observer);

private:
	//-------------------------------------------------
	// Private member functions								
	//-------------------------------------------------
	void SpawnExplosionAt(const TileCoords& tileCoords, ExplosionPartType type);
	void UpdateExplosion(ExplosionTile& explosion, float deltaTime);
	void RemoveExpiredExplosions();

	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
	std::vector<ExplosionTile> m_Explosions; // vector of explosions

	float m_FrameDuration{ 0.1f }; // duration of each explosion frame
	float m_TotalLifetime{ 0.5f }; // total duration of explosion tile

	std::unique_ptr<dae::Texture2D> m_ExplosionTexture; // texture for the explosion animation
	int m_FramesPerTile{ 4 }; // number of frames per explosion tile

	std::unique_ptr<Subject> m_SubjectPtr;
};
