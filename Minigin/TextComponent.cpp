#include "TextComponent.h"

#include <stdexcept>
#include <SDL_ttf.h>

#include "Renderer.h"
#include "Font.h"
#include "GameObject.h"
#include "Texture2D.h"
#include "Transform.h"
#include "TransformComponent.h"

//---------------------------
// Constructor
//---------------------------
TextComponent::TextComponent(dae::GameObject& owner, std::string text, dae::Font* font, SDL_Color color)
	: Component(owner), m_NeedsUpdate(true), m_Text(std::move(text)), m_Font(font), m_Color(color), m_TextTexture(nullptr)
{
	if (!m_Font)
	{
		throw std::runtime_error("Font cannot be null in TextComponent");
	}
}

//---------------------------
// Member functions
//---------------------------

void TextComponent::Update()
{
    if (m_NeedsUpdate)
    {
        SDL_Surface* surf = TTF_RenderText_Blended(m_Font->GetFont(), m_Text.c_str(), m_Color);
        if (!surf)
        {
            throw std::runtime_error(std::string("Render text failed: ") + SDL_GetError());
        }

        SDL_Texture* texture = SDL_CreateTextureFromSurface(dae::Renderer::GetInstance().GetSDLRenderer(), surf);
        SDL_FreeSurface(surf);

        if (!texture)
        {
            throw std::runtime_error(std::string("Create text texture from surface failed: ") + SDL_GetError());
        }

        // Assign new texture ownership
        m_TextTexture = std::make_unique<dae::Texture2D>(texture);
        m_NeedsUpdate = false;
    }
}

void TextComponent::Render() const
{
    if (m_TextTexture)
    {
        auto* transform = GetOwner() ? GetOwner()->GetComponent<TransformComponent>() : nullptr;

        if (transform)
        {
            const auto& pos = transform->GetWorldPosition();
            dae::Renderer::GetInstance().RenderTexture(*m_TextTexture, pos.x, pos.y);
        }
        else
        {
            dae::Renderer::GetInstance().RenderTexture(*m_TextTexture, 0.f, 0.f);
        }
    }
}

void TextComponent::SetText(const std::string& text)
{
    if (m_Text != text)
    {
        m_Text = text;
        m_NeedsUpdate = true;
    }
}
