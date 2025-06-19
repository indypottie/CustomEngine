#pragma once
#include "Component.h"
#include <array>

class TextComponent;

class NameEntrySelectorComponent final : public Component
{
public:

    NameEntrySelectorComponent(dae::GameObject& owner) : Component(owner) {}


    void SetTextComponent(TextComponent* textComponent);

    void Update() override;

    void SetScoreToSave(int score);
    void MoveLeft();
    void MoveRight();
    void IncrementLetter();
    void DecrementLetter();
    void Confirm();

private:
    void Init();

    std::array<char, 3> m_Name{ 'A', 'A', 'A' };
    int m_Index{ 0 };
    int m_ScoreToSave{ 0 };

    void UpdateDisplay();

    TextComponent* m_TextComponent{ nullptr };
};
