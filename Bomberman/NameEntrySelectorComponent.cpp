#include"NameEntrySelectorComponent.h"


#include <__msvc_ostream.hpp>

#include "GameObject.h"
#include "InputManager.h"
#include "LevelManager.h"
#include "TextComponent.h"


void NameEntrySelectorComponent::Init()
{
	SetTextComponent(GetOwner()->GetComponent<TextComponent>());
	UpdateDisplay();
}

void NameEntrySelectorComponent::SetTextComponent(TextComponent* textComponent)
{
	m_TextComponent = textComponent;
	UpdateDisplay();
}

void NameEntrySelectorComponent::Update()
{
	
}

void NameEntrySelectorComponent::SetScoreToSave(int score)
{
	m_ScoreToSave = score;
}

void NameEntrySelectorComponent::MoveLeft()
{
	if (m_Index > 0) --m_Index;
	UpdateDisplay();
}

void NameEntrySelectorComponent::MoveRight()
{
	if (m_Index < 2) ++m_Index;
	UpdateDisplay();
}

void NameEntrySelectorComponent::IncrementLetter()
{
	char& ch = m_Name[m_Index];
	ch = (ch == 'Z') ? 'A' : ch + 1;
	UpdateDisplay();
}

void NameEntrySelectorComponent::DecrementLetter()
{
	char& ch = m_Name[m_Index];
	ch = (ch == 'A') ? 'Z' : ch - 1;
	UpdateDisplay();
}

void NameEntrySelectorComponent::Confirm()
{
	const std::string name(m_Name.begin(), m_Name.end());
	LevelManager::GetInstance().AddHighScore(name, m_ScoreToSave);
	LevelManager::GetInstance().SaveScores();
	LevelManager::GetInstance().LoadScores();

	LevelManager::GetInstance().RestartGame();
}

void NameEntrySelectorComponent::UpdateDisplay()
{
	std::string display;
	for (int i = 0; i < 3; ++i)
	{
		if (i == m_Index)
		{
			display += "[" + std::string(1, m_Name[i]) + "] ";
		}
		else
		{
			display += " " + std::string(1, m_Name[i]) + "  ";
		}
	}

	if (m_TextComponent)
		m_TextComponent->SetText(display);
}
