//---------------------------
// Include Files
//---------------------------
#include "EngineCommands.h"

#include "EngineTime.h"


/// ----------------------------------
/// engine commands -- implementation
/// ----------------------------------


void MoveCommand::Execute()
{
	const float deltaTime = EngineTime::GetInstance().GetDeltaTime();

	switch (m_Direction)
	{
	case Direction::Up:
		MoveUp(deltaTime);
		break;

	case Direction::Down:
		MoveDown(deltaTime);
		break;

	case Direction::Left:
		MoveLeft(deltaTime);
		break;

	case Direction::Right:
		MoveRight(deltaTime);
		break;

	case Direction::None:
		break;
	}
}

void MoveCommand::MoveUp(float deltaTime) const
{
	auto velY = m_Speed * deltaTime;
	Move(0.f, -velY);
}

void MoveCommand::MoveDown(float deltaTime) const
{
	auto velY = m_Speed * deltaTime;
	Move(0.f, +velY);
}

void MoveCommand::MoveLeft(float deltaTime) const
{
	auto velX = m_Speed * deltaTime;
	Move(-velX, 0.f);
}

void MoveCommand::MoveRight(float deltaTime) const
{
	auto velX = m_Speed * deltaTime;
	Move(+velX, 0.f);
}

void MoveCommand::Move(float dx, float dy) const
{
	auto pos = GetGameObject()->GetPosition();
	GetGameObject()->SetPosition(pos.x + dx, pos.y + dy);
}
