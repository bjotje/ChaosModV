#include <stdafx.h>
#include "MultiplayerClient.h"

#define BUFFER_SIZE 256

MultiplayerClient::MultiplayerClient(const std::array<BYTE, 3>& rgTextColor) : m_rgTextColor(rgTextColor)
{
	LOG("Going to actually initialize the multiplayer client");
	m_enableMultiplayerClient = g_OptionsManager.GetMultiplayerValue<bool>("EnableMultiplayerClient", OPTION_DEFAULT_MULTIPLAYER_CLIENT_ENABLED);
	m_enableMultiplayerServer = g_OptionsManager.GetMultiplayerValue<bool>("EnableMultiplayerServer", OPTION_DEFAULT_MULTIPLAYER_SERVER_ENABLED);


	if (!m_enableMultiplayerClient && !m_enableMultiplayerServer)
	{
		LOG("Multiplayer is disabled, exiting");
		return;
	}

	if (m_enableMultiplayerClient)
	{
		//g_pEffectDispatcher->OverrideTimerDontDispatch(true);
		g_pEffectDispatcher->m_bDispatchEffectsOnTimer = false;
		LOG("Started multiplayer client");
	}
	else if (m_enableMultiplayerServer)
	{
		LOG("Started multiplayer server");
	}


	//g_effectDispatcher->OverrideenableNormalEffectDispatch(false);

	STARTUPINFO startupInfo = {};
	PROCESS_INFORMATION procInfo = {};

	char buffer[128];
	strcpy_s(buffer, "chaosmod\\SyncEffectProxy.exe");

	bool result = CreateProcess(NULL, buffer, NULL, NULL, TRUE, NULL, NULL, NULL, &startupInfo, &procInfo);

	//#ifdef _DEBUG || true
	//	DWORD attributes = NULL;
	//	if (DoesFileExist("chaosmod\\.forcenovotingconsole"))
	//	{
	//		attributes = CREATE_NO_WINDOW;
	//	}
	//
	//	bool result = CreateProcess(NULL, buffer, NULL, NULL, TRUE, attributes, NULL, NULL, &startupInfo, &procInfo);
	//#else
	//	bool result = CreateProcess(NULL, buffer, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &startupInfo, &procInfo);
	//#endif

		// A previous instance of the voting proxy could still be running, wait for it to release the mutex
	HANDLE mutex = OpenMutex(SYNCHRONIZE, FALSE, "ChaosModVMultiplayerClientMutex");
	if (mutex)
	{
		WaitForSingleObject(mutex, INFINITE);
		ReleaseMutex(mutex);
		CloseHandle(mutex);
	}

	if (!result)
	{
		ErrorOutWithMsg((std::ostringstream() << "Error while starting chaosmod/SyncEffectProxy.exe (Error Code: " << GetLastError() << "). Please verify the file exists. Reverting to normal mode.").str());

		return;
	}

	m_pipeHandle = CreateNamedPipe("\\\\.\\pipe\\ChaosModVMultiplayerClientPipe", PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_NOWAIT,
		1, BUFFER_SIZE, BUFFER_SIZE, 0, NULL);

	if (m_pipeHandle == INVALID_HANDLE_VALUE)
	{
		ErrorOutWithMsg("Error while creating a named pipe, previous instance of multiplayer client might be running. Try reloading the mod. Reverting to normal mode.");

		return;
	}

	ConnectNamedPipe(m_pipeHandle, NULL);


	LOG("Multiplayer client initialized!");
}

void MultiplayerClient::Run()
{
	//LOG("Tick!");
	// Check if there's been no ping for too long and error out
	// Also if the chance system is enabled, get current vote status every second (if shown on screen)
	DWORD64 curTick = GetTickCount64();
	if (m_lastPing < curTick - 1000)
	{
		if (m_noPingRuns == 5)
		{
			ErrorOutWithMsg("Connection to MultiplayerclientProxy aborted. Returning to normal mode.");

			return;
		}

		m_noPingRuns++;
		m_lastPing = curTick;
	}

	char buffer[BUFFER_SIZE];
	DWORD bytesRead;
	if (!ReadFile(m_pipeHandle, buffer, BUFFER_SIZE, &bytesRead, NULL))
	{
		while (GetLastError() == ERROR_IO_PENDING)
		{
			WAIT(0);
		}
	}

	if (bytesRead > 0)
	{
		if (!HandleMsg(std::string(buffer)))
		{
			return;
		}
	}

	if (!m_receivedHello)
	{
		return;
	}
}

void MultiplayerClient::DispatchEffect(EffectIdentifier effectIdentifier)
{
	try
	{
		if (m_enableMultiplayerServer)
		{
			//EffectData& effectData = g_enabledEffects.at(effectIdentifier);			
			EEffectType et = effectIdentifier.GetEffectType();

			LOG("Should dispatch effect: ");
			LOG("Dispatched effect effecttype:" << effectIdentifier.GetEffectType());
			std::string effectType = std::to_string(effectIdentifier.GetEffectType());

			SendToPipe("broadcasteffect;" + effectType);
		}
	}
	catch (const std::exception&)
	{
		LOG("Catch in dispatch mc");

	}

}

bool MultiplayerClient::HandleMsg(const std::string& msg)
{
	if (msg == "hello")
	{
		m_receivedHello = true;

		LOG("Received Hello from multiplayer client proxy");
	}
	else if (msg == "ping")
	{
		m_lastPing = GetTickCount64();
		m_noPingRuns = 0;
		m_receivedFirstPing = true;
	}
	else if (msg.starts_with("dispatcheffect;"))
	{
		if (m_enableMultiplayerClient)
		{
			std::string effectstring = msg.substr(15, msg.length());
			LOG("Received effect: " + effectstring);

			int effect = std::stoi(effectstring);

			EffectIdentifier effectIdentifier = EffectIdentifier(static_cast<EEffectType>(effect));
			g_pEffectDispatcher->DispatchEffect(effectIdentifier);
		}
	}
	else if (msg == "signalrclosed")
	{
		ErrorOutWithMsg("Connection to signalR server lost. Please restart chaosmod with ctrl+L");
	}

	return true;
}

template <class Container>
void split1(const std::string& str, Container& cont)
{
	std::istringstream iss(str);
	std::copy(std::istream_iterator<std::string>(iss),
		std::istream_iterator<std::string>(),
		std::back_inserter(cont));
}

void MultiplayerClient::SendToPipe(std::string&& msg)
{
	msg += "\n";
	WriteFile(m_pipeHandle, msg.c_str(), msg.length(), NULL, NULL);
}

void MultiplayerClient::ErrorOutWithMsg(const std::string&& msg)
{
	MessageBox(NULL, msg.c_str(), "ChaosModV Error", MB_OK | MB_ICONERROR);

	DisconnectNamedPipe(m_pipeHandle);
	CloseHandle(m_pipeHandle);
	m_pipeHandle = INVALID_HANDLE_VALUE;

	g_pEffectDispatcher->m_bDispatchEffectsOnTimer = false;
	m_enableMultiplayerClient = false;
}


MultiplayerClient::~MultiplayerClient()
{
	if (m_pipeHandle != INVALID_HANDLE_VALUE)
	{
		FlushFileBuffers(m_pipeHandle);
		DisconnectNamedPipe(m_pipeHandle);
		CloseHandle(m_pipeHandle);
	}

}