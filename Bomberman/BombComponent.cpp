//---------------------------
// Include Files
//---------------------------
#include "BombComponent.h"

#include "EngineTime.h"
#include "ExplosionComponent.h"
#include "GameObject.h"
#include "GridComponent.h"
#include "LevelManager.h"
#include "PlayerComponent.h"
#include "ServiceLocator.h"

//---------------------------
// Constructor & Destructor
//---------------------------

BombComponent::BombComponent(dae::GameObject& owner, int range) : Component(owner), m_Timer(0.f), m_ExplosionRange(range)
{
	LevelManager::GetInstance().GetExplosionSystem()->AddObserver(this);
}

//---------------------------
// Member functions
//---------------------------

void BombComponent::Update()
{
	if (IsMarkedForDeletion()) return;

	if (m_Exploded)
	{
		MarkForDeletion();
		if (auto* owner = GetOwner())
		{
			owner->MarkForDeletion();
		}
		return;
	}

	const auto player1 = LevelManager::GetInstance().GetPlayer(0);

	if (player1->HasRemote()) return;

	const float deltaTime = EngineTime::GetInstance().GetDeltaTime();

	m_Timer += deltaTime;

	if (m_Timer >= BOMB_EXPLOSION_TIME && !m_Exploded)
	{
		Explode();
	}
}

void BombComponent::Notify(const Event& event, dae::GameObject* /*actor*/)
{
	if (event.id == make_sdbm_hash("Explosion"))
	{
		if (m_Exploded || IsMarkedForDeletion()) return;

		TileCoords explosionPos{};
		if (!EventArg::TryGetEventArg(event.args[0], explosionPos)) return;

		const auto& gridPos = LevelManager::GetInstance().GetGrid()->GetTileFromWorldPosition(GetOwner()->GetPosition());

		if (explosionPos == gridPos)
		{
			Explode(); // explode early if in other bombs explosion
		}
	}
}

void BombComponent::Explode()
{
	m_Exploded = true;

	ServiceLocator::GetSoundSystem().QueueSound("Resources/Audio/BombExplodes.wav");

	auto grid = LevelManager::GetInstance().GetGrid();

	const TileCoords centerExplosion = grid->GetTileFromWorldPosition(GetOwner()->GetPosition());

	auto explosionSystem = LevelManager::GetInstance().GetExplosionSystem();

	explosionSystem->RemoveObserver(this);

	explosionSystem->TriggerExplosion(centerExplosion, m_ExplosionRange);

	const TileCoords directions[] = {
	{ 0, -1 }, // up
	{ 0, +1 }, // down
	{ -1, 0 }, // left
	{ +1, 0 }  // right
	};

	for (const auto& dir : directions)
	{
		for (int i = 1; i <= m_ExplosionRange; ++i)
		{
			const TileCoords explosionTile = centerExplosion + dir * i;

			if (grid->GetTile(explosionTile) == TileType::IndestructibleWall) break; // if explosion hits indestructible wall stop in that direction

			if (grid->BreakTile(explosionTile)) break; // if explosion hits wall destructible stop in that direction
		}
	}
}
