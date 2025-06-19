#pragma once


//-----------------------------------------------------
// Include Files
//-----------------------------------------------------
#include <memory>
#include <queue>

#include "CollisionInfo.h"
#include "Component.h"
#include "ICollisionResponder.h"
#include "Observer.h"
#include "States.h"
#include "StateMachine.h"

class BombComponent;
struct CollisionInfo;
//-----------------------------------------------------
// PlayerComponent Class									 
//-----------------------------------------------------
class PlayerComponent final : public Component, public ICollisionResponder, public Observer
{
public:
	 PlayerComponent(dae::GameObject& owner ,int playerIdx); // Constructor
	~PlayerComponent() override = default; // Destructor

	// -------------------------
	// Copy/move constructors and assignment operators
	// -------------------------    
	PlayerComponent(const PlayerComponent& other)					= delete;
	PlayerComponent(PlayerComponent&& other) noexcept				= delete;
	PlayerComponent& operator=(const PlayerComponent& other)		= delete;
	PlayerComponent& operator=(PlayerComponent&& other) noexcept	= delete;

	//-------------------------------------------------
	// Member functions						
	//-------------------------------------------------

	void Update() override;

	void SetState(std::unique_ptr<IPlayerState> newState);

	int GetPlayerIndex() const { return m_PlayerIdx; }


	void SetIsBalloom(bool val) { m_IsBalloom = val; }
	bool IsBalloom() const { return m_IsBalloom; }

	// collision
	void OnCollision(const CollisionInfo& collisionInfo) override;

	const CollisionInfo& GetLatestCollisionInfo() const { return m_LatestCollisionInfo; }
	void ResetCollisionInfo();

	// BOMB FUNCTIONS
	void DropBomb();
	void ExplodeOldestBomb();
	bool HasRemote() const { return m_HasRemote; }

	void Notify(const Event& event, dae::GameObject* actor) override;

	void OnDeath();

 private:
	//-------------------------------------------------
	// Private member functions								
	//-------------------------------------------------


	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
	int m_PlayerIdx{ 0 };

	CollisionInfo m_LatestCollisionInfo;

	std::unique_ptr<StateMachine<dae::GameObject, IPlayerState>> m_PlayerStateMachinePtr;

	std::queue<BombComponent*> m_ActiveBombs;

	int m_Lives;

	// BOMB RELATED DATA
	int m_MaxBombs;
	int m_CurrentBombs;
	int m_ExplosionRange;

	bool m_HasRemote;

	bool m_IsBalloom{ false };
};
