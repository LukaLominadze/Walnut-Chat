#pragma once

#include "Walnut/Layer.h"
#include "Walnut/Networking/Server.h"

#include "HeadlessConsole.h"
#include "../src/HttpClient.h"

#include "UserInfo.h"

#include <filesystem>

#include <array>

#include <curl/curl.h>
#include <future>

// TODO(Luka): Rewrite all of this to fit your needs

class ServerLayer : public Walnut::Layer
{
public:
	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate(float ts) override;
	virtual void OnUIRender() override {}
private:
	// Server event callbacks
	void OnClientConnected(const Walnut::ClientInfo& clientInfo);
	void OnClientDisconnected(const Walnut::ClientInfo& clientInfo);
	void OnDataReceived(const Walnut::ClientInfo& clientInfo, const Walnut::Buffer buffer);

	////////////////////////////////////////////////////////////////////////////////
	// Handle outgoing messages
	////////////////////////////////////////////////////////////////////////////////
	void SendClientList(const Walnut::ClientInfo& clientInfo);
	void SendClientConnect(const Walnut::ClientInfo& clientInfo);
	void SendClientConnectionRequestResponse(const Walnut::ClientInfo& clientInfo, bool response, const UserInfo& newClient, const std::string& authToken);
	void SendClientSessionRenewResponse(const Walnut::ClientInfo& clientInfo, const std::string& authToken);
	void SendClientDisconnect(const Walnut::ClientInfo& clientInfo);
	void SendClientKick(const Walnut::ClientInfo& clientInfo, std::string_view reason);
	void SendClientMessage(const Walnut::ClientID& clientId, const ChatMessage& message);
	void SendChatHistory(const Walnut::ClientInfo& clientInfo, const std::vector<ChatMessage> messages);
	////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////
	// Commands
	////////////////////////////////////////////////////////////////////////////////
	bool KickUser(std::string_view username, std::string_view reason = "");
	void Quit();
	////////////////////////////////////////////////////////////////////////////////

	bool IsValidUsername(const std::string& username) const;

	void SendChatMessage(std::string_view message);
	void OnCommand(std::string_view command);
private:
	bool ValidateSession(std::string& authToken, const std::string& username, const Walnut::ClientInfo& clientInfo);
private:
	std::unique_ptr<Walnut::Server> m_Server;
	HeadlessConsole m_Console;
	Curl::HttpClient m_HttpClient;

	Walnut::Buffer m_ScratchBuffer;

	std::map<Walnut::ClientID, UserInfo> m_ConnectedClients;
	// TODO: Figure out a better way to do this
	std::map<std::string, Walnut::ClientID> m_ConnectedClientIDs;
};