//---------------------------
// Include Files
//---------------------------
#include "EnemyComponent.h"

#include "ExplosionComponent.h"
#include "GameObject.h"
#include "GridComponent.h"
#include "HudManager.h"
#include "LevelManager.h"
#include "Scene.h"
#include "SceneManager.h"

//---------------------------
// Constructor & Destructor
//---------------------------
EnemyComponent::EnemyComponent(dae::GameObject& owner, EnemyType enemyType) : Component(owner), m_Type(enemyType)
{
	InitializeEnemy();
}

EnemyComponent::~EnemyComponent()
{
}


//---------------------------
// Member functions
//---------------------------

void EnemyComponent::Update()
{
	m_EnemyStateMachinePtr->Update(*GetOwner());

	// Skip if this enemy type doesn't chase the player
	if (m_Type == EnemyType::Balloom || m_Type == EnemyType::Doll)
		return;

	// Check if the enemy has line of sight to the player
	if (HasLineOfSightToPlayer())
	{
		m_SeesPlayer = true;

		// If not already chasing, switch to chase state
		if (dynamic_cast<ChaseState*>(GetCurrentState()))
		{
			// Already in chase state, do nothing
			return;
		}

		SetState(std::make_unique<ChaseState>(*this));
	}
	else
	{
		m_SeesPlayer = false;
	}
}

void EnemyComponent::SetState(std::unique_ptr<IEnemyState> newState)
{
    m_EnemyStateMachinePtr->SetState(std::move(newState), *GetOwner());
}

void EnemyComponent::Notify(const Event& event, dae::GameObject* /*actor*/)
{
	if (event.id == make_sdbm_hash("Explosion"))
	{
		TileCoords explosionPos{};
		if (!EventArg::TryGetEventArg(event.args[0], explosionPos)) return;

		const auto& gridPos = LevelManager::GetInstance().GetGrid()->GetTileFromWorldPosition(GetOwner()->GetPosition());

		if (explosionPos == gridPos)
		{
			LevelManager::GetInstance().GetExplosionSystem()->RemoveObserver(this);
			LevelManager::GetInstance().AddPoints(m_PointsOnDeath);
			HudManager::ShowFloatingPoints(m_PointsOnDeath, GetOwner()->GetPosition());
			SetState(std::make_unique<DeadState>(*this));
		}
	}
}

void EnemyComponent::OnCollision(const CollisionInfo& collisionInfo)
{
	m_LatestCollisionInfo = collisionInfo;
}

void EnemyComponent::InitializeEnemy()
{
	m_EnemyStateMachinePtr = std::make_unique<StateMachine<dae::GameObject, IEnemyState>>();

	switch (m_Type)
	{
    case EnemyType::Balloom:
        m_Speed = 50.f;
		m_PointsOnDeath = 100;
        break;

    case EnemyType::Oneal:
        m_Speed = 60.f;
		m_PointsOnDeath = 200;
        break;

    case EnemyType::Doll:
        m_Speed = 60.f;
		m_PointsOnDeath = 400;
        break;

    case EnemyType::Minvo:
        m_Speed = 90.f;
		m_PointsOnDeath = 800;
        break;
	}

	LevelManager::GetInstance().GetExplosionSystem()->AddObserver(this);
	LevelManager::GetInstance().AddEnemy(*this);
    // set initial state
    m_EnemyStateMachinePtr->SetState(std::make_unique<WanderState>(*this), *GetOwner());
}

bool EnemyComponent::HasLineOfSightToPlayer()
{
	auto* grid = LevelManager::GetInstance().GetGrid();
	if (!grid) return false;

	if (!m_PlayerPtr)
	{
		m_PlayerPtr = dae::SceneManager::GetInstance().GetActiveScene()->FindObjectByTag("Player");
		if (!m_PlayerPtr) return false;
	}

	auto obj = GetOwner();

	TileCoords enemyCoords = grid->GetTileFromWorldPosition(obj->GetPosition());
	TileCoords playerCoords = grid->GetTileFromWorldPosition(m_PlayerPtr->GetPosition());

	if (enemyCoords.row == playerCoords.row)
	{
		int start = std::min(enemyCoords.col, playerCoords.col);
		int end = std::max(enemyCoords.col, playerCoords.col);
		for (int col = start + 1; col < end; ++col)
		{
			auto tile = grid->GetTile({ col, enemyCoords.row });
			if (tile == TileType::IndestructibleWall || tile == TileType::DestructibleWall)
				return false;
		}
		return true;
	}

	if (enemyCoords.col == playerCoords.col)
	{
		int start = std::min(enemyCoords.row, playerCoords.row);
		int end = std::max(enemyCoords.row, playerCoords.row);
		for (int row = start + 1; row < end; ++row)
		{
			auto tile = grid->GetTile({ enemyCoords.col, row });
			if (tile == TileType::IndestructibleWall || tile == TileType::DestructibleWall)
				return false;
		}
		return true;
	}

	return false; // Not in straight line
}
