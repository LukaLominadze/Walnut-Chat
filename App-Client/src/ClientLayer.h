#pragma once

#include "Walnut/Layer.h"
#include "Walnut/Networking/Client.h"

#include "Walnut/UI/Console.h"

#include "UserInfo.h"

#include <set>
#include <filesystem>

class ClientLayer : public Walnut::Layer
{
public:
	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUIRender() override;

	bool IsConnected() const;
	void OnDisconnectButton();
private:
	// UI
	void UI_ConnectionModal();
	void UI_ClientList();

	// Server event callbacks
	void OnConnected();
	void OnDisconnected();
	void OnDataReceived(const Walnut::Buffer buffer);

	void SendChatMessage(std::string_view message);
	void SendGetChatHistoryRequest(const std::string& receiver);

private:
	void SaveConnectionDetails(const std::filesystem::path& filepath);
	bool LoadConnectionDetails(const std::filesystem::path& filepath);
	void SaveUserDetails(const std::filesystem::path& filepath);
	bool LoadUserDetails(const std::filesystem::path& filepath);
private:
	std::unique_ptr<Walnut::Client> m_Client;
	Walnut::UI::Console m_Console{ "Chat" };
	std::string m_ServerIP;
	std::filesystem::path m_ConnectionDetailsFilePath = "ConnectionDetails.yaml";
	std::filesystem::path m_UserDetailsPath = "UserDetails.yaml";

	Walnut::Buffer m_ScratchBuffer;

	float m_ColorBuffer[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	std::string m_Username, m_Email, m_Password, m_AuthToken, m_SelectedUser;
	uint32_t m_Color = 0xffffffff;

	std::map<std::string, UserInfo> m_ConnectedClients;
	bool m_ConnectionModalOpen = false;
	bool m_ShowSuccessfulConnectionMessage = false;
	bool m_Disconnect = false;
};