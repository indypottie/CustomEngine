//---------------------------
// Include Files
//---------------------------
#include "GameCommands.h"

#include "EnemyComponent.h"
#include "EngineTime.h"
#include "HealthComponent.h"
#include "PlayerComponent.h"
#include "Scene.h"
#include "SceneManager.h"
#include "ServiceLocator.h"
#include "SpriteAnimatorComponent.h"

BombermanDropBombCommand::BombermanDropBombCommand(dae::GameObject& gameObject) : GameObjectCommand(gameObject)
{
	if (PlayerComponent* playerComponent = GetGameObject()->GetComponent<PlayerComponent>())
	{
		m_PlayerComponentPtr = playerComponent;
	}
}

void BombermanDropBombCommand::Execute()
{
	if (!m_PlayerComponentPtr) return;

	m_PlayerComponentPtr->DropBomb();
}

BombermanDetonateBombCommand::BombermanDetonateBombCommand(dae::GameObject& gameObject) : GameObjectCommand(gameObject)
{
	if (PlayerComponent* playerComponent = GetGameObject()->GetComponent<PlayerComponent>())
	{
		m_PlayerComponentPtr = playerComponent;
	}
}

void BombermanDetonateBombCommand::Execute()
{
	if (!m_PlayerComponentPtr) return;

	m_PlayerComponentPtr->ExplodeOldestBomb();
}


// movement command

BombermanMoveCommand::BombermanMoveCommand(dae::GameObject& gameObject, Direction direction, int playerIndex) :
	GameObjectCommand(gameObject),
	m_Direction(direction),
	m_PlayerIndex(playerIndex)
{
	if (PlayerComponent* playerComponent = GetGameObject()->GetComponent<PlayerComponent>())
	{
		m_PlayerComponentPtr = playerComponent;
	}
}

void BombermanMoveCommand::Execute()
{
	if (!IsMyPlayer()) return;

	float deltaTime = EngineTime::GetInstance().GetDeltaTime();

	m_LatestCollisionInfo = m_PlayerComponentPtr->GetLatestCollisionInfo();

	switch (m_Direction)
	{
	case Direction::Up:		MoveUp(deltaTime);		break;
	case Direction::Down:	MoveDown(deltaTime);	break;
	case Direction::Left:	MoveLeft(deltaTime);	break;
	case Direction::Right:	MoveRight(deltaTime);	break;
	case Direction::None:	StopMovement();			break;
	}
}

void BombermanMoveCommand::MoveUp(float deltaTime)		const
{
	if (!m_LatestCollisionInfo.blockUp) // Only block if vertical (Y) is blocked
	{
		auto velY = m_Speed * deltaTime;

		Move(0, -velY);

		PlayAnim("WalkUp");
	}
}

void BombermanMoveCommand::MoveDown(float deltaTime) const
{
	if (!m_LatestCollisionInfo.blockDown)
	{
		auto velY = m_Speed * deltaTime;

		Move(0, +velY);

		PlayAnim("WalkDown");
	}
}

void BombermanMoveCommand::MoveLeft(float deltaTime) const
{
	if (!m_LatestCollisionInfo.blockLeft)
	{
		auto velX = m_Speed * deltaTime;

		Move(-velX, 0);

		PlayAnim("WalkLeft");
	}
}

void BombermanMoveCommand::MoveRight(float deltaTime) const
{
	if (!m_LatestCollisionInfo.blockRight)
	{
		auto velX = m_Speed * deltaTime;

		Move(+velX, 0);

		PlayAnim("WalkRight");
	}
}

void BombermanMoveCommand::Move(float dx, float dy) const
{
	auto pos = GetGameObject()->GetPosition();
	GetGameObject()->SetPosition(pos.x + dx, pos.y + dy);

	m_StepCooldown -= EngineTime::GetInstance().GetDeltaTime();

	if (m_StepCooldown <= 0.f)
	{
		ServiceLocator::GetSoundSystem().QueueSound("../Bomberman/Resources/Audio/BombermanStep.wav");
		m_StepCooldown = STEP_INTERVAL;
	}
}

void BombermanMoveCommand::PlayAnim(const std::string& animName) const
{
	if (auto animator = GetGameObject()->GetComponent<SpriteAnimatorComponent>())
		animator->Play(animName);
}

void BombermanMoveCommand::StopMovement() const
{
	if (auto player = GetGameObject()->GetComponent<PlayerComponent>())
	{
		player->SetState(std::make_unique<IdleState>());
	}
}


bool BombermanMoveCommand::IsMyPlayer() const
{
	if (auto* pc = GetGameObject()->GetComponent<PlayerComponent>())
	{
		return pc->GetPlayerIndex() == m_PlayerIndex;
	}

	return false;
}
