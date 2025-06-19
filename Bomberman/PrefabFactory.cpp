#include "PrefabFactory.h"

// includes for the components

#include <algorithm>
#include <SDL.h>
#include <iostream>
#include <random>

#include "BombComponent.h"
#include "BombermanLevelLoader.h"
#include "ColliderComponent.h"
#include "DisplayHealthComponent.h"
#include "EnemyComponent.h"
#include "ExplosionComponent.h"
#include "GameObject.h"
#include "GameOptions.h"
#include "GridComponent.h"
#include "HealthComponent.h"
#include "HudManager.h"
#include "InputManager.h"
#include "LambdaCommand.h"
#include "LevelManager.h"
#include "NameEntrySelectorComponent.h"
#include "PlayerComponent.h"
#include "ResourceManager.h"
#include "Scene.h"
#include "ServiceLocator.h"
#include "SpriteAnimatorComponent.h"
#include "StartMenuSelectorComponent.h"
#include "TextureComponent.h"


/// STATIC HELPER FUNCTIONS
static void CreateGameUI(dae::Font* font, dae::Scene& scene)
{
	// --- UI ROOT ---
	auto uiRoot = std::make_unique<dae::GameObject>();
	uiRoot->SetPosition(0.f, 0.f);
	uiRoot->AddComponent<TextureComponent>("BombermanGrayBackground.tga");

	// --- UI ELEMENTS ---
	// TIME
	auto timeUI = std::make_unique<dae::GameObject>();
	timeUI->SetParent(uiRoot.get(), true);
	timeUI->SetPosition(16.f, 16.f);
	auto timerElement = timeUI->AddComponent<TextComponent>("TIME ", font);


	// SCORE
	auto scoreUI = std::make_unique<dae::GameObject>();
	scoreUI->SetParent(uiRoot.get(), true);
	scoreUI->SetPosition(400.f, 16.f);
	auto scoreElement = scoreUI->AddComponent<TextComponent>("0", font);

	// ENEMIES
	auto enemiesUI = std::make_unique<dae::GameObject>();
	enemiesUI->SetParent(uiRoot.get(), true);
	enemiesUI->SetPosition(792.f, 16.f);
	auto enemiesElement = enemiesUI->AddComponent<TextComponent>("LEFT ", font);


	// --- ADD TO HUD MANAGER ---
	auto& hudManager = HudManager::GetInstance();

	hudManager.SetTimerElement(*timerElement);
	hudManager.SetScoreElement(*scoreElement);
	hudManager.SetEnemiesElement(*enemiesElement);

	// --- ADD TO SCENE ---
	scene.Add(std::move(uiRoot));
	scene.Add(std::move(timeUI));
	scene.Add(std::move(scoreUI));
	scene.Add(std::move(enemiesUI));
}

static std::unique_ptr<dae::GameObject> CreateLevel(const std::string& levelFilePath, int rows, int cols, float cellSize)
{
	auto LevelObject = std::make_unique<dae::GameObject>();

	LevelObject->SetTag("LevelObject");

	LevelObject->SetPosition(0.f, 84.f);

	auto explosionComponent = LevelObject->AddComponent<ExplosionComponent>();

	auto gridComponent = LevelObject->AddComponent<GridComponent>(rows, cols, cellSize);
	if (!BombermanLevelLoader::LoadLevelFromFile(levelFilePath, *gridComponent))
	{
		std::cerr << "Failed to load level from file: " << levelFilePath << '\n';
		return nullptr;
	}

	LevelManager::GetInstance().SetGrid(gridComponent);
	LevelManager::GetInstance().SetExplosionSystem(explosionComponent);

	return LevelObject;
}


static std::unique_ptr<dae::GameObject> CreateEnemy(const glm::vec2& pos, EnemyType type)
{
	std::unique_ptr<dae::GameObject> enemy;

	switch (type)
	{
	case EnemyType::Balloom:
		enemy = Prefab::CreateBalloom();
		break;

	case EnemyType::Oneal:
		enemy = Prefab::CreateOneal();
		break;

	case EnemyType::Doll:
		enemy = Prefab::CreateDoll();
		break;

	case EnemyType::Minvo:
		enemy = Prefab::CreateMinvo();
		break;
	}

	if (enemy)
		enemy->SetPosition(pos.x, pos.y);

	if (!enemy)
	{
		std::cout << "Enemy creation failed for type: " << static_cast<int>(type) << "\n";
	}

	return enemy;
}


static void AddEnemiesToScene(GridComponent* grid, dae::Scene& scene, int rows, int cols)
{
	std::vector<TileCoords> walkableTiles;

	for (int row = 0; row < rows; ++row)
	{
		for (int col = 0; col < cols; ++col)
		{
			// Skip tiles in the first 3 columns to avoid spawning enemies near player start
			if (col < 3)
				continue;

			TileCoords coords{ col, row };
			if (grid->IsTileWalkable(coords))
				walkableTiles.emplace_back(coords);
		}
	}

	std::ranges::shuffle(walkableTiles, std::mt19937{ std::random_device{}() });

	int enemyCount = 8;

	std::mt19937 rng{ std::random_device{}() };
	std::uniform_int_distribution<int> dist(0, static_cast<int>(EnemyType::Minvo));

	int spawnedEnemies = 0;
	int attempts = 0;
	constexpr int maxAttempts = 50;

	for (size_t i = 0; spawnedEnemies < enemyCount && i < walkableTiles.size() && attempts < maxAttempts; ++i)
	{
		const auto& coords = walkableTiles[i];
		glm::vec2 pos = grid->GetWorldPosition(coords);

		EnemyType type = static_cast<EnemyType>(dist(rng));
		auto enemy = CreateEnemy(pos, type);
		enemy->SetRenderLayer(10);

		if (enemy)
		{
			scene.Add(std::move(enemy));
			++spawnedEnemies;
		}
		++attempts;
	}
}

/// PREFAB FUNCTIONS
void Prefab::CreateStartMenu(dae::Scene& scene)
{
	// Background
	auto background = std::make_unique<dae::GameObject>();
	background->AddComponent<TextureComponent>("MainMenu.tga");
	background->SetPosition(0.f, 0.f);
	scene.Add(std::move(background));

	// Arrow
	auto arrow = std::make_unique<dae::GameObject>();
	arrow->AddComponent<TextureComponent>("Arrow.tga");
	auto* selector = arrow->AddComponent<StartMenuSelectorComponent>();

	// Set options
	selector->AddOption({ 371.f, 306.f }, []() { LevelManager::GetInstance().LoadSoloGame(); });
	selector->AddOption({ 352.f, 376.f }, []() { LevelManager::GetInstance().LoadCoOpGame(); });
	selector->AddOption({ 416.f, 437.f }, []() { LevelManager::GetInstance().LoadVersusGame(); });

	arrow->SetPosition(371.f, 306.f);

	scene.Add(std::move(arrow));

	// Bind inputs
	auto& input = dae::InputManager::GetInstance();
	input.BindKeyboardCommand(SDLK_UP,			keyState::released,		std::make_unique<LambdaCommand>([selector]()				{ selector->MoveUp();	}));
	input.BindKeyboardCommand(SDLK_DOWN,		keyState::released,		std::make_unique<LambdaCommand>([selector]()				{ selector->MoveDown(); }));
	input.BindKeyboardCommand(SDLK_RETURN,		keyState::released,		std::make_unique<LambdaCommand>([selector]()				{ selector->Select();	}));


	input.BindKeyboardCommand(SDLK_F2, keyState::released, std::make_unique<LambdaCommand>([] {GameOptions::GetInstance().ToggleMute(); }));
	input.BindKeyboardCommand(SDLK_F1, keyState::released, std::make_unique<LambdaCommand>([] {GameOptions::GetInstance().SkipLevel(); }));

	ServiceLocator::GetSoundSystem().QueueSound("../Bomberman/Resources/Audio/TitleScreen.wav", -1);
}

void Prefab::CreateEndScreen(dae::Scene& scene, int scoreToSave)
{
	// Optional background
	auto background = std::make_unique<dae::GameObject>();
	background->AddComponent<TextureComponent>("EndScreen.tga");
	background->SetPosition(0.f, 0.f);
	scene.Add(std::move(background));


	// UI Text Display
	auto font = dae::ResourceManager::GetInstance().LoadFont("Emulogic.ttf", 24);

	auto textGO = std::make_unique<dae::GameObject>();
	auto* textComp = textGO->AddComponent<TextComponent>("AAA", font);
	textGO->SetPosition(320.f, 320.f); 
	auto* rawTextPtr = textComp;
	scene.Add(std::move(textGO));

	std::string scoreString = "SCORE: " + std::to_string(scoreToSave);

	auto scoreGO = std::make_unique<dae::GameObject>();
	scoreGO->AddComponent<TextComponent>(scoreString, font);
	scoreGO->SetPosition(350.f, 130.f);
	scene.Add(std::move(scoreGO));

	// Selector Component
	auto selectorGO = std::make_unique<dae::GameObject>();
	auto* selector = selectorGO->AddComponent<NameEntrySelectorComponent>();
	selector->SetScoreToSave(scoreToSave);
	selector->SetTextComponent(rawTextPtr);
	selectorGO->SetPosition(0.f, 0.f); // Doesn't need to be visible
	scene.Add(std::move(selectorGO));

	// Show Top 3 High Scores
	font = dae::ResourceManager::GetInstance().LoadFont("Emulogic.ttf", 16);

	LevelManager::GetInstance().LoadScores();
	const auto& highScores = LevelManager::GetInstance().GetHighScores();
	const int maxToShow = std::min(3, static_cast<int>(highScores.size()));

	float startY = 5.f; // adjust for vertical position
	float xPos = 50.f;   // adjust for horizontal position

	for (int i = 0; i < maxToShow; ++i)
	{
		const auto& entry = highScores[i];
		std::string entryText = std::to_string(i + 1) + ". " + entry.name + " - " + std::to_string(entry.score);

		auto hsGO = std::make_unique<dae::GameObject>();
		hsGO->AddComponent<TextComponent>(entryText, font);
		hsGO->SetPosition(xPos, startY + static_cast<float>(i) * 30.f); // vertical spacing
		scene.Add(std::move(hsGO));
	}

	// Bind inputs
	auto& input = dae::InputManager::GetInstance();
	input.BindKeyboardCommand(SDLK_LEFT, keyState::released, std::make_unique<LambdaCommand>([selector]() { selector->MoveLeft(); }));
	input.BindKeyboardCommand(SDLK_RIGHT, keyState::released, std::make_unique<LambdaCommand>([selector]() { selector->MoveRight(); }));
	input.BindKeyboardCommand(SDLK_UP, keyState::released, std::make_unique<LambdaCommand>([selector]() { selector->IncrementLetter(); }));
	input.BindKeyboardCommand(SDLK_DOWN, keyState::released, std::make_unique<LambdaCommand>([selector]() { selector->DecrementLetter(); }));
	input.BindKeyboardCommand(SDLK_RETURN, keyState::released, std::make_unique<LambdaCommand>([selector]() { selector->Confirm(); }));
}

void Prefab::CreateGame(const std::string& levelFilePath, dae::Font* font, dae::Scene& scene, int rows, int cols, float cellSize, bool isVersus)
{
	// 1. Create HUD first
	CreateGameUI(font, scene);

	// 2. Assert HUD is ready before level uses it
	assert(HudManager::GetInstance().IsInitialized() && "HUD must be initialized before creating the level!");

	// 3. Create level and add to scene
	if (auto level = CreateLevel(levelFilePath, rows, cols, cellSize))
	{
		scene.Add(std::move(level));

		if (isVersus) return;

		if (const auto grid = LevelManager::GetInstance().GetGrid())
		{
			AddEnemiesToScene(grid, scene, rows, cols);
		}
	}
	else
	{
		std::cerr << "CreateGame: Level creation failed!\n";
	}
}

std::unique_ptr<dae::GameObject> Prefab::CreateBomberman(int playerIndex)
{
	// create the bomberman object
	auto bomberman = std::make_unique<dae::GameObject>();

	// add related components
	auto playerComp = bomberman->AddComponent<PlayerComponent>(playerIndex);
	bomberman->AddComponent<TextureComponent>("PlayerSpriteSheet.tga");
	auto healthComp = bomberman->AddComponent<HealthComponent>();
	bomberman->AddComponent<SpriteAnimatorComponent>(32, 32);

	healthComp->AddObserver(playerComp);

	

	// add collider
	SDL_FRect collider = { 0.f,0.f, 30.f, 30.f };
	bomberman->AddComponent<ColliderComponent>(collider);

	// set tag
	bomberman->SetTag("Player");

	// create the animator
	auto animator = bomberman->GetComponent<SpriteAnimatorComponent>();

	// create the animations
	Animation walkDown;
	walkDown.frames = { {3, 0}, {4, 0}, {5, 0} };
	walkDown.frameTime = 0.1f;
	walkDown.looping = true;

	Animation WalkLeft;
	WalkLeft.frames = { {0, 0}, {1, 0}, {2, 0} };
	WalkLeft.frameTime = 0.1f;
	WalkLeft.looping = true;

	Animation WalkRight;
	WalkRight.frames = { {0, 1}, {1, 1}, {2, 1} };
	WalkRight.frameTime = 0.1f;
	WalkRight.looping = true;

	Animation WalkUp;
	WalkUp.frames = { {3, 1}, {4, 1}, {5, 1} };
	WalkUp.frameTime = 0.1f;
	WalkUp.looping = true;


	// add animations to the animator
	animator->AddAnimation("WalkDown", walkDown);
	animator->AddAnimation("WalkLeft", WalkLeft);
	animator->AddAnimation("WalkRight", WalkRight);
	animator->AddAnimation("WalkUp", WalkUp);
	animator->SetDefaultFrame({ 4, 0 });

	// add player to level manager
	LevelManager::GetInstance().AddPlayer(*bomberman->GetComponent<PlayerComponent>());

	return bomberman;
}

std::unique_ptr<dae::GameObject> Prefab::CreateBalloomPlayer(int playerIndex)
{
	// create the balloom object
	auto balloom = std::make_unique<dae::GameObject>();

	// add related components
	auto playerComp = balloom->AddComponent<PlayerComponent>(playerIndex);
	playerComp->SetIsBalloom(true); // mark this player as Balloom
	balloom->AddComponent<TextureComponent>("BalloomSpriteSheet.tga"); // different texture
	auto healthComp = balloom->AddComponent<HealthComponent>();
	balloom->AddComponent<SpriteAnimatorComponent>(32, 32);

	healthComp->AddObserver(playerComp);

	// add collider
	SDL_FRect collider = { 0.f, 0.f, 30.f, 30.f };
	balloom->AddComponent<ColliderComponent>(collider);

	// set tag
	balloom->SetTag("Enemy");

	// create the animator
	auto animator = balloom->GetComponent<SpriteAnimatorComponent>();

	// create Balloom animations
	Animation walkDown;
	walkDown.frames = { {0, 1}, {1, 1}, {2, 1} };
	walkDown.frameTime = 0.1f;
	walkDown.looping = true;

	Animation walkLeft;
	walkLeft.frames = { {0, 1}, {1, 1}, {2, 1} };
	walkLeft.frameTime = 0.1f;
	walkLeft.looping = true;

	Animation walkRight;
	walkRight.frames = { {0, 0}, {1, 0}, {2, 0} };
	walkRight.frameTime = 0.1f;
	walkRight.looping = true;

	Animation walkUp;
	walkUp.frames = { {0, 0}, {1, 0}, {2, 0} };
	walkUp.frameTime = 0.1f;
	walkUp.looping = true;

	// add animations to the animator
	animator->AddAnimation("WalkDown", walkDown);
	animator->AddAnimation("WalkLeft", walkLeft);
	animator->AddAnimation("WalkRight", walkRight);
	animator->AddAnimation("WalkUp", walkUp);
	animator->SetDefaultFrame({ 1, 0 });

	// add player to level manager
	LevelManager::GetInstance().AddPlayer(*balloom->GetComponent<PlayerComponent>());

	return balloom;
}

std::unique_ptr<dae::GameObject> Prefab::CreateBalloom()
{
	// create the balloom object
	auto balloomEnemy = std::make_unique<dae::GameObject>();

	// add related components
	balloomEnemy->AddComponent<TextureComponent>("BalloomSpriteSheet.tga");
	balloomEnemy->AddComponent<SpriteAnimatorComponent>(32, 32);

	// add collider
	SDL_FRect collider = { 0.f,0.f, 29.f, 29.f };
	balloomEnemy->AddComponent<ColliderComponent>(collider);

	// create the animator
	auto animator = balloomEnemy->GetComponent<SpriteAnimatorComponent>();

	//create the animations
	Animation floatLeft;
	floatLeft.frames = { {0, 1}, {1, 1}, {2, 1} };
	floatLeft.frameTime = 0.15f;
	floatLeft.looping = true;

	Animation floatRight;
	floatRight.frames = { {0, 0}, {1, 0}, {2, 0} };
	floatRight.frameTime = 0.15f;
	floatRight.looping = true;

	Animation deathAnimation;
	deathAnimation.frames = { {0, 2}, {1, 2}, {2, 2}, {3, 2}, {4, 2} };
	deathAnimation.frameTime = 0.2f;
	deathAnimation.looping = false;
	deathAnimation.destroyAfterAnimation = true;

	// add the anims to the animator
	animator->AddAnimation("MoveLeft", floatLeft);
	animator->AddAnimation("MoveRight", floatRight);
	animator->AddAnimation("DeathAnimation", deathAnimation);

	animator->SetDefaultFrame({ 0, 1 });

	balloomEnemy->SetPosition(LevelManager::GetInstance().GetGrid()->GetWorldPosition({ 15, 5 }).x, LevelManager::GetInstance().GetGrid()->GetWorldPosition({ 15, 5 }).y);

	// add enemy comp
	balloomEnemy->AddComponent<EnemyComponent>(EnemyType::Balloom);

	// set tag
	balloomEnemy->SetTag("Enemy");

	return balloomEnemy;
}

std::unique_ptr<dae::GameObject> Prefab::CreateOneal()
{
	// create the oneal object
	auto onealEnemy = std::make_unique<dae::GameObject>();

	// add related components
	onealEnemy->AddComponent<TextureComponent>("OnealSpriteSheet.tga");
	onealEnemy->AddComponent<SpriteAnimatorComponent>(32, 32);

	// add collider
	SDL_FRect collider = { 0.f,0.f, 29.f, 29.f };
	onealEnemy->AddComponent<ColliderComponent>(collider);

	// create the animator
	auto animator = onealEnemy->GetComponent<SpriteAnimatorComponent>();

	// create the animations
	Animation moveLeft;
	moveLeft.frames = { {3, 0}, {4, 0}, {5, 0} };
	moveLeft.frameTime = 0.15f;
	moveLeft.looping = true;

	Animation moveRight;
	moveRight.frames = { {0, 0}, {1, 0}, {2, 0} };
	moveRight.frameTime = 0.15f;
	moveRight.looping = true;

	Animation deathAnimation;
	deathAnimation.frames = { {6, 0}, {6, 0}, {6, 0} };
	deathAnimation.frameTime = 0.2f;
	deathAnimation.looping = false;
	deathAnimation.destroyAfterAnimation = true;

	// add the anims to the animator
	animator->AddAnimation("MoveLeft", moveLeft);
	animator->AddAnimation("MoveRight", moveRight);
	animator->AddAnimation("DeathAnimation", deathAnimation);

	animator->SetDefaultFrame({ 0, 1 });

	// add enemy comp
	onealEnemy->AddComponent<EnemyComponent>(EnemyType::Oneal);

	// set tag
	onealEnemy->SetTag("Enemy");

	return onealEnemy;
}

std::unique_ptr<dae::GameObject> Prefab::CreateDoll()
{
	auto dollEnemy = std::make_unique<dae::GameObject>();

	dollEnemy->AddComponent<TextureComponent>("DollSpriteSheet.tga");
	dollEnemy->AddComponent<SpriteAnimatorComponent>(32, 32);

	SDL_FRect collider = { 0.f,0.f, 29.f, 29.f };
	dollEnemy->AddComponent<ColliderComponent>(collider);

	auto animator = dollEnemy->GetComponent<SpriteAnimatorComponent>();

	Animation moveLeft;
	moveLeft.frames = { {3, 0}, {4, 0}, {5, 0} };
	moveLeft.frameTime = 0.15f;
	moveLeft.looping = true;

	Animation moveRight;
	moveRight.frames = { {0, 0}, {1, 0}, {2, 0} };
	moveRight.frameTime = 0.15f;
	moveRight.looping = true;

	Animation deathAnimation;
	deathAnimation.frames = { {6, 0}, {7, 0}, {8, 0} };
	deathAnimation.frameTime = 0.2f;
	deathAnimation.looping = false;
	deathAnimation.destroyAfterAnimation = true;

	animator->AddAnimation("MoveLeft", moveLeft);
	animator->AddAnimation("MoveRight", moveRight);
	animator->AddAnimation("DeathAnimation", deathAnimation);
	animator->SetDefaultFrame({ 0, 1 });

	dollEnemy->AddComponent<EnemyComponent>(EnemyType::Doll);
	dollEnemy->SetTag("Enemy");

	return dollEnemy;
}

std::unique_ptr<dae::GameObject> Prefab::CreateMinvo()
{
	auto minvoEnemy = std::make_unique<dae::GameObject>();

	minvoEnemy->AddComponent<TextureComponent>("MinvoSpriteSheet.tga");
	minvoEnemy->AddComponent<SpriteAnimatorComponent>(32, 32);

	SDL_FRect collider = { 0.f,0.f, 29.f, 29.f };
	minvoEnemy->AddComponent<ColliderComponent>(collider);

	auto animator = minvoEnemy->GetComponent<SpriteAnimatorComponent>();

	Animation moveLeft;
	moveLeft.frames = { {3, 0}, {4, 0}, {5, 0} };
	moveLeft.frameTime = 0.15f;
	moveLeft.looping = true;

	Animation moveRight;
	moveRight.frames = { {0, 0}, {1, 0}, {2, 0} };
	moveRight.frameTime = 0.15f;
	moveRight.looping = true;

	Animation deathAnimation;
	deathAnimation.frames = { {6, 0}, {7, 0}, {8, 0} };
	deathAnimation.frameTime = 0.2f;
	deathAnimation.looping = false;
	deathAnimation.destroyAfterAnimation = true;

	animator->AddAnimation("MoveLeft", moveLeft);
	animator->AddAnimation("MoveRight", moveRight);
	animator->AddAnimation("DeathAnimation", deathAnimation);
	animator->SetDefaultFrame({ 0, 1 });

	minvoEnemy->AddComponent<EnemyComponent>(EnemyType::Minvo);
	minvoEnemy->SetTag("Enemy");

	return minvoEnemy;
}

std::unique_ptr<dae::GameObject> Prefab::CreateBomb(const glm::vec2& playerPos, int explosionRange, int playerIdx)
{
	auto bomb = std::make_unique<dae::GameObject>();

	bomb->SetTag("BombObject");

	auto grid = LevelManager::GetInstance().GetGrid();
	if (!grid) return bomb;

	TileCoords tileCoords = grid->GetTileFromWorldPosition(playerPos);

	TileType tile = grid->GetTile(tileCoords);
	if (tile != TileType::Floor) return bomb;

	auto bombPos = grid->GetWorldPosition(tileCoords);

	bomb->AddComponent<TextureComponent>("BombSpriteSheet.tga");
	bomb->AddComponent<SpriteAnimatorComponent>(32, 32);
	bomb->AddComponent<BombComponent>(explosionRange);


	bomb->SetPosition(bombPos.x, bombPos.y);


	auto animator = bomb->GetComponent<SpriteAnimatorComponent>();

	// create the animations
	Animation explode;
	explode.frames = { {0, 0}, {1, 0}, {2, 0} };
	explode.frameTime = 0.40f;
	explode.looping = true;

	// add the anims to the animator
	animator->AddAnimation("Explode", explode);
	animator->SetDefaultFrame({ 0,0 });

	if (!LevelManager::GetInstance().GetPlayer(playerIdx)->HasRemote())
	{
		animator->Play("Explode");
	}

	return bomb;
}

