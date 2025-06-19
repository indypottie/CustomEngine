#pragma once

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------
#include <map>
#include <string>
#include <vec2.hpp>
#include <vector>

#include "Component.h"


class TextureComponent;

struct Animation
{
	std::vector<glm::ivec2> frames; // vector of frames
	float frameTime = 0.1f; // time per frame
	bool looping = true; // whether the animation loops

	bool destroyAfterAnimation = false; // whether the gameObject will be destroyed after this animation is done playing
};

//-----------------------------------------------------
// SpriteAnimatorComponent Class									 
//-----------------------------------------------------
class SpriteAnimatorComponent final : public Component
{
public:
	SpriteAnimatorComponent(dae::GameObject& owner, int frameWidth, int frameHeight); // Constructor
	~SpriteAnimatorComponent() override = default; // Destructor

	// -------------------------
	// Copy/move constructors and assignment operators
	// -------------------------    
	SpriteAnimatorComponent(const SpriteAnimatorComponent& other)					= delete;
	SpriteAnimatorComponent(SpriteAnimatorComponent&& other) noexcept				= delete;
	SpriteAnimatorComponent& operator=(const SpriteAnimatorComponent& other)		= delete;
	SpriteAnimatorComponent& operator=(SpriteAnimatorComponent&& other) noexcept	= delete;

	//-------------------------------------------------
	// Member functions						
	//-------------------------------------------------
	void Update() override;

	void AddAnimation(const std::string& name, const Animation& animation);
	void Play(const std::string& name);
	void Stop();

	void SetDefaultFrame(const glm::ivec2& frame);

	bool IsAnimationFinished() const;

private:
	//-------------------------------------------------
	// Private member functions								
	//-------------------------------------------------
	void ApplyFrame(const glm::ivec2& framePos);

	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
	TextureComponent* m_TextureComponentPtr = nullptr; // ptr to texture component

	std::map<std::string, Animation> m_Animations; // map of animations
	std::string m_CurrentAnimation; // current animation name

	int m_FrameWidth;
	int m_FrameHeight;
	int m_CurrentFrame = 0; // current frame index

	float m_ElapsedTime = 0.f; // elapsed time since last frame
	bool m_IsPlaying = false; // whether the animation is playing
	bool m_AnimationFinished = false;

	glm::ivec2 m_DefaultFrame;
	bool m_HasDefaultFrame = false;
};
