#include "LevelManager.h"

#include <fstream>
#include <iostream>
#include <SDL_keycode.h>
#include <nlohmann/json.hpp>

#include "ControllerButtons.h"
#include "GameCommands.h"
#include "GameOptions.h"
#include "HudManager.h"
#include "InputManager.h"
#include "LambdaCommand.h"
#include "PlayerComponent.h"
#include "PrefabFactory.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Scene.h"
#include "SceneManager.h"
#include "ServiceLocator.h"

LevelManager::LevelManager(): m_CanExitLevel(false), m_FoundExit(false), m_CurrentScore(0), m_HudManager(HudManager::GetInstance())
{
}

void LevelManager::AddPlayer(PlayerComponent& player)
{
	m_Players.emplace_back(&player);
}

PlayerComponent* LevelManager::GetPlayer(int index) const
{
	for (auto& player : m_Players)
	{
		if (player->GetPlayerIndex() == index) return player;
	}

	return nullptr;
}

void LevelManager::AddEnemy(EnemyComponent& enemy)
{
	m_Enemies.emplace_back(&enemy);

	int totalEnemies = static_cast<int>(m_Enemies.size());

	m_HudManager.UpdateEnemies(totalEnemies);
}

void LevelManager::RemoveEnemy(const EnemyComponent& enemy)
{
	if(m_Enemies.empty()) return;

	std::erase(m_Enemies, &enemy);

	int totalEnemies = static_cast<int>(m_Enemies.size());

	if (totalEnemies <= 0 && m_FoundExit) m_CanExitLevel = true;

	m_HudManager.UpdateEnemies(totalEnemies);
}

void LevelManager::AddPoints(int points)
{
	m_CurrentScore += points;
	m_HudManager.UpdateScore(m_CurrentScore);
}

void LevelManager::TryAdvanceLevel()
{
	if (!m_CanExitLevel)
		return;

	// Check if player is on the exit tile position
	for (auto* player : m_Players)
	{
		if (player == nullptr) continue;

		++m_CurrentLevel;

		if (m_CurrentLevel >= m_MaxLevels)
		{
			// Player won the game
			m_GameWon = true;
			EndGame();
		}
		else
		{
			// Load next level
			LoadLevel(m_CurrentLevel);
			// Reset exit state for new level
			m_FoundExit = false;
			m_CanExitLevel = false;
		}
	}
}

void LevelManager::SkipLevel()
{
	++m_CurrentLevel;

	if (m_CurrentLevel >= m_MaxLevels)
	{
		m_GameWon = true;
		// Trigger game over or win screen here
		std::cout << "Game Won by skipping all levels!" << '\n';
		EndGame();
	}
	else
	{
		LoadLevel(m_CurrentLevel);
		m_FoundExit = false;
		m_CanExitLevel = false;
	}
}

void LevelManager::LoadLevel(int levelIndex)
{
	switch (levelIndex)
	{
	case 0:
		LoadLevelFromFile("../Data/Level1.json");
		break;
	case 1:
		LoadLevelFromFile("../Data/Level2.json");
		break;
	case 2:
		LoadLevelFromFile("../Data/Level3.json");
		break;
	default:
		std::cerr << "Invalid level index: " << levelIndex << '\n';
		break;
	}
}

void LevelManager::LoadLevelFromFile(const std::string& levelFile)
{
	std::cout << "Loading level: " << levelFile << '\n';

	m_Enemies.clear();
	m_Players.clear();
	m_CanExitLevel = false;
	m_FoundExit = false;

	dae::InputManager::GetInstance().ClearCommands();

	ServiceLocator::GetCollisionSystem().ClearColliders();

	auto scene = dae::SceneManager::GetInstance().GetActiveScene();
	scene->RemoveAll();

	auto font = dae::ResourceManager::GetInstance().LoadFont("Emulogic.ttf", 24);

	Prefab::CreateGame(levelFile, font, *scene, 13, 31, 32.f, false);

	auto gridComponent = GetGrid();

	auto bombermanObject = Prefab::CreateBomberman(0);
	bombermanObject->SetPosition(gridComponent->GetWorldPosition({ 2, 1 }).x, gridComponent->GetWorldPosition({ 2, 1 }).y);

	auto& input = dae::InputManager::GetInstance();

	// Bind controller commands
	input.BindControllerCommand(0, ControllerButton::DPAD_RIGHT, keyState::down, std::make_unique<BombermanMoveCommand>(*bombermanObject, Direction::Right, 0));
	input.BindControllerCommand(0, ControllerButton::DPAD_LEFT, keyState::down, std::make_unique<BombermanMoveCommand>(*bombermanObject, Direction::Left, 0));
	input.BindControllerCommand(0, ControllerButton::DPAD_UP, keyState::down, std::make_unique<BombermanMoveCommand>(*bombermanObject, Direction::Up, 0));
	input.BindControllerCommand(0, ControllerButton::DPAD_DOWN, keyState::down, std::make_unique<BombermanMoveCommand>(*bombermanObject, Direction::Down, 0));

	input.BindControllerCommand(0, ControllerButton::DPAD_RIGHT, keyState::released, std::make_unique<BombermanMoveCommand>(*bombermanObject, Direction::None, 0));
	input.BindControllerCommand(0, ControllerButton::DPAD_LEFT, keyState::released, std::make_unique<BombermanMoveCommand>(*bombermanObject, Direction::None, 0));
	input.BindControllerCommand(0, ControllerButton::DPAD_UP, keyState::released, std::make_unique<BombermanMoveCommand>(*bombermanObject, Direction::None, 0));
	input.BindControllerCommand(0, ControllerButton::DPAD_DOWN, keyState::released, std::make_unique<BombermanMoveCommand>(*bombermanObject, Direction::None, 0));

	input.BindControllerCommand(0, ControllerButton::BUTTON_B, keyState::released, std::make_unique<BombermanDropBombCommand>(*bombermanObject));

	// Bind keyboard commands
	input.BindKeyboardCommand(SDLK_w, keyState::down, std::make_unique<BombermanMoveCommand>(*bombermanObject, Direction::Up, 0));
	input.BindKeyboardCommand(SDLK_a, keyState::down, std::make_unique<BombermanMoveCommand>(*bombermanObject, Direction::Left, 0));
	input.BindKeyboardCommand(SDLK_s, keyState::down, std::make_unique<BombermanMoveCommand>(*bombermanObject, Direction::Down, 0));
	input.BindKeyboardCommand(SDLK_d, keyState::down, std::make_unique<BombermanMoveCommand>(*bombermanObject, Direction::Right, 0));

	input.BindKeyboardCommand(SDLK_w, keyState::released, std::make_unique<BombermanMoveCommand>(*bombermanObject, Direction::None, 0));
	input.BindKeyboardCommand(SDLK_a, keyState::released, std::make_unique<BombermanMoveCommand>(*bombermanObject, Direction::None, 0));
	input.BindKeyboardCommand(SDLK_s, keyState::released, std::make_unique<BombermanMoveCommand>(*bombermanObject, Direction::None, 0));
	input.BindKeyboardCommand(SDLK_d, keyState::released, std::make_unique<BombermanMoveCommand>(*bombermanObject, Direction::None, 0));

	input.BindKeyboardCommand(SDLK_q, keyState::released, std::make_unique<BombermanDropBombCommand>(*bombermanObject));
	input.BindKeyboardCommand(SDLK_t, keyState::released, std::make_unique<BombermanDetonateBombCommand>(*bombermanObject));

	scene->Add(std::move(bombermanObject));

	input.BindKeyboardCommand(SDLK_F2, keyState::released, std::make_unique<LambdaCommand>([] {GameOptions::GetInstance().ToggleMute(); }));
	input.BindKeyboardCommand(SDLK_F1, keyState::released, std::make_unique<LambdaCommand>([] {GameOptions::GetInstance().SkipLevel(); }));
}

void LevelManager::LoadSoloGame()
{
	std::cout << "SOLO GAME CALLED\n";

	ServiceLocator::GetSoundSystem().StopSound("Resources/Audio/TitleScreen.wav");
	ServiceLocator::GetSoundSystem().QueueSound("Resources/Audio/BombermanLevelMusic.ogg", -1);

	LoadLevelFromFile("../Data/Level1.json");
}

void LevelManager::LoadCoOpGame()
{
	std::cout << "CO-OP GAME CALLED\n";

	dae::InputManager::GetInstance().ClearCommands();

	ServiceLocator::GetCollisionSystem().ClearColliders();

	auto scene = dae::SceneManager::GetInstance().GetActiveScene();
	scene->RemoveAll();

	auto font = dae::ResourceManager::GetInstance().LoadFont("Emulogic.ttf", 24);

	Prefab::CreateGame("../Data/Level1.json", font, *scene, 13, 31, 32.f, false);

	auto gridComponent = GetGrid();

	// === Player 1 Setup ===
	auto p1 = Prefab::CreateBomberman(0);
	p1->SetPosition(gridComponent->GetWorldPosition({ 2, 1 }).x, gridComponent->GetWorldPosition({ 2, 1 }).y);

	auto& input = dae::InputManager::GetInstance();

	// Controller 0
	input.BindControllerCommand(0, ControllerButton::DPAD_RIGHT, keyState::down, std::make_unique<BombermanMoveCommand>(*p1, Direction::Right, 0));
	input.BindControllerCommand(0, ControllerButton::DPAD_LEFT, keyState::down, std::make_unique<BombermanMoveCommand>(*p1, Direction::Left, 0));
	input.BindControllerCommand(0, ControllerButton::DPAD_UP, keyState::down, std::make_unique<BombermanMoveCommand>(*p1, Direction::Up, 0));
	input.BindControllerCommand(0, ControllerButton::DPAD_DOWN, keyState::down, std::make_unique<BombermanMoveCommand>(*p1, Direction::Down, 0));

	input.BindControllerCommand(0, ControllerButton::DPAD_RIGHT, keyState::released, std::make_unique<BombermanMoveCommand>(*p1, Direction::None, 0));
	input.BindControllerCommand(0, ControllerButton::DPAD_LEFT, keyState::released, std::make_unique<BombermanMoveCommand>(*p1, Direction::None, 0));
	input.BindControllerCommand(0, ControllerButton::DPAD_UP, keyState::released, std::make_unique<BombermanMoveCommand>(*p1, Direction::None, 0));
	input.BindControllerCommand(0, ControllerButton::DPAD_DOWN, keyState::released, std::make_unique<BombermanMoveCommand>(*p1, Direction::None, 0));

	input.BindControllerCommand(0, ControllerButton::BUTTON_B, keyState::released, std::make_unique<BombermanDropBombCommand>(*p1));

	// Keyboard (WASD)
	input.BindKeyboardCommand(SDLK_w, keyState::down, std::make_unique<BombermanMoveCommand>(*p1, Direction::Up, 0));
	input.BindKeyboardCommand(SDLK_a, keyState::down, std::make_unique<BombermanMoveCommand>(*p1, Direction::Left, 0));
	input.BindKeyboardCommand(SDLK_s, keyState::down, std::make_unique<BombermanMoveCommand>(*p1, Direction::Down, 0));
	input.BindKeyboardCommand(SDLK_d, keyState::down, std::make_unique<BombermanMoveCommand>(*p1, Direction::Right, 0));

	input.BindKeyboardCommand(SDLK_w, keyState::released, std::make_unique<BombermanMoveCommand>(*p1, Direction::None, 0));
	input.BindKeyboardCommand(SDLK_a, keyState::released, std::make_unique<BombermanMoveCommand>(*p1, Direction::None, 0));
	input.BindKeyboardCommand(SDLK_s, keyState::released, std::make_unique<BombermanMoveCommand>(*p1, Direction::None, 0));
	input.BindKeyboardCommand(SDLK_d, keyState::released, std::make_unique<BombermanMoveCommand>(*p1, Direction::None, 0));

	input.BindKeyboardCommand(SDLK_q, keyState::released, std::make_unique<BombermanDropBombCommand>(*p1));


	// === Player 2 Setup ===
	auto p2 = Prefab::CreateBomberman(1);
	p2->SetPosition(gridComponent->GetWorldPosition({ 1, 2 }).x, gridComponent->GetWorldPosition({ 1, 2 }).y);

	// Controller 1
	input.BindControllerCommand(1, ControllerButton::DPAD_RIGHT, keyState::down, std::make_unique<BombermanMoveCommand>(*p2, Direction::Right, 1));
	input.BindControllerCommand(1, ControllerButton::DPAD_LEFT, keyState::down, std::make_unique<BombermanMoveCommand>(*p2, Direction::Left, 1));
	input.BindControllerCommand(1, ControllerButton::DPAD_UP, keyState::down, std::make_unique<BombermanMoveCommand>(*p2, Direction::Up, 1));
	input.BindControllerCommand(1, ControllerButton::DPAD_DOWN, keyState::down, std::make_unique<BombermanMoveCommand>(*p2, Direction::Down, 1));

	input.BindControllerCommand(1, ControllerButton::DPAD_RIGHT, keyState::released, std::make_unique<BombermanMoveCommand>(*p2, Direction::None, 1));
	input.BindControllerCommand(1, ControllerButton::DPAD_LEFT, keyState::released, std::make_unique<BombermanMoveCommand>(*p2, Direction::None, 1));
	input.BindControllerCommand(1, ControllerButton::DPAD_UP, keyState::released, std::make_unique<BombermanMoveCommand>(*p2, Direction::None, 1));
	input.BindControllerCommand(1, ControllerButton::DPAD_DOWN, keyState::released, std::make_unique<BombermanMoveCommand>(*p2, Direction::None, 1));

	input.BindControllerCommand(1, ControllerButton::BUTTON_B, keyState::released, std::make_unique<BombermanDropBombCommand>(*p2));

	// Keyboard (Arrow keys)
	input.BindKeyboardCommand(SDLK_UP, keyState::down, std::make_unique<BombermanMoveCommand>(*p2, Direction::Up, 1));
	input.BindKeyboardCommand(SDLK_LEFT, keyState::down, std::make_unique<BombermanMoveCommand>(*p2, Direction::Left, 1));
	input.BindKeyboardCommand(SDLK_DOWN, keyState::down, std::make_unique<BombermanMoveCommand>(*p2, Direction::Down, 1));
	input.BindKeyboardCommand(SDLK_RIGHT, keyState::down, std::make_unique<BombermanMoveCommand>(*p2, Direction::Right, 1));

	input.BindKeyboardCommand(SDLK_UP, keyState::released, std::make_unique<BombermanMoveCommand>(*p2, Direction::None, 1));
	input.BindKeyboardCommand(SDLK_LEFT, keyState::released, std::make_unique<BombermanMoveCommand>(*p2, Direction::None, 1));
	input.BindKeyboardCommand(SDLK_DOWN, keyState::released, std::make_unique<BombermanMoveCommand>(*p2, Direction::None, 1));
	input.BindKeyboardCommand(SDLK_RIGHT, keyState::released, std::make_unique<BombermanMoveCommand>(*p2, Direction::None, 1));

	input.BindKeyboardCommand(SDLK_SLASH, keyState::released, std::make_unique<BombermanDropBombCommand>(*p2));

	// Add both players to scene
	scene->Add(std::move(p1));
	scene->Add(std::move(p2));

	ServiceLocator::GetSoundSystem().StopSound("Resources/Audio/TitleScreen.wav");
	ServiceLocator::GetSoundSystem().QueueSound("Resources/Audio/BombermanLevelMusic.ogg", -1);

	input.BindKeyboardCommand(SDLK_F2, keyState::released, std::make_unique<LambdaCommand>([] {GameOptions::GetInstance().ToggleMute(); }));
	input.BindKeyboardCommand(SDLK_F1, keyState::released, std::make_unique<LambdaCommand>([] {GameOptions::GetInstance().SkipLevel(); }));
}

void LevelManager::LoadVersusGame()
{
	std::cout << "VS GAME CALLED\n";
	auto& input = dae::InputManager::GetInstance();
	input.ClearCommands();

	ServiceLocator::GetCollisionSystem().ClearColliders();

	auto scene = dae::SceneManager::GetInstance().GetActiveScene();
	scene->RemoveAll();

	auto font = dae::ResourceManager::GetInstance().LoadFont("Emulogic.ttf", 24);

	// Create the level without enemies
	Prefab::CreateGame("../Data/VersusLevel.json", font, *scene, 13, 31, 32.f, true);

	// Create players
	auto bomberman = Prefab::CreateBomberman(0);
	auto balloom = Prefab::CreateBalloomPlayer(1);

	// Get grid and set starting positions
	auto grid = GetGrid();
	bomberman->SetPosition(grid->GetWorldPosition({ 2, 1 }).x, grid->GetWorldPosition({ 2, 1 }).y);
	balloom->SetPosition(grid->GetWorldPosition({ 1, 2 }).x, grid->GetWorldPosition({ 1, 2 }).y);

	// === Player 1 Input Setup (Bomberman) ===
	// Controller 0
	input.BindControllerCommand(0, ControllerButton::DPAD_RIGHT, keyState::down, std::make_unique<BombermanMoveCommand>(*bomberman, Direction::Right, 0));
	input.BindControllerCommand(0, ControllerButton::DPAD_LEFT, keyState::down, std::make_unique<BombermanMoveCommand>(*bomberman, Direction::Left, 0));
	input.BindControllerCommand(0, ControllerButton::DPAD_UP, keyState::down, std::make_unique<BombermanMoveCommand>(*bomberman, Direction::Up, 0));
	input.BindControllerCommand(0, ControllerButton::DPAD_DOWN, keyState::down, std::make_unique<BombermanMoveCommand>(*bomberman, Direction::Down, 0));

	input.BindControllerCommand(0, ControllerButton::DPAD_RIGHT, keyState::released, std::make_unique<BombermanMoveCommand>(*bomberman, Direction::None, 0));
	input.BindControllerCommand(0, ControllerButton::DPAD_LEFT, keyState::released, std::make_unique<BombermanMoveCommand>(*bomberman, Direction::None, 0));
	input.BindControllerCommand(0, ControllerButton::DPAD_UP, keyState::released, std::make_unique<BombermanMoveCommand>(*bomberman, Direction::None, 0));
	input.BindControllerCommand(0, ControllerButton::DPAD_DOWN, keyState::released, std::make_unique<BombermanMoveCommand>(*bomberman, Direction::None, 0));

	input.BindControllerCommand(0, ControllerButton::BUTTON_B, keyState::released, std::make_unique<BombermanDropBombCommand>(*bomberman));

	// Keyboard (WASD + Q)
	input.BindKeyboardCommand(SDLK_w, keyState::down, std::make_unique<BombermanMoveCommand>(*bomberman, Direction::Up, 0));
	input.BindKeyboardCommand(SDLK_a, keyState::down, std::make_unique<BombermanMoveCommand>(*bomberman, Direction::Left, 0));
	input.BindKeyboardCommand(SDLK_s, keyState::down, std::make_unique<BombermanMoveCommand>(*bomberman, Direction::Down, 0));
	input.BindKeyboardCommand(SDLK_d, keyState::down, std::make_unique<BombermanMoveCommand>(*bomberman, Direction::Right, 0));

	input.BindKeyboardCommand(SDLK_w, keyState::released, std::make_unique<BombermanMoveCommand>(*bomberman, Direction::None, 0));
	input.BindKeyboardCommand(SDLK_a, keyState::released, std::make_unique<BombermanMoveCommand>(*bomberman, Direction::None, 0));
	input.BindKeyboardCommand(SDLK_s, keyState::released, std::make_unique<BombermanMoveCommand>(*bomberman, Direction::None, 0));
	input.BindKeyboardCommand(SDLK_d, keyState::released, std::make_unique<BombermanMoveCommand>(*bomberman, Direction::None, 0));

	input.BindKeyboardCommand(SDLK_q, keyState::released, std::make_unique<BombermanDropBombCommand>(*bomberman));


	// === Player 2 Input Setup (Balloom) ===
	// Controller 1
	input.BindControllerCommand(1, ControllerButton::DPAD_RIGHT, keyState::down, std::make_unique<BombermanMoveCommand>(*balloom, Direction::Right, 1));
	input.BindControllerCommand(1, ControllerButton::DPAD_LEFT, keyState::down, std::make_unique<BombermanMoveCommand>(*balloom, Direction::Left, 1));
	input.BindControllerCommand(1, ControllerButton::DPAD_UP, keyState::down, std::make_unique<BombermanMoveCommand>(*balloom, Direction::Up, 1));
	input.BindControllerCommand(1, ControllerButton::DPAD_DOWN, keyState::down, std::make_unique<BombermanMoveCommand>(*balloom, Direction::Down, 1));

	input.BindControllerCommand(1, ControllerButton::DPAD_RIGHT, keyState::released, std::make_unique<BombermanMoveCommand>(*balloom, Direction::None, 1));
	input.BindControllerCommand(1, ControllerButton::DPAD_LEFT, keyState::released, std::make_unique<BombermanMoveCommand>(*balloom, Direction::None, 1));
	input.BindControllerCommand(1, ControllerButton::DPAD_UP, keyState::released, std::make_unique<BombermanMoveCommand>(*balloom, Direction::None, 1));
	input.BindControllerCommand(1, ControllerButton::DPAD_DOWN, keyState::released, std::make_unique<BombermanMoveCommand>(*balloom, Direction::None, 1));

	input.BindControllerCommand(1, ControllerButton::BUTTON_B, keyState::released, std::make_unique<BombermanDropBombCommand>(*balloom));

	// Keyboard (Arrow keys + Slash)
	input.BindKeyboardCommand(SDLK_UP, keyState::down, std::make_unique<BombermanMoveCommand>(*balloom, Direction::Up, 1));
	input.BindKeyboardCommand(SDLK_LEFT, keyState::down, std::make_unique<BombermanMoveCommand>(*balloom, Direction::Left, 1));
	input.BindKeyboardCommand(SDLK_DOWN, keyState::down, std::make_unique<BombermanMoveCommand>(*balloom, Direction::Down, 1));
	input.BindKeyboardCommand(SDLK_RIGHT, keyState::down, std::make_unique<BombermanMoveCommand>(*balloom, Direction::Right, 1));

	input.BindKeyboardCommand(SDLK_UP, keyState::released, std::make_unique<BombermanMoveCommand>(*balloom, Direction::None, 1));
	input.BindKeyboardCommand(SDLK_LEFT, keyState::released, std::make_unique<BombermanMoveCommand>(*balloom, Direction::None, 1));
	input.BindKeyboardCommand(SDLK_DOWN, keyState::released, std::make_unique<BombermanMoveCommand>(*balloom, Direction::None, 1));
	input.BindKeyboardCommand(SDLK_RIGHT, keyState::released, std::make_unique<BombermanMoveCommand>(*balloom, Direction::None, 1));

	input.BindKeyboardCommand(SDLK_SLASH, keyState::released, std::make_unique<BombermanDropBombCommand>(*balloom));


	// Add players to scene
	scene->Add(std::move(bomberman));
	scene->Add(std::move(balloom));

	// Start music or other setup
	ServiceLocator::GetSoundSystem().StopSound("Resources/Audio/TitleScreen.wav");
	ServiceLocator::GetSoundSystem().QueueSound("Resources/Audio/BombermanLevelMusic.ogg", -1);

	input.BindKeyboardCommand(SDLK_F2, keyState::released, std::make_unique<LambdaCommand>([] {GameOptions::GetInstance().ToggleMute(); }));
	input.BindKeyboardCommand(SDLK_F1, keyState::released, std::make_unique<LambdaCommand>([] {GameOptions::GetInstance().SkipLevel(); }));
}

void LevelManager::LoadScores()
{
	m_Scores.clear();
	std::ifstream inFile(m_SaveFile);
	std::string name;
	int score;
	while (inFile >> name >> score)
		m_Scores.emplace_back(HighScoreEntry{ name, score });
}

void LevelManager::SaveScores()
{
	std::ofstream outFile(m_SaveFile);
	for (const auto& entry : m_Scores)
		outFile << entry.name << ' ' << entry.score << '\n';
}

std::vector<HighScoreEntry> LevelManager::GetHighScores() const
{
	auto sorted = m_Scores;

	std::ranges::sort(sorted, [](const auto& a, const auto& b) {
		return a.score > b.score;
		});

	return sorted;
}

void LevelManager::AddHighScore(const std::string& name, int score)
{
	m_Scores.emplace_back(HighScoreEntry{ name, score });
	SortAndTrim();
}

void LevelManager::EndGame()
{
	dae::InputManager::GetInstance().ClearCommands();
	auto scene = dae::SceneManager::GetInstance().GetActiveScene();
	scene->RemoveAll();

	ServiceLocator::GetSoundSystem().StopSound("Resources/Audio/BombermanLevelMusic.ogg");
	ServiceLocator::GetCollisionSystem().ClearColliders();
	SDL_RenderClear(dae::Renderer::GetInstance().GetSDLRenderer());

	Prefab::CreateEndScreen(*scene, m_CurrentScore);
}

void LevelManager::RestartGame()
{
	auto scene = dae::SceneManager::GetInstance().GetActiveScene();
	dae::InputManager::GetInstance().ClearCommands();

	m_Enemies.clear();
	m_Players.clear();
	m_CanExitLevel = false;
	m_CurrentScore = 0;
	m_FoundExit = false;
	m_Grid = nullptr;


	scene->RemoveAll();
	Prefab::CreateStartMenu(*scene);
}

void LevelManager::SortAndTrim()
{
	std::ranges::sort(m_Scores, [](const auto& a, const auto& b) {
		return a.score > b.score;
		});

	if (m_Scores.size() > MaxScores)
		m_Scores.resize(MaxScores);
}
