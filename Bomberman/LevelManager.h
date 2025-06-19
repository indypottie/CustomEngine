#pragma once
#include <string>

#include "Singleton.h"
#include <vector>

class HudManager;
class EnemyComponent;
class PlayerComponent;
class ExplosionComponent;
class GridComponent;

struct HighScoreEntry
{
    std::string name;
    int score;
};

class LevelManager final : public dae::Singleton<LevelManager>
{
public:
    LevelManager();

    void SetGrid(GridComponent* grid) { m_Grid = grid; }
    GridComponent* GetGrid() const { return m_Grid; }

    void SetExplosionSystem(ExplosionComponent* explosionComponent) { m_ExplosionSystem = explosionComponent; }
    ExplosionComponent* GetExplosionSystem() const { return m_ExplosionSystem; }

    void AddPlayer(PlayerComponent& player);
    PlayerComponent* GetPlayer(int index) const;

    void AddEnemy(EnemyComponent& enemy);
    void RemoveEnemy(const EnemyComponent& enemy);

    void AddPoints(int points);
    int GetCurrentScore() const { return m_CurrentScore; }

    void FoundExit() { m_FoundExit = true; }

    void TryAdvanceLevel();
    void LoadLevel(int levelIndex);

    bool IsGameWon() const { return m_GameWon; }

    void LoadSoloGame();
    void LoadCoOpGame();
    void LoadVersusGame();

    void LoadScores();
    void SaveScores();

	std::vector<HighScoreEntry> GetHighScores() const;
    void AddHighScore(const std::string& name, int score);

    void EndGame();

    void RestartGame();

    void SkipLevel();

private:
    friend class dae::Singleton<LevelManager>;

    void SortAndTrim();
    void LoadLevelFromFile(const std::string& levelFile);

    std::vector<HighScoreEntry> m_Scores;
    const std::string m_SaveFile = "Resources/highscores.txt";
    static constexpr size_t MaxScores = 10;


    GridComponent* m_Grid{ nullptr }; // Non-owning pointer, managed elsewhere
    ExplosionComponent* m_ExplosionSystem{ nullptr }; // non-owning pointer

    std::vector<PlayerComponent*> m_Players;
    std::vector<EnemyComponent*> m_Enemies;

    bool m_CanExitLevel;
    bool m_FoundExit;
    int m_CurrentScore;
    bool m_GameWon = false;

    const HudManager& m_HudManager;

    int m_CurrentLevel = 0;
    static constexpr int m_MaxLevels = 3;
};
