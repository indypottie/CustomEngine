#pragma once
#include <functional>
#include <vec2.hpp>

#include "Component.h"

class StartMenuSelectorComponent final : public Component
{
public:
    StartMenuSelectorComponent(dae::GameObject& owner);

	void Update() override;


    void AddOption(const glm::vec2& position, const std::function<void()>& onSelect);
    void MoveUp();
    void MoveDown();
    void Select() const;

private:
    void UpdateArrowPosition() const;
    void Init() const;

    std::vector<glm::vec2> m_OptionPositions;
    std::vector<std::function<void()>> m_OnSelectActions;
    int m_CurrentIndex = 0;
};
