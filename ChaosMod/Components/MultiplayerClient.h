#pragma once

#include "Component.h"
#include "EffectDispatcher.h"

#include <vector>
#include <memory>
#include <array>

#define _NODISCARD [[nodiscard]]

using DWORD64 = unsigned long long;
using BYTE = unsigned char;

using HANDLE = void*;

class MultiplayerClient : public Component
{

private:
	bool m_receivedHello = false;
	bool m_enableMultiplayerClient;
	bool m_enableMultiplayerServer;
	HANDLE m_pipeHandle = INVALID_HANDLE_VALUE;
	DWORD64 m_lastPing = GetTickCount64();
	int m_noPingRuns = 0;
	bool m_receivedFirstPing = false;
	std::array<BYTE, 3> m_rgTextColor;

	bool HandleMsg(const std::string& msg);
	void SendToPipe(std::string&& msg);
	void ErrorOutWithMsg(const std::string&& msg);
	

public:
	MultiplayerClient(const std::array<BYTE, 3>& rgTextColor);
	~MultiplayerClient();

	virtual void Run() override;

	_NODISCARD bool IsEnabled() const;

	inline bool IsClientEnabled() const
	{
		if (m_enableMultiplayerClient)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	void DispatchEffect(EffectIdentifier effectIdentifier);
};

inline std::unique_ptr<MultiplayerClient> g_multiplayerClient;