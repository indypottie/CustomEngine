#pragma once
#include <memory>
#include <SDL.h>
#include <unordered_map>

#include "Command.h"
#include "Controller.h"
#include "Singleton.h"


enum class ControllerButton : uint8_t;

enum class keyState : uint8_t
{
	down,
	pressed,
	released
};

namespace dae
{
	class InputManager final : public Singleton<InputManager>
	{
	public:
		bool ProcessInput();

		void BindControllerCommand(int controllerIndex, ControllerButton button, keyState state, std::unique_ptr<Command> command);
		void BindKeyboardCommand(SDL_Keycode key, keyState state, std::unique_ptr<Command> command);

		void ClearCommands();

	private:

		std::unordered_map<int, std::unordered_map<ControllerButton, std::unique_ptr<Command>>> m_ControllerCommandsDown;
		std::unordered_map<int, std::unordered_map<ControllerButton, std::unique_ptr<Command>>> m_ControllerCommandsPressed;
		std::unordered_map<int, std::unordered_map<ControllerButton, std::unique_ptr<Command>>> m_ControllerCommandsReleased;

		std::unordered_map<int, std::unique_ptr<Controller>> m_Controllers;


		std::unordered_map<SDL_Keycode, std::unique_ptr<Command>> m_KeyCommandsDown;
		std::unordered_map<SDL_Keycode, std::unique_ptr<Command>> m_KeyCommandsPressed;
		std::unordered_map<SDL_Keycode, std::unique_ptr<Command>> m_KeyCommandsReleased;

		const Uint8* m_CurrentKeyState = nullptr;
		Uint8 m_PreviousKeyState[SDL_NUM_SCANCODES]{};

		bool m_Cleared{ false };
	};

}
