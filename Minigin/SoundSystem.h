#pragma once

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------
#include <memory>
#include <string>

//-----------------------------------------------------
// SoundSystem Class									 
//-----------------------------------------------------
class SoundSystem
{
public:
	SoundSystem(); // Constructor
	virtual ~SoundSystem(); // Destructor

	SoundSystem(SoundSystem&&) noexcept;
	SoundSystem& operator=(SoundSystem&&) noexcept;

	//-------------------------------------------------
	// Member functions						
	//-------------------------------------------------

	// if loops is -1 sound will loop "infinitely", 0 = no loops
	virtual void QueueSound(const std::string& soundFile, int loops = 0);

	virtual void StopSound(const std::string& soundFile);

	virtual void Mute(bool mute);

private:
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------

	class SoundSystemImpl;
	std::unique_ptr<SoundSystemImpl> m_Impl;
};

// null sound system class
class NullSoundSystem final : public SoundSystem
{
public:
	NullSoundSystem() = default;
	~NullSoundSystem() override = default;

	void QueueSound(const std::string& /*soundFile*/, int /*loops*/) override {}
	void StopSound(const std::string& /*soundFile*/) override {}
	void Mute(bool /*mute*/) override {}
};