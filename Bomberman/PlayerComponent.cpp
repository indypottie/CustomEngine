//---------------------------
// Include Files
//---------------------------
#include "PlayerComponent.h"

#include <iostream>

#include "BombComponent.h"
#include "CollisionSystem.h"
#include "ExplosionComponent.h"
#include "GameObject.h"
#include "GridComponent.h"
#include "LevelManager.h"
#include "PrefabFactory.h"
#include "Scene.h"
#include "SceneManager.h"
#include "ServiceLocator.h"

//---------------------------
// Constructor & Destructor
//---------------------------

//---------------------------
// Member functions
//---------------------------

PlayerComponent::PlayerComponent(dae::GameObject& owner, int playerIdx) : Component(owner),
	m_PlayerIdx(playerIdx),
	m_MaxBombs(1),
	m_CurrentBombs(0),
	m_ExplosionRange(1),
	m_HasRemote(false),
	m_Lives(3)
{
	m_PlayerStateMachinePtr = std::make_unique<StateMachine<dae::GameObject, IPlayerState>>();

	LevelManager::GetInstance().GetExplosionSystem()->AddObserver(this);
}

void PlayerComponent::Update()
{
	ResetCollisionInfo();

	m_PlayerStateMachinePtr->Update(*GetOwner());
}

void PlayerComponent::SetState(std::unique_ptr<IPlayerState> newState)
{
	m_PlayerStateMachinePtr->SetState(std::move(newState), *GetOwner());
}

void PlayerComponent::ResetCollisionInfo()
{
	m_LatestCollisionInfo = CollisionInfo{};
}

void PlayerComponent::DropBomb()
{
	if (m_IsBalloom) return;
	if (m_CurrentBombs >= m_MaxBombs) return;

	const auto playerPos = GetOwner()->GetPosition();

	const auto& grid = LevelManager::GetInstance().GetGrid();

	auto currentTile = grid->GetTileFromWorldPosition(playerPos);
	auto tileType = grid->GetTile(currentTile);

	if (tileType == TileType::ExitBlock)
	{
		LevelManager::GetInstance().TryAdvanceLevel();
	}

	auto bomb = Prefab::CreateBomb(playerPos, m_ExplosionRange, m_PlayerIdx);
	if (!bomb)
	{
		std::cout << "BOMB CREATION FAILED\n";
		__debugbreak();
	}

	if (m_HasRemote) // only store bombs if the player has a remote 
	{
		m_ActiveBombs.emplace(bomb->GetComponent<BombComponent>());
	}

	dae::SceneManager::GetInstance().GetActiveScene()->Add(std::move(bomb));
	++m_CurrentBombs;
}

void PlayerComponent::ExplodeOldestBomb()
{
	while (!m_ActiveBombs.empty())
	{
		BombComponent* bomb = m_ActiveBombs.front();
		m_ActiveBombs.pop();

		// Check if bomb is still valid and hasn't already exploded
		if (bomb && !bomb->HasExploded())
		{
			bomb->Explode();
			break;
		}
		// Else: skip to next entry in the queue
	}
}

void PlayerComponent::Notify(const Event& event, dae::GameObject* /*actor*/)
{
	if (event.id == make_sdbm_hash("Explosion"))
	{
		TileCoords explosionPos{};
		if (!EventArg::TryGetEventArg(event.args[0], explosionPos)) return;

		const auto& gridPos = LevelManager::GetInstance().GetGrid()->GetTileFromWorldPosition(GetOwner()->GetPosition());

		if (explosionPos == gridPos)
		{
			--m_Lives;

			if (m_Lives <= 0)
			{
				OnDeath();
			}
		}
	}
	else if (event.id == make_sdbm_hash("BombExploded"))
	{
		if (m_CurrentBombs <= 0) return;
		--m_CurrentBombs;
	}
}

void PlayerComponent::OnDeath()
{
	LevelManager::GetInstance().EndGame();
}

void PlayerComponent::OnCollision(const CollisionInfo& collisionInfo)
{
	if (m_IsBalloom) return;

	// Check if collided with an enemy
	if (collisionInfo.other && collisionInfo.other->GetTag() == "Enemy")
	{
		--m_Lives;

		if (m_Lives <= 0)
		{
			OnDeath();
		}
		return;
	}

	const auto& grid = LevelManager::GetInstance().GetGrid();

	const auto tile = grid->GetTileFromWorldPosition(collisionInfo.contactPoint);
	const auto typeOverlayLayer = grid->GetTile(tile, 1);


	// check if wall is broken above the power-up if not do normal collision handling 
	if (typeOverlayLayer == TileType::DestructibleWall)
	{
		m_LatestCollisionInfo = collisionInfo;
		return;
	}

	const auto& typeObjectLayer = grid->GetTile(tile, 0);

	if (GridComponent::IsObjectTile(typeObjectLayer))
	{
		switch (typeObjectLayer)  // NOLINT(clang-diagnostic-switch-enum)
		{
		case TileType::PowerUpExtraBomb:
			++m_MaxBombs;
			grid->SetTile(tile, TileType::Empty, 0);
			grid->SetTile(tile, TileType::Floor, 1);
			ServiceLocator::GetSoundSystem().QueueSound("../Bomberman/Resources/Audio/PowerUp.wav");
			break;

		case TileType::PowerUpDetonator:
			m_HasRemote = true;
			grid->SetTile(tile, TileType::Empty, 0);
			grid->SetTile(tile, TileType::Floor, 1);
			ServiceLocator::GetSoundSystem().QueueSound("../Bomberman/Resources/Audio/PowerUp.wav");
			break;

		case TileType::PowerUpFlames:
			++m_ExplosionRange;
			grid->SetTile(tile, TileType::Empty, 0);
			grid->SetTile(tile, TileType::Floor, 1);
			ServiceLocator::GetSoundSystem().QueueSound("../Bomberman/Resources/Audio/PowerUp.wav");
			break;

		case TileType::ExitBlock:
			grid->SetTile(tile, TileType::Empty, 0);
			grid->SetTile(tile, TileType::ExitBlock, 1);
			LevelManager::GetInstance().FoundExit();
			break;


		default:
			break;
		}

		ServiceLocator::GetCollisionSystem().UnregisterStaticCollider(tile.ToVec2(), true);
	}
}
