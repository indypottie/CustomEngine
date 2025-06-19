#include "States.h"

#include <geometric.hpp>
#include <random>

#include "EnemyComponent.h"
#include "EngineTime.h"
#include "GameObject.h"
#include "GridComponent.h"
#include "LevelManager.h"
#include "SpriteAnimatorComponent.h"


////////////////////////////
/// PLAYER STATES
////////////////////////////

void IdleState::Enter(dae::GameObject& obj)
{
	if (auto animator = obj.GetComponent<SpriteAnimatorComponent>())
		animator->Stop();
}

void IdleState::Update(dae::GameObject& /*obj*/)
{
}

void IdleState::Exit(dae::GameObject& /*obj*/)
{
}


////////////////////////////
/// ENEMY STATES
////////////////////////////

void WanderState::Enter(dae::GameObject& obj)
{
    if (m_Directions.empty())
    {
        m_Directions = { { 1, 0 }, { -1, 0}, { 0, 1 }, { 0, -1 } };
    }

    m_TurnTimer = m_TurnCooldown;

    m_TurnCooldown = GetRandomTurnCooldown();

    if (auto* grid = LevelManager::GetInstance().GetGrid())
    {
        TileCoords currentTile = grid->GetTileFromWorldPosition(obj.GetPosition());
        glm::vec2 snapPos = grid->GetWorldPosition(currentTile);

        obj.SetPosition(snapPos.x, snapPos.y);
        ChooseNewDirection(currentTile);
    }
}

void WanderState::Update(dae::GameObject& /*obj*/)
{
    const float deltaTime = EngineTime::GetInstance().GetDeltaTime();

    EnemyComponent* pEnemy = GetOwningEnemy();
    if (!pEnemy)
        return;

    auto* grid = LevelManager::GetInstance().GetGrid();
    if (!grid)
        return;

    m_TurnTimer += deltaTime;

    const glm::vec2 currentPos = pEnemy->GetOwner()->GetPosition();
	const TileCoords currentTile = grid->GetTileFromWorldPosition(currentPos);
    const CollisionInfo& collisionInfo = pEnemy->GetLatestCollisionInfo();

    bool isBlocked = false;
    if (m_CurrentDirection.x < 0 && collisionInfo.blockLeft) isBlocked = true;
    if (m_CurrentDirection.x > 0 && collisionInfo.blockRight) isBlocked = true;
    if (m_CurrentDirection.y < 0 && collisionInfo.blockUp) isBlocked = true;
    if (m_CurrentDirection.y > 0 && collisionInfo.blockDown) isBlocked = true;

    // Choose new direction if blocked or cooldown expired
    if (isBlocked)
    {
        // Snap to grid to avoid drift and recenter
        glm::vec2 snapPos = grid->GetWorldPosition(currentTile);
        pEnemy->GetOwner()->SetPosition(snapPos.x, snapPos.y);

        ChooseNewDirection(currentTile);
    }
    else if (m_TurnTimer >= m_TurnCooldown)
    {
        // Snap to grid to avoid drift and recenter
        glm::vec2 snapPos = grid->GetWorldPosition(currentTile);
        pEnemy->GetOwner()->SetPosition(snapPos.x, snapPos.y);

        ChooseNewDirection(currentTile);

        m_TurnTimer = 0.0f;
        m_TurnCooldown = GetRandomTurnCooldown();
    }

    // Apply movement if a direction exists
    if (m_CurrentDirection != glm::ivec2{ 0, 0 })
    {
        const float speed = pEnemy->GetSpeed();
        glm::vec2 moveDelta = static_cast<glm::vec2>(m_CurrentDirection) * speed * deltaTime;
        glm::vec2 newPos = currentPos + moveDelta;

        pEnemy->GetOwner()->SetPosition(newPos.x, newPos.y);
    }
}

void WanderState::ChooseNewDirection(const TileCoords& coords)
{
    auto* grid = LevelManager::GetInstance().GetGrid();
    if (!grid) return;

    std::vector<glm::ivec2> validDirs;

    for (const auto& dir : m_Directions)
    {
        TileCoords testCoord = coords;
        testCoord.col += static_cast<int>(dir.x);
        testCoord.row += static_cast<int>(dir.y);

        if (testCoord.row >= 0 && testCoord.row < grid->GetRows() &&
            testCoord.col >= 0 && testCoord.col < grid->GetCols())
        {
            auto tile = grid->GetTile(testCoord);

            if (tile == TileType::Floor)
            {
                validDirs.emplace_back(dir);
            }
        }
    }

    // Static RNG for better randomness across multiple calls
    static std::mt19937 gen(std::random_device{}());

    if (!validDirs.empty())
    {
        // Filter out reverse direction (if any)
        std::vector<glm::ivec2> nonReverseDirs;
        for (const auto& dir : validDirs)
        {
            if (dir != -m_PreviousDirection)
                nonReverseDirs.push_back(dir);
        }

        glm::ivec2 chosenDir;

        if (!nonReverseDirs.empty())
        {
            // Pick a random direction from filtered list
            std::uniform_int_distribution<> distribution(0, static_cast<int>(nonReverseDirs.size()) - 1);
            chosenDir = nonReverseDirs[distribution(gen)];
        }
        else
        {
            // Only reverse is available
            std::uniform_int_distribution<> distribution(0, static_cast<int>(validDirs.size()) - 1);
            chosenDir = validDirs[distribution(gen)];
        }

        m_CurrentDirection = chosenDir;
        m_PreviousDirection = m_CurrentDirection;
    }
    else
    {
        m_CurrentDirection = {};
    }

    auto animator = GetOwningEnemy()->GetOwner()->GetComponent<SpriteAnimatorComponent>();

    if (m_CurrentDirection == glm::ivec2{ 1, 0 })
    {
        animator->Play("MoveRight");
    }
    else if (m_CurrentDirection == glm::ivec2{ -1, 0 })
    {
        animator->Play("MoveLeft");
    }
}

float WanderState::GetRandomTurnCooldown() const
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(m_TurnCooldownMin, m_TurnCooldownMax);

    return dis(gen);
}


void WanderState::Exit(dae::GameObject& /*obj*/)
{
    m_CurrentDirection = {}; // zero out direction
    m_TurnTimer = 0.0f;        //  reset timer
}


// chase
void ChaseState::Enter(dae::GameObject& obj)
{
    m_CurrentTile = GetTileFromPosition(obj.GetPosition());
}

void ChaseState::Update(dae::GameObject& obj)
{
    auto* enemy = GetOwningEnemy();
    if (!enemy) return;

    auto* player = enemy->GetPlayer();
    if (!player) return;

    m_CurrentTile = GetTileFromPosition(obj.GetPosition());

    if (enemy->SeesPlayer())
    {
        // Player is visible: update last known position and chase
        m_LastKnownPlayerTile = GetTileFromPosition(player->GetPosition());
        m_TargetTile = m_LastKnownPlayerTile;
    }
    else
    {
        // Player not visible: move to last known position
        if (m_LastKnownPlayerTile == glm::ivec2(-1, -1))
        {
            // No valid last known position, just wander
            enemy->SetState(std::make_unique<WanderState>(*enemy));
            return;
        }
        m_TargetTile = m_LastKnownPlayerTile;

        // If reached last known position and still no sight, wander
        if (m_CurrentTile == m_LastKnownPlayerTile)
        {
            enemy->SetState(std::make_unique<WanderState>(*enemy));
            return;
        }
    }

    float deltaTime = EngineTime::GetInstance().GetDeltaTime();
    MoveTowardsTarget(obj, m_TargetTile, deltaTime);
}

void ChaseState::Exit(dae::GameObject& /*obj*/)
{
    // Clean up if needed
}

void ChaseState::MoveTowardsTarget(const dae::GameObject& obj, const glm::ivec2& targetTile, float deltaTime) const
{
    auto* grid = LevelManager::GetInstance().GetGrid();
    if (!grid) return;

    glm::ivec2 currentTile = GetTileFromPosition(obj.GetPosition());
    glm::ivec2 delta = targetTile - currentTile;

    // Determine next tile step — move horizontally first, then vertically
    glm::ivec2 nextStep = currentTile;

    if (delta.x != 0)
        nextStep.x += (delta.x > 0) ? 1 : -1;
    else if (delta.y != 0)
        nextStep.y += (delta.y > 0) ? 1 : -1;

    // Check if next tile is walkable
    if (!grid->IsTileWalkable(nextStep))
    {
        // Optional: Try other axis if blocked
        if (delta.x != 0 && delta.y != 0)
        {
            // Try vertical if horizontal blocked
            glm::ivec2 altStep = currentTile;
            altStep.y += (delta.y > 0) ? 1 : -1;
            if (grid->IsTileWalkable(altStep))
                nextStep = altStep;
            else
                return; // stuck, no movement this frame
        }
        else
        {
            return; // stuck, no movement this frame
        }
    }

    // Get world position of the next tile (center)
    glm::vec2 targetWorldPos = grid->GetWorldPosition(nextStep);
    glm::vec2 currentPos = obj.GetPosition();

    // Move smoothly towards next tile center
    glm::vec2 direction = targetWorldPos - currentPos;
    float distance = glm::length(direction);
    if (distance < 0.001f)
    {
        // Snap to tile center if close enough to avoid jitter
        obj.SetPosition(targetWorldPos.x, targetWorldPos.y);
        return;
    }

    glm::vec2 moveDir = glm::normalize(direction);
    float speed = GetOwningEnemy()->GetSpeed();
    glm::vec2 moveDelta = moveDir * speed * deltaTime;

    // Clamp so we don't overshoot the tile center
    if (glm::length(moveDelta) > distance)
        moveDelta = direction;

    glm::vec2 newPos = currentPos + moveDelta;

    obj.SetPosition(newPos.x, newPos.y);
}

glm::ivec2 ChaseState::GetTileFromPosition(const glm::vec2& pos)
{
    auto* grid = LevelManager::GetInstance().GetGrid();
    if (!grid)
        return { -1, -1 };

    return grid->GetTileFromWorldPosition(pos);
}

// dead
void DeadState::Enter(dae::GameObject& obj)
{
    auto animator = obj.GetComponent<SpriteAnimatorComponent>();
    animator->Play("DeathAnimation");

}

void DeadState::Update(dae::GameObject& /*obj*/)
{

}

void DeadState::Exit(dae::GameObject& /*obj*/)
{
}
