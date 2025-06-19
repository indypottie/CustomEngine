#include <SDL.h>
#include "InputManager.h"

#include <ranges>

#include "Command.h"
#include "Controller.h"
#include "ControllerButtons.h"

bool dae::InputManager::ProcessInput()
{
	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		if (e.type == SDL_QUIT) {
			return false;
		}
	}

		m_CurrentKeyState = SDL_GetKeyboardState(nullptr);

		// Execute commands for key held down (continuously while pressed)
		for (const auto& [key, command] : m_KeyCommandsDown)
		{
			if (m_CurrentKeyState[SDL_GetScancodeFromKey(key)])
			{
				command->Execute();  // Execute continuously while key is held down
			}

			if (m_Cleared) break;
		}

		// Execute commands for key pressed (only the first time it is pressed)
		for (const auto& [key, command] : m_KeyCommandsPressed)
		{
			if (m_CurrentKeyState[SDL_GetScancodeFromKey(key)] && !m_PreviousKeyState[SDL_GetScancodeFromKey(key)])
			{
				command->Execute();  // Execute only once when key is pressed down for the first time
			}

			if (m_Cleared) break;
		}

		// Execute commands for key released (only when key is released)
		for (const auto& [key, command] : m_KeyCommandsReleased)
		{
			if (!m_CurrentKeyState[SDL_GetScancodeFromKey(key)] && m_PreviousKeyState[SDL_GetScancodeFromKey(key)])
			{
				command->Execute();  // Execute only once when key is released
			}

			if (m_Cleared) break;
		}

		// Store the current state as the previous state for the next frame
		memcpy(m_PreviousKeyState, m_CurrentKeyState, SDL_NUM_SCANCODES);
	


	for (auto& [index, controller] : m_Controllers)
	{
		controller->Update();

		for (const auto& [button, command] : m_ControllerCommandsDown[index])
		{
			if (controller->IsButtonDown(button))
			{
				command->Execute();
			}

			if (m_Cleared) break;
		}

		for (const auto& [button, command] : m_ControllerCommandsPressed[index])
		{
			if (controller->IsButtonPressed(button))
			{
				command->Execute();
			}

			if (m_Cleared) break;
		}

		for (const auto& [button, command] : m_ControllerCommandsReleased[index])
		{
			if (controller->IsButtonReleased(button))
			{
				command->Execute();
			}

			if (m_Cleared) break;

		}

		if (m_Cleared) break;
	}


	m_Cleared = false;

	return true;
}

void dae::InputManager::BindControllerCommand(int controllerIndex, ControllerButton button, keyState state, std::unique_ptr<Command> command)
{
	// Check if the controller exists, and if not, create it
	if (!m_Controllers.contains(controllerIndex))
	{
		m_Controllers[controllerIndex] = std::make_unique<Controller>(controllerIndex);  // Initialize the controller
	}

	switch (state)
	{
	case keyState::down:
		m_ControllerCommandsDown[controllerIndex][button] = std::move(command);
		break;

	case keyState::pressed:
		m_ControllerCommandsPressed[controllerIndex][button] = std::move(command);
		break;

	case keyState::released:
		m_ControllerCommandsReleased[controllerIndex][button] = std::move(command);
		break;
	}
}

void dae::InputManager::BindKeyboardCommand(SDL_Keycode key, keyState state, std::unique_ptr<Command> command)
{
	switch (state)
	{
	case keyState::down:
		m_KeyCommandsDown[key] = std::move(command);
		break;
	case keyState::pressed:
		m_KeyCommandsPressed[key] = std::move(command);
		break;
	case keyState::released:
		m_KeyCommandsReleased[key] = std::move(command);
		break;
	}
}

void dae::InputManager::ClearCommands()
{
	m_KeyCommandsDown.clear();
	m_KeyCommandsPressed.clear();
	m_KeyCommandsReleased.clear();

	m_ControllerCommandsDown.clear();
	m_ControllerCommandsPressed.clear();
	m_ControllerCommandsReleased.clear();

	m_Controllers.clear();

	m_Cleared = true;
}
