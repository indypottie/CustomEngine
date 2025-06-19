// file to store all commands this is cleaner than creating a new file for each command 

#pragma once

#include "CollisionInfo.h"
#include "EngineCommands.h"
#include "GameObjectCommand.h"


class PlayerComponent;

// player action command
class BombermanDropBombCommand final : public GameObjectCommand
{
public:
	BombermanDropBombCommand(dae::GameObject& gameObject);

	void Execute() override;

private:

	PlayerComponent* m_PlayerComponentPtr{ nullptr };
};

class BombermanDetonateBombCommand final : public GameObjectCommand
{
public:
	BombermanDetonateBombCommand(dae::GameObject& gameObject);

	void Execute() override;

private:

	PlayerComponent* m_PlayerComponentPtr{ nullptr };
};

// movement command
class BombermanMoveCommand final : public GameObjectCommand
{
public:
	BombermanMoveCommand(dae::GameObject& gameObject, Direction direction, int playerIndex);

	void Execute() override;

	void SetPlayerSpeed(float speed) { m_Speed = speed; }
	float GetPlayerSpeed() const { return m_Speed; }

private:

	void MoveUp(float deltaTime) const;
	void MoveDown(float deltaTime) const;
	void MoveLeft(float deltaTime) const;
	void MoveRight(float deltaTime) const;

	void StopMovement() const;

	void Move(float dx, float dy) const;
	void PlayAnim(const std::string& animName) const;

	bool IsMyPlayer() const;

	Direction m_Direction;

	float m_Speed{ 150.f };

	int m_PlayerIndex{ -1 };

	PlayerComponent* m_PlayerComponentPtr{ nullptr };
	CollisionInfo m_LatestCollisionInfo{};

	mutable float m_StepCooldown{ 0.f };
	static constexpr float STEP_INTERVAL{ 0.3f }; // Play footstep every 0.3s
};