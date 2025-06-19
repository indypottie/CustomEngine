#pragma once
#include <vec2.hpp>
#include <vec3.hpp>
#include <vector>

#include "GridComponent.h"


class EnemyComponent;

namespace dae
{
	class GameObject;
}

// base player state interface class
class IPlayerState
{
public:
	virtual ~IPlayerState() = default;

	virtual void Enter(dae::GameObject& obj) = 0;
	virtual void Update(dae::GameObject& obj) = 0;
	virtual void Exit(dae::GameObject& obj) = 0;
};


class IdleState : public IPlayerState
{
	void Enter(dae::GameObject& obj) override;
	void Update(dae::GameObject& obj) override;
	void Exit(dae::GameObject& obj) override;
};




/// base enemy state interface class
class IEnemyState
{
public:
	IEnemyState(EnemyComponent& owningEnemy) { m_EnemyPtr = &owningEnemy; }
	virtual ~IEnemyState() = default;

	virtual void Enter(dae::GameObject& obj) = 0;
	virtual void Update(dae::GameObject& obj) = 0;
	virtual void Exit(dae::GameObject& obj) = 0;

protected:
	EnemyComponent* GetOwningEnemy() const { return m_EnemyPtr; }

private:
	EnemyComponent* m_EnemyPtr = nullptr;
};

// wander
class WanderState final : public IEnemyState
{
public:
	explicit WanderState(EnemyComponent& owningEnemy) : IEnemyState(owningEnemy) {}

	void Enter(dae::GameObject& obj) override;
	void Update(dae::GameObject& obj) override;
	void Exit(dae::GameObject& obj) override;

private:

	void ChooseNewDirection(const TileCoords& coords);
	float GetRandomTurnCooldown() const;

	std::vector<glm::ivec2> m_Directions = {
		{1, 0}, { -1, 0 }, {0, 1}, {0, -1}
	};


	glm::ivec2 m_CurrentDirection{ 0, 0 };
	glm::ivec2 m_PreviousDirection{ 0, 0 };

	float m_TurnCooldownMin = 1.0f;			// minimum seconds before turning
	float m_TurnCooldownMax = 3.0f;			// maximum seconds before turning
	float m_TurnCooldown = 1.f;
	float m_TurnTimer = 0.0f;				// Timer tracking how long since last turn
};

// ChaseState
class ChaseState : public IEnemyState
{
public:
	explicit ChaseState(EnemyComponent& enemy) : IEnemyState(enemy), m_TargetTile(), m_CurrentTile() {}

	void Enter(dae::GameObject& obj) override;
	void Update(dae::GameObject& obj) override;
	void Exit(dae::GameObject& obj) override;

private:

	glm::ivec2 m_TargetTile;        // Tile position of player to chase
	glm::ivec2 m_CurrentTile;       // Current tile of the enemy
	glm::ivec2 m_LastKnownPlayerTile = glm::ivec2(-1, -1);

	void MoveTowardsTarget(const dae::GameObject& obj, const glm::ivec2& targetTile, float deltaTime) const;
	static glm::ivec2 GetTileFromPosition(const glm::vec2& pos);
};

// dead
class DeadState final : public IEnemyState
{
public:
	explicit DeadState(EnemyComponent& owningEnemy) : IEnemyState(owningEnemy) {}

private:
	void Enter(dae::GameObject& obj) override;
	void Update(dae::GameObject& obj) override;
	void Exit(dae::GameObject& obj) override;
};