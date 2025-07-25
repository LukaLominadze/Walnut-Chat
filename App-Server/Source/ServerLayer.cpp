#include "ServerLayer.h"

#include "ServerPacket.h"

#include "Walnut/Core/Assert.h"
#include "Walnut/Serialization/BufferStream.h"

#include "Walnut/Utils/StringUtils.h"
#include "Walnut/Utils/json.hpp"
#include "Walnut/ApplicationHeadless.h"

#include "../src/Requests.h"

#include <yaml-cpp/yaml.h>

#include <iostream>
#include <fstream>

#if WL_PLATFORM_WINDOWS
#include <windows.h>

PROCESS_INFORMATION gProcessInfo = {};
#elif WL_PLATFORM_LINUX
// TODO:
#endif

void ServerLayer::OnAttach()
{
	const int Port = 8192;

	m_ScratchBuffer.Allocate(8192); // 8KB for now? probably too small for things like the client list/chat history

	m_Server = std::make_unique<Walnut::Server>(Port);
	m_Server->SetClientConnectedCallback([this](const Walnut::ClientInfo& clientInfo) { OnClientConnected(clientInfo); });
	m_Server->SetClientDisconnectedCallback([this](const Walnut::ClientInfo& clientInfo) { OnClientDisconnected(clientInfo); });
	m_Server->SetDataReceivedCallback([this](const Walnut::ClientInfo& clientInfo, const Walnut::Buffer data) { OnDataReceived(clientInfo, data); });
	m_Server->Start();

	m_Console.AddTaggedMessage("Info", "Started server on port {}", Port);

	m_Console.SetMessageSendCallback([this](std::string_view message) { SendChatMessage(message); });

	curl_global_init(CURL_GLOBAL_DEFAULT);

	m_HttpClient.AddHeader("Content-Type: application/json");
	m_HttpClient.AddHeader("Accept: application/json");

	// TODO: make this cross-platform and cleaner
#if WL_PLATFORM_WINDOWS
	STARTUPINFOA si = { sizeof(STARTUPINFOA) };
	ZeroMemory(&gProcessInfo, sizeof(PROCESS_INFORMATION));

	std::string command = "dotnet run --project Walnut.Api --launch-profile https";

	BOOL success = CreateProcessA(
		nullptr,                      // lpApplicationName
		&command[0],                  // lpCommandLine
		nullptr,                      // lpProcessAttributes
		nullptr,                      // lpThreadAttributes
		FALSE,                        // bInheritHandles
		CREATE_NEW_CONSOLE,          // dwCreationFlags
		nullptr,                      // lpEnvironment
		"../Walnut-Chat-Api",           // lpCurrentDirectory
		&si,                          // lpStartupInfo
		&gProcessInfo                 // lpProcessInformation
	);

	if (!success) {
		std::cerr << "CreateProcess failed. Error: " << GetLastError() << "\n";
	}

	std::cout << "Process started with PID: " << gProcessInfo.dwProcessId << "\n";
#endif
}

void ServerLayer::OnDetach()
{
	m_Server->Stop();
	// Wait for server to shutdown
	while (!m_Server->HasShutdown());
	m_ScratchBuffer.Release();
	curl_global_cleanup();
#if WL_PLATFORM_WINDOWS
	if (gProcessInfo.hProcess) {
		TerminateProcess(gProcessInfo.hProcess, 0);
		CloseHandle(gProcessInfo.hProcess);
		CloseHandle(gProcessInfo.hThread);
		std::cout << "Process terminated.\n";
	}
#endif
}

void ServerLayer::OnUpdate(float ts)
{
}

void ServerLayer::OnClientConnected(const Walnut::ClientInfo& clientInfo)
{
	std::cout << "Client connected! -> " << clientInfo.ID << std::endl;
}

void ServerLayer::OnClientDisconnected(const Walnut::ClientInfo& clientInfo)
{
	if (m_ConnectedClients.contains(clientInfo.ID))
	{
		SendClientDisconnect(clientInfo);
		const auto& userInfo = m_ConnectedClients.at(clientInfo.ID);
		m_Console.AddItalicMessage("Client {} disconnected", userInfo.Username);
		m_ConnectedClientIDs.erase(userInfo.Username);
		m_ConnectedClients.erase(clientInfo.ID);
	}
	else
	{
		std::cout << "[ERROR] OnClientDisconnected - Could not find client with ID=" << clientInfo.ID << std::endl;
		std::cout << "  ConnectionDesc=" << clientInfo.ConnectionDesc << std::endl;
	}
	//SendClientList(clientInfo);
}

// TODO: Figure out a better way to do async
void ServerLayer::OnDataReceived(const Walnut::ClientInfo& clientInfo, const Walnut::Buffer buffer)
{
	Walnut::BufferStreamReader stream(buffer);

	PacketType type;
	bool success = stream.ReadRaw<PacketType>(type);
	WL_CORE_VERIFY(success);
	if (!success) // Why couldn't we read packet type? Probs invalid packet
		return;

	switch (type) {
		case PacketType::ClientConnectionRequest: {
			std::string email;
			std::string password;
			uint32_t color;
			if (!stream.ReadString(email)) {
				return;
			}
			if (!stream.ReadString(password)) {
				return;
			}
			stream.ReadRaw<uint32_t>(color);
			
			std::async(std::launch::async,
				[this, email, password, color, clientInfo]() {

				nlohmann::json jsonData;
				jsonData["email"] = email;
				jsonData["password"] = password;

				// login 
				Curl::HttpResponse response = m_HttpClient.Post("https://localhost:7003/api/Auth/login", jsonData.dump());

				if (response.Code != Curl::StatusCodes::OK200) {
					std::string log = "HTTP " + std::to_string(static_cast<int>(response.Code)) + ": " + response.Response;
					m_Console.AddTaggedMessage("API", log);
					UserInfo invalid;
					SendClientConnectionRequestResponse(clientInfo, false, invalid, "");
					return;
				}

				YAML::Node data(YAML::Load(response.Response));

				std::string username = data["username"].as<std::string>();
				std::string authToken = data["authToken"].as<std::string>();

				bool isValidUsername = IsValidUsername(username);
				if (!isValidUsername) {
					return;
				}

				UserInfo& client = m_ConnectedClients[clientInfo.ID];
				client.Username = username;
				client.Color = color;
				m_ConnectedClientIDs[username] = clientInfo.ID;

				SendClientConnectionRequestResponse(clientInfo, isValidUsername, client, authToken);

				// connection complete? notify everyone else
				SendClientConnect(clientInfo);

				// Send the new client info about other connected clients
				SendClientList(clientInfo);
			});
			break;
		}
		case PacketType::MessageHistory:
		{
			std::string authToken;
			std::string receiver;
			stream.ReadString(authToken);
			stream.ReadString(receiver);
			if (!ValidateSession(authToken, m_ConnectedClients[clientInfo.ID].Username, clientInfo)) {
				break;
			}
			std::vector<std::string> headers = {
				"Authorization: Bearer " + authToken
			};
			std::vector<Curl::Parameter> params = {
				{ "receiverUsername", receiver }
			};
			Curl::HttpResponse response = m_HttpClient.Get("https://localhost:7003/api/Message", params, headers);
			if (response.Code != Curl::StatusCodes::OK200) {
				std::string log = "HTTP " + std::to_string(static_cast<int>(response.Code)) + ": " + response.Response;
				std::cout << "[API] " << log << std::endl;
				return;
			}
			YAML::Node root = YAML::Load(response.Response);
			std::vector<ChatMessage> messages;
			for (const auto& node : root) {
				messages.emplace_back(node["senderUsername"].as<std::string>(), node["content"].as<std::string>());
			}
			SendChatHistory(clientInfo, messages);
			break;
		}
		case PacketType::Message:
		{
			std::string authToken;
			std::string username;
			std::string receiver;
			std::string message;
			// TODO: Probably should send the client a message in case of invalid data
			if (!stream.ReadString(authToken)) {
				break;
			}
			stream.ReadString(username);
			stream.ReadString(receiver);
			stream.ReadString(message);
			if (!ValidateSession(authToken, m_ConnectedClients[clientInfo.ID].Username, clientInfo)) {
				break;
			}
			std::async(std::launch::async,
				[this, clientInfo = clientInfo, authToken = authToken,
				username = username, receiver = receiver, message = message]() {
				m_Console.AddTaggedMessage(username, message);
				std::vector<std::string> headers = {
					"Authorization: Bearer " + authToken
				};
				std::vector<Curl::Parameter> params;
				nlohmann::json jsonData;
				jsonData["receiverUsername"] = receiver;
				jsonData["content"] = message;
				Curl::HttpResponse response = m_HttpClient.Post("https://localhost:7003/api/Message", jsonData.dump(), params, headers);
				if (response.Code != Curl::StatusCodes::OK200) {
					std::string log = "HTTP " + std::to_string(static_cast<int>(response.Code)) + ": " + response.Response;
					std::cout << "[API] " << log << std::endl;
					return;
				}
				SendClientMessage(m_ConnectedClientIDs.at(receiver), { username, message });
				});
			break;
		}
	}
}

void ServerLayer::SendClientList(const Walnut::ClientInfo& clientInfo)
{
	std::vector<UserInfo> clientList(m_ConnectedClients.size());
	uint32_t index = 0;
	for (const auto& [clientID, clientInfo] : m_ConnectedClients)
		clientList[index++] = clientInfo;

	Walnut::BufferStreamWriter stream(m_ScratchBuffer);
	stream.WriteRaw<PacketType>(PacketType::ClientList);
	stream.WriteArray(clientList);

	// WL_INFO("Sending client list to all clients");
	m_Server->SendBufferToClient(clientInfo.ID, Walnut::Buffer(m_ScratchBuffer, stream.GetStreamPosition()));
}

void ServerLayer::SendClientConnect(const Walnut::ClientInfo& newClient)
{
	WL_VERIFY(m_ConnectedClients.contains(newClient.ID));
	const auto& newClientInfo = m_ConnectedClients.at(newClient.ID);

	Walnut::BufferStreamWriter stream(m_ScratchBuffer);
	stream.WriteRaw<PacketType>(PacketType::ClientConnect);
	stream.WriteObject(newClientInfo);

	m_Server->SendBufferToAllClients(Walnut::Buffer(m_ScratchBuffer, stream.GetStreamPosition()), newClient.ID);
}

void ServerLayer::SendClientConnectionRequestResponse(const Walnut::ClientInfo& clientInfo, bool response, const UserInfo& newClient, const std::string& authToken)
{
	Walnut::BufferStreamWriter stream(m_ScratchBuffer);
	stream.WriteRaw<PacketType>(PacketType::ClientConnectionRequest);
	stream.WriteRaw<bool>(response);
	if (response == true) {
		stream.WriteString(authToken);
		stream.WriteObject(newClient);
	}

	m_Server->SendBufferToClient(clientInfo.ID, Walnut::Buffer(m_ScratchBuffer, stream.GetStreamPosition()));
}

void ServerLayer::SendClientSessionRenewResponse(const Walnut::ClientInfo& clientInfo, const std::string& authToken)
{
	Walnut::BufferStreamWriter stream(m_ScratchBuffer);
	stream.WriteRaw<PacketType>(PacketType::ClientSessionRenewResponse);
	stream.WriteString(authToken);

	m_Server->SendBufferToClient(clientInfo.ID, Walnut::Buffer(m_ScratchBuffer, stream.GetStreamPosition()));
}

void ServerLayer::SendClientDisconnect(const Walnut::ClientInfo& clientInfo)
{
	WL_VERIFY(m_ConnectedClients.contains(clientInfo.ID));
	const auto newClientInfo = m_ConnectedClients.at(clientInfo.ID);

	Walnut::BufferStreamWriter stream(m_ScratchBuffer);
	stream.WriteRaw<PacketType>(PacketType::ClientDisconnect);
	stream.WriteObject(newClientInfo);

	m_Server->SendBufferToAllClients(Walnut::Buffer(m_ScratchBuffer, stream.GetStreamPosition()), clientInfo.ID);
}

void ServerLayer::SendClientKick(const Walnut::ClientInfo& clientInfo, std::string_view reason)
{
	Walnut::BufferStreamWriter stream(m_ScratchBuffer);
	stream.WriteRaw<PacketType>(PacketType::ClientKick);
	stream.WriteString(std::string(reason));

	m_Server->SendBufferToClient(clientInfo.ID, Walnut::Buffer(m_ScratchBuffer, stream.GetStreamPosition()));
}

void ServerLayer::SendClientMessage(const Walnut::ClientID& clientId, const ChatMessage& message)
{
	if (!m_ConnectedClients.contains(clientId)) {
		m_Console.AddTaggedMessage("SERVER", "Client {} not found to send message", clientId);
	}
	Walnut::BufferStreamWriter stream(m_ScratchBuffer);
	stream.WriteRaw<PacketType>(PacketType::Message);
	stream.WriteObject<ChatMessage>(message);

	m_Server->SendBufferToClient(clientId, Walnut::Buffer(m_ScratchBuffer, stream.GetStreamPosition()));
}

void ServerLayer::SendChatHistory(const Walnut::ClientInfo& clientInfo, const std::vector<ChatMessage> messages)
{
	Walnut::BufferStreamWriter stream(m_ScratchBuffer);
	stream.WriteRaw<PacketType>(PacketType::MessageHistory);
	stream.WriteArray<ChatMessage>(messages);

	m_Server->SendBufferToClient(clientInfo.ID, Walnut::Buffer(m_ScratchBuffer, stream.GetStreamPosition()));
}

bool ServerLayer::KickUser(std::string_view username, std::string_view reason)
{
	for (const auto& [clientID, userInfo] : m_ConnectedClients)
	{
		if (userInfo.Username == username)
		{
			Walnut::ClientInfo clientInfo = { clientID, "" };
			SendClientKick(clientInfo, reason);
			m_Server->KickClient(clientID);
			OnClientDisconnected(clientInfo);
			return true;
		}
	}

	// Could not find user with requested username
	return false;
}

void ServerLayer::Quit()
{
	m_Server->Stop();
	while (!m_Server->HasShutdown());
	Walnut::Application::Get().Close();
}

bool ServerLayer::IsValidUsername(const std::string& username) const
{
	for (const auto& [id, client] : m_ConnectedClients)
	{
		if (client.Username == username)
			return false;
	}

	return true;
}

void ServerLayer::SendChatMessage(std::string_view message)
{
	if (message.size() == 0)
		return;
	if (message[0] == '/')
	{
		// Try to run command instead
		OnCommand(message);
		return;
	}
	m_Console.AddTaggedMessage("SERVER", message);
}

void ServerLayer::OnCommand(std::string_view command)
{
	if (command.size() < 2 || command[0] != '/')
		return;

	std::string_view commandStr(&command[1], command.size() - 1);

	auto tokens = Walnut::Utils::SplitString(commandStr, ' ');
	if (tokens[0] == "stop") {
		std::string_view reason = "";
		std::string temp = "";
		for (size_t i = 1; i < tokens.size(); i++) {
			temp += tokens[i];
		}
		reason = std::string_view(temp);
		m_Console.AddItalicMessage("Stopping Server..");
		if (!reason.empty())
			m_Console.AddItalicMessage("  Reason: {}", reason);
		Quit();
	}
	else if (tokens[0] == "kick")
	{
		if (tokens.size() == 2 || tokens.size() == 3)
		{
			std::string_view reason = tokens.size() == 3 ? tokens[2] : "";
			if (KickUser(tokens[1], reason))
			{
				m_Console.AddItalicMessage("User {} has been kicked.", tokens[1]);
				if (!reason.empty())
					m_Console.AddItalicMessage("  Reason: {}", reason);
			}
			else
			{
				m_Console.AddItalicMessage("Could not kick user {}; user not found.", tokens[1]);
			}
		}
		else
		{
			m_Console.AddItalicMessage("Kick command requires single argument, eg. /kick <username>");
		}
	}
	else
	{
		m_Console.AddItalicMessage("Invalid command");
	}
}

// TODO: maybe use it as async?
bool ServerLayer::ValidateSession(std::string& authToken, const std::string& username, const Walnut::ClientInfo& clientInfo)
{
	// token validation
	std::vector<std::string> headers{
		"Authorization: Bearer " + authToken
	};
	std::vector<Curl::Parameter> params;
	Curl::HttpResponse response = m_HttpClient.Get("https://localhost:7003/api/Auth/validate", params, headers);
	if (response.Code != Curl::StatusCodes::OK200) {
		nlohmann::json jsonData;
		jsonData["authToken"] = authToken;

		m_Console.AddTaggedMessage("SERVER", "Renewing Session? -> " + username);

		response = m_HttpClient.Put("https://localhost:7003/api/Auth/refresh", jsonData.dump());
		if (response.Code != Curl::StatusCodes::OK200) {
			KickUser(m_ConnectedClients[clientInfo.ID].Username, "Session Expired");
			return false;
		}

		const auto& tokens = Walnut::Utils::SplitString(response.Response, "\"");
		authToken = tokens[0];

		m_Console.AddTaggedMessage("SERVER", "Renewed session -> " + username);

		SendClientSessionRenewResponse(clientInfo, authToken);
		return true;
	}
	return true;
}
