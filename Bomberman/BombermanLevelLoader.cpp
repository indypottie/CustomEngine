#include "BombermanLevelLoader.h"
#include "GridComponent.h"

#include <fstream>
#include <iostream>
#include <SDL_rect.h>
#include <nlohmann/json.hpp>

#include "ServiceLocator.h"


bool BombermanLevelLoader::LoadLevelFromFile(const std::string& filePath,
                                             GridComponent& grid)
{
    std::ifstream file(filePath);
    if (!file.is_open()) return false;

    nlohmann::json levelJson;
    file >> levelJson;

    if (!levelJson.contains("environmentTiles") || !levelJson.contains("objectTiles"))
    {
        std::cerr << "Level file missing required grids\n";
        return false;
    }

    const auto& envTiles = levelJson["environmentTiles"];
    const auto& objTiles = levelJson["objectTiles"];

    if (!envTiles.is_array() || !objTiles.is_array()) return false;

    int rowsEnv = static_cast<int>(envTiles.size());
    int colsEnv = static_cast<int>(envTiles[0].size());

    int rowsObj = static_cast<int>(objTiles.size());
    int colsObj = static_cast<int>(objTiles[0].size());

    if (rowsEnv != grid.GetRows() || colsEnv != grid.GetCols())
    {
        std::cerr << "Environment grid size mismatch\n";
        return false;
    }

    if (rowsObj != grid.GetRows() || colsObj != grid.GetCols())
    {
        std::cerr << "Object grid size mismatch\n";
        return false;
    }

    // Load environment grid
    for (int row = 0; row < rowsEnv; ++row)
    {
        for (int col = 0; col < colsEnv; ++col)
        {
            int tileValue = envTiles[row][col];
            TileType type = static_cast<TileType>(tileValue);
            grid.SetTile({ col, row }, type, 1); // layer 1 = overlay

            if (type == TileType::IndestructibleWall || type == TileType::DestructibleWall)
            {
                glm::vec2 worldPos = grid.GetWorldPosition({ col, row });
                float cellSize = grid.GetCellSize();

                SDL_FRect colliderRect{
                    worldPos.x,
                    worldPos.y,
                    cellSize,
                    cellSize
                };

                ServiceLocator::GetCollisionSystem().RegisterStaticCollider(colliderRect, { row, col }, false);
            }
        }
    }

    // Load object grid (powerups, exit, etc)
    for (int row = 0; row < rowsObj; ++row)
    {
        for (int col = 0; col < colsObj; ++col)
        {
            int tileValue = objTiles[row][col];
            TileType type = static_cast<TileType>(tileValue);
            grid.SetTile({ col, row }, type, 0); // layer 0 = base

            if (GridComponent::IsObjectTile(type))
            {
                glm::vec2 worldPos = grid.GetWorldPosition({ col, row });
                float cellSize = grid.GetCellSize();

                SDL_FRect colliderRect{
                    worldPos.x,
                    worldPos.y,
                    cellSize,
                    cellSize
                };

                ServiceLocator::GetCollisionSystem().RegisterStaticCollider(colliderRect, { row, col }, true);
            }
        }
    }

    return true;
}
