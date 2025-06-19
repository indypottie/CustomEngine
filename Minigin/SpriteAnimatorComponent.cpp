//---------------------------
// Include Files
//---------------------------
#include "SpriteAnimatorComponent.h"

#include <SDL_rect.h>
#include <utility>

#include "EngineTime.h"
#include "GameObject.h"
#include "TextureComponent.h"

//---------------------------
// Constructor & Destructor
//---------------------------
SpriteAnimatorComponent::SpriteAnimatorComponent(dae::GameObject& owner, int frameWidth, int frameHeight) :
	Component(owner), m_FrameWidth(frameWidth), m_FrameHeight(frameHeight), m_DefaultFrame({1,1})
{
	m_TextureComponentPtr = owner.GetComponent<TextureComponent>();
	assert(m_TextureComponentPtr && "SpriteAnimatorComponent requires a TextureComponent");
}


//---------------------------
// Member functions
//---------------------------

void SpriteAnimatorComponent::Update()
{
	float deltaTime = EngineTime::GetInstance().GetDeltaTime();


	if (!m_Animations.contains(m_CurrentAnimation)) return;
	auto& anim = m_Animations[m_CurrentAnimation];

	if (m_AnimationFinished && anim.destroyAfterAnimation)
	{
		MarkForDeletion();
		GetOwner()->MarkForDeletion();
		return;
	}

	if (!m_IsPlaying) return;

	m_ElapsedTime += deltaTime;

	if (m_ElapsedTime >= anim.frameTime)
	{
		m_ElapsedTime -= anim.frameTime;
		++m_CurrentFrame;

		if (std::cmp_greater_equal(m_CurrentFrame, anim.frames.size()))
		{
			if (anim.looping) m_CurrentFrame = 0;

			else
			{
				m_CurrentFrame = static_cast<int>(anim.frames.size()) - 1;
				m_IsPlaying = false;
				m_AnimationFinished = true;
			}
		}

		ApplyFrame(anim.frames[m_CurrentFrame]);
	}
}

void SpriteAnimatorComponent::AddAnimation(const std::string& name, const Animation& animation)
{
	m_Animations[name] = animation;
}

void SpriteAnimatorComponent::Play(const std::string& name)
{
	if (m_Animations.contains(name))
	{
		if (m_CurrentAnimation == name && m_IsPlaying) return; // already playing

		m_CurrentAnimation = name;
		m_CurrentFrame = 0;
		m_ElapsedTime = 0.f;
		m_IsPlaying = true;
		m_AnimationFinished = false;
		ApplyFrame(m_Animations[name].frames[0]);
	}
}

void SpriteAnimatorComponent::Stop()
{
	m_IsPlaying = false;
	m_AnimationFinished = false;
	m_CurrentFrame = 0;
	m_ElapsedTime = 0.f;

	if (m_HasDefaultFrame)
	{
		ApplyFrame(m_DefaultFrame);
	}
	else if (!m_CurrentAnimation.empty() && m_Animations.contains(m_CurrentAnimation))
	{
		ApplyFrame(m_Animations[m_CurrentAnimation].frames[0]);
	}
}

void SpriteAnimatorComponent::SetDefaultFrame(const glm::ivec2& frame)
{
	m_DefaultFrame = frame;
	m_HasDefaultFrame = true;

	if (!m_IsPlaying)
		ApplyFrame(m_DefaultFrame);
}

bool SpriteAnimatorComponent::IsAnimationFinished() const
{
	return m_AnimationFinished;
}

void SpriteAnimatorComponent::ApplyFrame(const glm::ivec2& framePos)
{
	SDL_Rect srcRect
	{
		framePos.x * m_FrameWidth,
		framePos.y * m_FrameHeight,
		m_FrameWidth,
		m_FrameHeight
	};

	m_TextureComponentPtr->SetSourceRect(srcRect);
}
