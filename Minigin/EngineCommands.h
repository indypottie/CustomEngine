// file to store all commands this is cleaner than creating a new file for each command 
#pragma once
#include "GameObjectCommand.h"



/// ----------------------------------
/// move command class -> this class will move the game object in a certain direction and will also handle movement speed
/// ----------------------------------
enum class Direction : uint8_t
{
    Up,
    Down,
    Left,
    Right,
    None
};

class MoveCommand final : public GameObjectCommand
{
public:
    MoveCommand(dae::GameObject& gameObject, Direction direction) : GameObjectCommand(gameObject), m_Direction(direction) {}

    void Execute() override;

    void SetSpeed(float speed) { m_Speed = speed; }
    float GetSpeed() const { return m_Speed; }

private:

    void MoveUp(float deltaTime) const;
    void MoveDown(float deltaTime) const;
	void MoveLeft(float deltaTime) const;
	void MoveRight(float deltaTime) const;

    void Move(float dx, float dy) const;

    Direction m_Direction;

    float m_Speed{ 1.f};
};