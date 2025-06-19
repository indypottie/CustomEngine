#pragma once

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------
#include <memory>
#include <SDL_pixels.h>
#include <string>
#include "Component.h"
#include "Font.h"
#include "Texture2D.h"

//-----------------------------------------------------
// TextComponent Class									 
//-----------------------------------------------------
class TextComponent final : public Component
{
public:
    TextComponent(dae::GameObject& owner, std::string text, dae::Font* font, SDL_Color color = { 255, 255, 255, 255 }); // Constructor
    ~TextComponent() override = default; // Destructor

    // -------------------------
    // Copy/move constructors and assignment operators
    // -------------------------    
    TextComponent(const TextComponent& other) = delete;
    TextComponent(TextComponent&& other) noexcept = delete;
    TextComponent& operator=(const TextComponent& other) = delete;
    TextComponent& operator=(TextComponent&& other) noexcept = delete;

    //-------------------------------------------------
    // Member functions						
    //-------------------------------------------------
    void Update() override;
    void Render() const override;

    void SetText(const std::string& text);

    void SetColor(const SDL_Color& color) { m_Color = color; }
    SDL_Color GetColor() const { return m_Color; }

private:
    //-------------------------------------------------
    // Datamembers											
    //-------------------------------------------------
    bool m_NeedsUpdate;
    std::string m_Text;
    dae::Font* m_Font;            // Raw pointer, managed by ResourceManager
    SDL_Color m_Color;
    std::unique_ptr<dae::Texture2D> m_TextTexture; // Raw pointer, managed by ResourceManager
};
