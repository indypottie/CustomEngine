#include "GridComponent.h"

#include <iostream>
#include <algorithm>

#include "GameObject.h"
#include "Renderer.h"
#include "Scene.h"
#include "SceneManager.h"
#include "ServiceLocator.h"
#include "SpriteAnimatorComponent.h"
#include "Texture2D.h"

GridComponent::GridComponent(dae::GameObject& owner, int rows, int cols, float cellSize)
    : Component(owner),
    m_Rows(rows), m_Cols(cols),
    m_CellSize(cellSize),
    m_BaseLayer(static_cast<size_t>(rows* cols), TileType::Empty),
    m_OverlayLayer(static_cast<size_t>(rows* cols), TileType::Empty)
{
    Initialize();
}

void GridComponent::Initialize()
{
    m_IndestructibleWallTexturePtr = std::make_unique<dae::Texture2D>("Resources/IndestructibleWall.tga");
    m_DestructibleWallTexturePtr = std::make_unique<dae::Texture2D>("Resources/DestructibleWall.tga");
    m_FloorTexturePtr = std::make_unique<dae::Texture2D>("Resources/Floor.tga");

    m_ExtraBombTexturePtr = std::make_unique<dae::Texture2D>("Resources/ExtraBomb.tga");
    m_DetonatorTexturePtr = std::make_unique<dae::Texture2D>("Resources/Detonator.tga");
    m_FlamesTexturePtr = std::make_unique<dae::Texture2D>("Resources/Flames.tga");

    m_DoorTexturePtr = std::make_unique<dae::Texture2D>("Resources/Door.tga");
    
}

TileType GridComponent::GetTile(const TileCoords& coords, int layer) const
{
    if (!IsInBounds(coords))
    {
        std::cout << "GridComponent::GetTile - index out of range\n";
        return TileType::Empty;
    }

    const size_t index = coords.row * m_Cols + coords.col;

    if (layer == 0) return m_BaseLayer[index];
    if (layer == 1) return m_OverlayLayer[index];

    std::cerr << "GridComponent::GetTile - invalid layer: " << layer << "\n";
    return TileType::Empty;
}

void GridComponent::SetTile(const TileCoords& coords, TileType type, int layer)
{
    if (!IsInBounds(coords))
    {
        std::cout << "GridComponent::SetTile - index out of range\n";
        return;
    }

    const size_t index = coords.row * m_Cols + coords.col;

    if (layer == 0) m_BaseLayer[index] = type;
    else if (layer == 1) m_OverlayLayer[index] = type;
    else std::cerr << "GridComponent::SetTile - invalid layer: " << layer << "\n";
}

glm::vec2 GridComponent::GetWorldPosition(const TileCoords& coords) const
{
    auto pos = GetOwner()->GetPosition();
    return {
        pos.x + static_cast<float>(coords.col) * m_CellSize,
        pos.y + static_cast<float>(coords.row) * m_CellSize
    };
}

TileCoords GridComponent::GetTileFromWorldPosition(const glm::vec2& position) const
{
    auto origin = GetOwner()->GetPosition();

    // Offset position by half the cell size to get the center of the object
    glm::vec2 centerPos = position + glm::vec2{ m_CellSize * 0.5f, m_CellSize * 0.5f };

    float localX = centerPos.x - origin.x;
    float localY = centerPos.y - origin.y;

    int col = std::max(0, static_cast<int>(localX / m_CellSize));
    int row = std::max(0, static_cast<int>(localY / m_CellSize));

    return {col, row};
}

bool GridComponent::IsTileWalkable(const TileCoords& coords) const
{
    if (!IsInBounds(coords))
        return false;

    const auto overlay = GetTile(coords, 1);

    return overlay == TileType::Floor;
}

bool GridComponent::BreakTile(const TileCoords& coords)
{
    const TileType type = GetTile(coords, 1);

    if (type != TileType::DestructibleWall) return false;

    ServiceLocator::GetCollisionSystem().UnregisterStaticCollider({ coords.row, coords.col }, false); // remove colliders from broken block

    const TileType typeObjectGrid = GetTile(coords, 0);

    IsObjectTile(typeObjectGrid) ? SetTile(coords, TileType::Empty, 1) : SetTile(coords, TileType::Floor, 1); // if there is an object under the tile set tile to empty to show object

    SpawnExplosionEffect(coords);

    return true;
}

void GridComponent::Render() const
{
    for (int row = 0; row < m_Rows; ++row)
    {
        for (int col = 0; col < m_Cols; ++col)
        {
            TileCoords coords(col, row);
            auto pos = GetWorldPosition(coords);

            // Render base first
            RenderTile(GetTile(coords, 0), pos.x, pos.y);

            // Then overlay
            RenderTile(GetTile(coords, 1), pos.x, pos.y);
        }
    }
}

void GridComponent::RenderTile(TileType type, float x, float y) const
{
    switch (type)
    {
    case TileType::Floor:
        dae::Renderer::GetInstance().RenderTexture(*m_FloorTexturePtr, x, y);
        break;

    case TileType::IndestructibleWall:
        dae::Renderer::GetInstance().RenderTexture(*m_IndestructibleWallTexturePtr, x, y);
        break;

    case TileType::DestructibleWall:
        dae::Renderer::GetInstance().RenderTexture(*m_DestructibleWallTexturePtr, x, y);
        break;

    case TileType::ExitBlock:
        dae::Renderer::GetInstance().RenderTexture(*m_DoorTexturePtr, x, y);
        break;

    case TileType::PowerUpExtraBomb:
        dae::Renderer::GetInstance().RenderTexture(*m_ExtraBombTexturePtr, x, y);
        break;

    case TileType::PowerUpDetonator:
        dae::Renderer::GetInstance().RenderTexture(*m_DetonatorTexturePtr, x, y);
        break;

    case TileType::PowerUpFlames:
        dae::Renderer::GetInstance().RenderTexture(*m_FlamesTexturePtr, x, y);
        break;

    case TileType::Empty:
        break;
    }
}

bool GridComponent::IsInBounds(const TileCoords& coords) const
{
    return coords.row >= 0 && coords.row < m_Rows &&
        coords.col >= 0 && coords.col < m_Cols;
}

bool GridComponent::IsObjectTile(TileType type)
{
    return (type == TileType::ExitBlock || type == TileType::PowerUpDetonator || type == TileType::PowerUpExtraBomb || type == TileType::PowerUpFlames);
}

void GridComponent::SpawnExplosionEffect(const TileCoords& coords)
{
    const glm::vec2 pos = GetWorldPosition(coords);

    auto explosionEffectObject = std::make_unique<dae::GameObject>();
    explosionEffectObject->SetTag("ExplosionFVX");

    explosionEffectObject->SetPosition(pos.x, pos.y);

    explosionEffectObject->AddComponent<TextureComponent>("ExplodingTile.tga");

    auto animator = explosionEffectObject->AddComponent<SpriteAnimatorComponent>(32, 32);

    Animation explodeTile;
    explodeTile.frames = { {0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 0} };
    explodeTile.frameTime = 0.10f;
    explodeTile.looping = false;
    explodeTile.destroyAfterAnimation = true;

    animator->AddAnimation("ExplodeTile", explodeTile);
    animator->Play("ExplodeTile");

    dae::SceneManager::GetInstance().GetActiveScene()->Add(std::move(explosionEffectObject));
}