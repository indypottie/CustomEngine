//---------------------------
// Include Files
//---------------------------
#include "ExplosionComponent.h"

#include "EngineTime.h"
#include "Event.h"
#include "LevelManager.h"
#include "Renderer.h"
#include "Subject.h"
#include "Texture2D.h"

//---------------------------
// Constructor & Destructor
//---------------------------
ExplosionComponent::ExplosionComponent(dae::GameObject& owner) : Component(owner), m_SubjectPtr(std::make_unique<Subject>())
{
	m_ExplosionTexture = std::make_unique<dae::Texture2D>("Resources/Explosion.tga");
}

//---------------------------
// Member functions
//---------------------------
void ExplosionComponent::Update()
{
	const float deltaTime = EngineTime::GetInstance().GetDeltaTime();

	for (auto& explosion : m_Explosions)
	{
		UpdateExplosion(explosion, deltaTime);
	}

	RemoveExpiredExplosions();
}

void ExplosionComponent::Render() const
{
	if (!m_ExplosionTexture) return;

	for (const auto& explosion : m_Explosions)
	{
		const auto& grid = LevelManager::GetInstance().GetGrid();
		if (!grid) continue;

		const auto pos = grid->GetWorldPosition(explosion.coords);

		dae::Renderer::GetInstance().RenderTexture(*m_ExplosionTexture, pos.x, pos.y, explosion.m_SourceRect);
	}
}

void ExplosionComponent::TriggerExplosion(const TileCoords& center, int range)
{
	auto* grid = LevelManager::GetInstance().GetGrid();
	if (!grid) return;

	Event bombExplodedEvent(make_sdbm_hash("BombExploded"));
	m_SubjectPtr->NotifyObservers(bombExplodedEvent, GetOwner());

	// Spawn center tile
	SpawnExplosionAt(center, ExplosionPartType::Center);

	const TileCoords directions[]{ {1, 0}, {-1, 0}, {0, 1}, {0, -1} };

	for (const auto& dir : directions)
	{
		for (int i = 1; i <= range; ++i)
		{
			TileCoords target = center + dir * i;

			if (!grid->IsTileWalkable(target)) break;

			bool isLast = (i == range);
			ExplosionPartType type{};

			if (dir.col != 0) // Horizontal
			{
				if (isLast)
					type = (dir.col > 0) ? ExplosionPartType::EndRight : ExplosionPartType::EndLeft;
				else
					type = ExplosionPartType::HorizontalMiddle;
			}
			else if (dir.row != 0) // Vertical
			{
				if (isLast)
					type = (dir.row > 0) ? ExplosionPartType::EndBottom : ExplosionPartType::EndTop;
				else
					type = ExplosionPartType::VerticalMiddle;
			}

			SpawnExplosionAt(target, type);
		}
	}
}

void ExplosionComponent::AddObserver(Observer* observer)
{
	if (!m_SubjectPtr) return;

	m_SubjectPtr->AddObserver(observer);
}

void ExplosionComponent::RemoveObserver(Observer* observer)
{
	if (!m_SubjectPtr) return;

	m_SubjectPtr->RemoveObserver(observer);
}


void ExplosionComponent::SpawnExplosionAt(const TileCoords& tileCoords, ExplosionPartType type)
{
	ExplosionTile tile{};
	tile.coords = tileCoords;
	tile.type = type;
	tile.lifeTime = m_TotalLifetime;
	tile.frameTimer = m_FrameDuration;

	tile.currentFrame = 0;

	// Calculate source rect
	const int frameWidth = m_ExplosionTexture->GetSize().x / m_FramesPerTile;
	const int frameHeight = m_ExplosionTexture->GetSize().y / 7; // 7 parts (rows)

	tile.m_SourceRect = SDL_Rect{
		0, // Start at frame 0
		static_cast<int>(type) * frameHeight,
		frameWidth,
		frameHeight
	};

	m_Explosions.push_back(tile);

	Event explosionEvent(make_sdbm_hash("Explosion"), tileCoords);
	m_SubjectPtr->NotifyObservers(explosionEvent, this->GetOwner());
}



void ExplosionComponent::UpdateExplosion(ExplosionTile& explosion, float deltaTime)
{
	explosion.lifeTime -= deltaTime;
	explosion.frameTimer -= deltaTime;

	if (explosion.frameTimer <= 0.f)
	{
		explosion.frameTimer += m_FrameDuration;
		++explosion.currentFrame;

		if (explosion.currentFrame < m_FramesPerTile)
		{
			const int frameWidth = m_ExplosionTexture->GetSize().x / m_FramesPerTile;
			const int frameHeight = m_ExplosionTexture->GetSize().y / 7;

			explosion.m_SourceRect.x = explosion.currentFrame * frameWidth;
			explosion.m_SourceRect.y = static_cast<int>(explosion.type) * frameHeight;
			// width and height remain the same
		}
	}
}



void ExplosionComponent::RemoveExpiredExplosions()
{
	std::erase_if(m_Explosions,
	              [](const ExplosionTile& e) {return e.lifeTime <= 0.f; });
}
