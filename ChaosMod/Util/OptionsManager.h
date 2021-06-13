#pragma once

#include "OptionsFile.h"

class OptionsManager
{
private:
	OptionsFile m_ConfigFile{ "chaosmod/config.ini" };
	OptionsFile m_TwitchFile{ "chaosmod/twitch.ini" };
	OptionsFile m_multiplayerFile{ "chaosmod/multiplayer.ini" };

public:
	void Reset()
	{
		m_ConfigFile.Reset();
		m_TwitchFile.Reset();
		m_multiplayerFile.Reset();
	}

	template <typename T>
	inline T GetConfigValue(const std::string& szKey, T defaultValue)
	{
		return GetOptionValue(m_ConfigFile, szKey, defaultValue);
	}

	template <typename T>
	inline T GetTwitchValue(const std::string& szKey, T defaultValue)
	{
		return GetOptionValue(m_TwitchFile, szKey, defaultValue);
	}

	template <typename T>
	inline T GetMultiplayerValue(const std::string& key, T defaultValue)
	{
		return GetOptionValue(m_multiplayerFile, key, defaultValue);
	}

private:
	template <typename T>
	inline T GetOptionValue(const OptionsFile& optionsFile, const std::string& szKey, T defaultValue = T())
	{
		if constexpr (std::is_same<std::remove_cv<T>::type, std::string>() || std::is_same<std::remove_cv<T>::type, char*>())
		{
			return optionsFile.ReadValueString(szKey, defaultValue);
		}
		else
		{
			return optionsFile.ReadValue(szKey, defaultValue);
		}
	}
};

inline OptionsManager g_OptionsManager;