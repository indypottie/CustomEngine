#pragma once
#include <glm.hpp>
#include <memory>
#include <string>


namespace dae
{
	class GameObject;
	class Font;
	class Scene;
}

class HealthComponent;

namespace Prefab
{
	void CreateStartMenu(dae::Scene& scene);

	void CreateEndScreen(dae::Scene& scene, int scoreToSave);

	void CreateGame(const std::string& levelFilePath, dae::Font* font, dae::Scene& scene, int rows, int cols, float cellSize, bool isVersus);


	std::unique_ptr<dae::GameObject> CreateBomberman(int playerIndex);

	std::unique_ptr<dae::GameObject> CreateBalloomPlayer(int playerIndex);

	std::unique_ptr<dae::GameObject> CreateBalloom();

	std::unique_ptr<dae::GameObject> CreateOneal();

	std::unique_ptr<dae::GameObject> CreateDoll();

	std::unique_ptr<dae::GameObject> CreateMinvo();

	std::unique_ptr<dae::GameObject> CreateBomb(const glm::vec2& playerPos, int explosionRange = 1, int playerIdx = 0);

}
