//---------------------------
// Include Files
//---------------------------
#include "TextureComponent.h"

#include <iostream>

#include "GameObject.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "TransformComponent.h"

//---------------------------
// Constructor & Destructor
//---------------------------
TextureComponent::TextureComponent(dae::GameObject& owner, const std::string& fileName) : Component(owner)
{
	m_Texture = dae::ResourceManager::GetInstance().LoadTexture(fileName);
}

//---------------------------
// Member functions
//---------------------------

// Write member functions here
void TextureComponent::SetTexture(const std::string& fileName)
{
	m_Texture = dae::ResourceManager::GetInstance().LoadTexture(fileName);
}

void TextureComponent::SetSourceRect(const SDL_Rect& srcRect)
{
	m_SourceRect = srcRect;
}

void TextureComponent::ClearSourceRect()
{
	m_SourceRect.reset();
}

void TextureComponent::Render() const
{
	if (!m_Visible) return;

	auto owner = GetOwner();

	if (owner and m_Texture)
	{
		auto pos = owner->GetPosition();

		if (m_SourceRect.has_value())
		{
			dae::Renderer::GetInstance().RenderTexture(*m_Texture, pos.x, pos.y, *m_SourceRect);
		}
		else
		{
			dae::Renderer::GetInstance().RenderTexture(*m_Texture, pos.x, pos.y);
		}
	}
}

