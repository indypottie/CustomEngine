// ReSharper disable CppClangTidyModernizeReturnBracedInitList
#pragma once
#include <memory>
#include <string>
#include <vec2.hpp>
#include <vec3.hpp>

#include "Component.h"
#include <vector>

#include "TextureComponent.h"

namespace dae
{
	class Texture2D;
}

enum class TileType
{
    Floor = 0,
    IndestructibleWall = 1,
    DestructibleWall = 2,
    ExitBlock = 3,
    PowerUpExtraBomb = 4,   // Extra bomb pickup
    PowerUpDetonator = 5,   // Detonator pickup
    PowerUpFlames = 6,      // Flames pickup (explosion range)

    Empty = 7
};


struct TileCoords
{
    int row{}; // y-axis (vertical index)
    int col{}; // x-axis (horizontal index)

    TileCoords() = default;

    // Note: Parameters are (col, row) for human readability (x, y),
    // but stored internally as (row, col) to match grid[row][col] access.
    TileCoords(int c, int r) : row(r), col(c) {}

    // Named constructor for clarity
    static TileCoords FromColRow(int col, int row) {
        return TileCoords(col, row);
    }

    // Addition operator (col + col, row + row)
    TileCoords operator+(const TileCoords& other) const
    {
        return TileCoords(col + other.col, row + other.row);
    }

    // Scalar multiplication
    TileCoords operator*(int scalar) const
    {
        return TileCoords(col * scalar, row * scalar);
    }

    // Equality comparison
    bool operator==(const TileCoords& other) const
    {
        return col == other.col && row == other.row;
    }

    // conversion from iVec2
	TileCoords(const glm::ivec2& iVec2) : TileCoords(iVec2.x, iVec2.y) {}

    // conversion to iVec2
	operator glm::ivec2() const
    {
        return glm::ivec2{ col, row };
    }

    // conversion to vec2 coords
    [[nodiscard]] glm::vec2 ToVec2() const
    {
        return { row, col };
    }

    // String representation for debugging
    [[nodiscard]] std::string ToString() const
    {
        return "TileCoords(col: " + std::to_string(col) + ", row: " + std::to_string(row) + ")";
    }
};




class GridComponent : public Component
{
public:
    GridComponent(dae::GameObject& owner, int rows, int cols, float cellSize);

    void Initialize();
    void Render() const override;

    TileType GetTile(const TileCoords& coords, int layer = 1) const;
    void SetTile(const TileCoords& coords, TileType type, int layer = 1);

    int GetRows() const { return m_Rows; }
    int GetCols() const { return m_Cols; }
    float GetCellSize() const { return m_CellSize; }

    glm::vec2 GetWorldPosition(const TileCoords& coords) const;
	TileCoords GetTileFromWorldPosition(const glm::vec2& position) const;

    bool IsTileWalkable(const TileCoords& coords) const;

    bool BreakTile(const TileCoords& coords);

    int GetRenderPriority() const override { return 0; } // lower = drawn first grid has the highest priority since it's the background

    static bool IsObjectTile(TileType type);

private:

    void RenderTile(TileType type, float x, float y) const;
    bool IsInBounds(const TileCoords& coords) const;

    void SpawnExplosionEffect(const TileCoords& coords);

    int m_Rows;
	int m_Cols;

    float m_CellSize;

    std::vector<TileType> m_BaseLayer;
    std::vector<TileType> m_OverlayLayer;

    std::unique_ptr<dae::Texture2D> m_IndestructibleWallTexturePtr;
	std::unique_ptr<dae::Texture2D> m_DestructibleWallTexturePtr;
    std::unique_ptr<dae::Texture2D> m_FloorTexturePtr;

    std::unique_ptr<dae::Texture2D> m_ExtraBombTexturePtr;
    std::unique_ptr<dae::Texture2D> m_DetonatorTexturePtr;
    std::unique_ptr<dae::Texture2D> m_FlamesTexturePtr;
    std::unique_ptr<dae::Texture2D> m_DoorTexturePtr;
};


