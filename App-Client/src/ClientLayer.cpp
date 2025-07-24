#include "ClientLayer.h"

#include "ServerPacket.h"

#include "Walnut/Application.h"
#include "Walnut/UI/UI.h"
#include "Walnut/Serialization/BufferStream.h"
#include "Walnut/Networking/NetworkingUtils.h"
#include "Walnut/Utils/StringUtils.h"

#include "misc/cpp/imgui_stdlib.h"

#include <yaml-cpp/yaml.h>

#include <iostream>
#include <fstream>

void ClientLayer::OnAttach()
{
	m_ScratchBuffer.Allocate(1024);

	m_Client = std::make_unique<Walnut::Client>();
	m_Client->SetServerConnectedCallback([this]() { OnConnected(); });
	m_Client->SetServerDisconnectedCallback([this]() { OnDisconnected(); });
	m_Client->SetDataReceivedCallback([this](const Walnut::Buffer data) { OnDataReceived(data); });

	m_Console.SetMessageSendCallback([this](std::string_view message) { SendChatMessage(message); });

	LoadConnectionDetails(m_ConnectionDetailsFilePath);
	LoadUserDetails(m_UserDetailsPath);
}

void ClientLayer::OnDetach()
{
	m_Client->Disconnect();
	// ^ currently disconnect is blocking

	m_ScratchBuffer.Release();
}

void ClientLayer::OnUIRender()
{
	if (m_Disconnect) {
		m_Client->Disconnect();
		m_Disconnect = false;
	}
	UI_ConnectionModal();
	
	m_Console.OnUIRender();
	UI_ClientList();
}

bool ClientLayer::IsConnected() const
{
	return m_Client->GetConnectionStatus() == Walnut::Client::ConnectionStatus::Connected;
}

void ClientLayer::OnDisconnectButton()
{
	m_Client->Disconnect();
}

void ClientLayer::UI_ConnectionModal()
{
	if (!m_ConnectionModalOpen && m_Client->GetConnectionStatus() != Walnut::Client::ConnectionStatus::Connected)
	{
		ImGui::OpenPopup("Connect to server");
	}

	m_ConnectionModalOpen = ImGui::BeginPopupModal("Connect to server", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	if (m_ConnectionModalOpen)
	{
		ImGui::Text("Email");
		ImGui::InputText("##email", &m_Email);
		ImGui::Text("Password");
		ImGui::InputText("##password", &m_Password);
		ImGui::Text("Pick a color");
		ImGui::SameLine();
		ImGui::ColorEdit4("##color", m_ColorBuffer);

		ImGui::Text("Server Address");
		ImGui::InputText("##address", &m_ServerIP);
		ImGui::SameLine();
		if (ImGui::Button("Connect"))
		{
			m_Color = IM_COL32(m_ColorBuffer[0] * 255.0f, m_ColorBuffer[1] * 255.0f, m_ColorBuffer[2] * 255.0f, m_ColorBuffer[3] * 255.0f);

			if (Walnut::Utils::IsValidIPAddress(m_ServerIP))
			{
				m_Client->ConnectToServer(m_ServerIP);
			}
			else
			{
				// Try resolve domain name
				auto ipTokens = Walnut::Utils::SplitString(m_ServerIP, ':'); // [0] == hostname, [1] (optional) == port
				std::string serverIP = Walnut::Utils::ResolveDomainName(ipTokens[0]);
				if (ipTokens.size() != 2)
					serverIP = fmt::format("{}:{}", serverIP, 8192); // Add default port if hostname doesn't contain port
				else
					serverIP = fmt::format("{}:{}", serverIP, ipTokens[1]); // Add specified port

				m_Client->ConnectToServer(serverIP);
			}

		}

		if (Walnut::UI::ButtonCentered("Quit"))
			Walnut::Application::Get().Close();

		if (m_Client->GetConnectionStatus() == Walnut::Client::ConnectionStatus::Connected)
		{
			// Send username
			Walnut::BufferStreamWriter stream(m_ScratchBuffer);
			stream.WriteRaw<PacketType>(PacketType::ClientConnectionRequest);
			stream.WriteString(m_Email);
			stream.WriteString(m_Password);
			stream.WriteRaw<uint32_t>(m_Color); // Color

			m_Client->SendBuffer(stream.GetBuffer());

			// Wait for response
			ImGui::CloseCurrentPopup();
		}
		else if (m_Client->GetConnectionStatus() == Walnut::Client::ConnectionStatus::FailedToConnect)
		{
			ImGui::TextColored(ImVec4(0.9f, 0.2f, 0.1f, 1.0f), "Connection failed.");
			const auto& debugMessage = m_Client->GetConnectionDebugMessage();
			if (!debugMessage.empty())
				ImGui::TextColored(ImVec4(0.9f, 0.2f, 0.1f, 1.0f), debugMessage.c_str());
		}
		else if (m_Client->GetConnectionStatus() == Walnut::Client::ConnectionStatus::Connecting)
		{
			ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Connecting...");
		}

		ImGui::EndPopup();
	}
}

void ClientLayer::UI_ClientList()
{
	ImGui::Begin("Users Online");
	ImGui::Text("Online: %d", m_ConnectedClients.size());

	static std::string selected("");
	for (const auto& [username, clientInfo] : m_ConnectedClients)
	{
		if (username.empty())
			continue;

		if (ImGui::Selectable(username.c_str(), selected == username)) {
			if (selected == username)
				selected = "";
			else
			{
				m_SelectedUser = username;
				SendGetChatHistoryRequest(m_SelectedUser);
				selected = username;
			}
		}
	}
	ImGui::End();
}

void ClientLayer::OnConnected()
{
	m_Console.ClearLog();
	// Welcome message sent in PacketType::ClientConnectionRequest response handling
}

void ClientLayer::OnDisconnected()
{
	m_Console.AddItalicMessageWithColor(0xff8a8a8a, "Lost connection to server!");
}

void ClientLayer::OnDataReceived(const Walnut::Buffer buffer)
{
	Walnut::BufferStreamReader stream(buffer);

	PacketType type;
	stream.ReadRaw<PacketType>(type);

	switch (type)
	{
		case PacketType::ClientConnectionRequest:
		{
			bool requestStatus;
			stream.ReadRaw<bool>(requestStatus);
			if (requestStatus)
			{
				std::string authToken;
				if (!stream.ReadString(authToken)) {
					m_Disconnect = true;
					break;
				}
				UserInfo clientDetails;
				stream.ReadObject(clientDetails);
				if (clientDetails.Username.length() == std::string::npos) {
					m_Disconnect = true;
					break;
				}

				// Defer connection message to after message history is received
				m_ShowSuccessfulConnectionMessage = true;
				m_Console.AddItalicMessageWithColor(0xff8a8a8a, "Successfully connected to {}", m_ServerIP);

				m_Username = clientDetails.Username;
				m_AuthToken = authToken;

				SaveConnectionDetails(m_ConnectionDetailsFilePath);
				SaveUserDetails(m_UserDetailsPath);
			}
			else
			{
				m_Console.AddItalicMessageWithColor(0xfffa4a4a, "Server rejected connection with username {}", m_Username);
				m_Disconnect = true;
			}
			break;
		}
		case PacketType::ClientConnect:
		{
			UserInfo newClient;
			stream.ReadObject(newClient);

			m_ConnectedClients[newClient.Username] = newClient;
			m_Console.AddItalicMessage("Welcome {}!", newClient.Username);

			break;
		}
		case PacketType::ClientDisconnect:
		{
			UserInfo disconnectedClient;
			stream.ReadObject(disconnectedClient);

			if (m_ConnectedClients.contains(disconnectedClient.Username)) {
				m_ConnectedClients.erase(disconnectedClient.Username);
				m_Console.AddItalicMessage("{} disconnected!", disconnectedClient.Username);
			}

			break;
		}
		case PacketType::ClientList:
		{
			std::vector<UserInfo> clientList;
			stream.ReadArray(clientList);

			// Update our client list
			m_ConnectedClients.clear();
			for (const auto& client : clientList)
				m_ConnectedClients[client.Username] = client;

			break;
		}
		case PacketType::ClientSessionRenewResponse:
		{
			stream.ReadString(m_AuthToken);
			break;
		}
		case PacketType::MessageHistory:
		{
			std::vector<ChatMessage> messages;
			stream.ReadArray(messages);
			if (messages.size() > 0) {
				for (const auto& message : messages) {
					uint32_t color = m_ConnectedClients[message.Username].Color;
					m_Console.AddTaggedMessageWithColor(color, message.Username, message.Message);
				}
			}
			break;
		}
		case PacketType::Message:
		{
			ChatMessage message;
			stream.ReadObject(message);
			uint32_t color = m_ConnectedClients[message.Username].Color;
			m_Console.AddTaggedMessageWithColor(color, message.Username, message.Message);
			break;
		}
	}
}

void ClientLayer::SendChatMessage(std::string_view message)
{
	std::string messageToSend(message);
	if (IsValidMessage(messageToSend) && !m_SelectedUser.empty())
	{
		Walnut::BufferStreamWriter stream(m_ScratchBuffer);
		stream.WriteRaw<PacketType>(PacketType::Message);
		stream.WriteString(m_AuthToken);
		stream.WriteString(m_Username);
		stream.WriteString(m_SelectedUser);
		stream.WriteString(messageToSend);
		m_Client->SendBuffer(stream.GetBuffer());

		// echo in own console
		m_Console.AddTaggedMessageWithColor(m_Color | 0xff000000, m_Username, messageToSend);
	}
}

void ClientLayer::SendGetChatHistoryRequest(const std::string& receiver)
{
	m_Console.ClearLog();
	Walnut::BufferStreamWriter stream(m_ScratchBuffer);
	stream.WriteRaw<PacketType>(PacketType::MessageHistory);
	stream.WriteString(m_AuthToken);
	stream.WriteString(receiver);
	m_Client->SendBuffer(stream.GetBuffer());
}

void ClientLayer::SaveConnectionDetails(const std::filesystem::path& filepath)
{
	YAML::Emitter out;
	{
		out << YAML::BeginMap; // Root
		out << YAML::Key << "ConnectionDetails" << YAML::Value;

		out << YAML::BeginMap;
		out << YAML::Key << "Email" << YAML::Value << m_Email;
		out << YAML::Key << "Password" << YAML::Value << m_Password;
		out << YAML::Key << "Color" << YAML::Value << m_Color;
		out << YAML::Key << "ServerIP" << YAML::Value << m_ServerIP;
		out << YAML::EndMap;

		out << YAML::EndMap; // Root
	}

	std::ofstream fout(filepath);
	fout << out.c_str();
}

bool ClientLayer::LoadConnectionDetails(const std::filesystem::path& filepath)
{
	if (!std::filesystem::exists(filepath))
		return false;

	YAML::Node data;
	try
	{
		data = YAML::LoadFile(filepath.string());
	}
	catch (YAML::ParserException e)
	{
		std::cout << "[ERROR] Failed to load message history " << filepath << std::endl << e.what() << std::endl;
		return false;
	}

	auto rootNode = data["ConnectionDetails"];
	if (!rootNode)
		return false;

	m_Email = rootNode["Email"].as<std::string>();
	m_Password = rootNode["Password"].as<std::string>();
	m_Color = rootNode["Color"].as<uint32_t>();
	ImVec4 color = ImColor(m_Color).Value;
	m_ColorBuffer[0] = color.x;
	m_ColorBuffer[1] = color.y;
	m_ColorBuffer[2] = color.z;
	m_ColorBuffer[3] = color.w;
	m_ServerIP = rootNode["ServerIP"].as<std::string>();

	return true;
}

void ClientLayer::SaveUserDetails(const std::filesystem::path& filepath)
{
	YAML::Emitter out;
	{
		out << YAML::BeginMap; // Root
		out << YAML::Key << "UserDetails" << YAML::Value;

		out << YAML::BeginMap;
		out << YAML::Key << "Username" << YAML::Value << m_Username;
		out << YAML::Key << "AuthToken" << YAML::Value << m_AuthToken;
		out << YAML::EndMap;

		out << YAML::EndMap; // Root
	}

	std::ofstream fout(filepath);
	fout << out.c_str();
}

bool ClientLayer::LoadUserDetails(const std::filesystem::path& filepath)
{
	if (!std::filesystem::exists(filepath))
		return false;

	YAML::Node data;
	try
	{
		data = YAML::LoadFile(filepath.string());
	}
	catch (YAML::ParserException e)
	{
		std::cout << "[ERROR] Failed to load message history " << filepath << std::endl << e.what() << std::endl;
		return false;
	}

	auto rootNode = data["UserDetails"];
	if (!rootNode)
		return false;

	if (!m_ConnectedClients.contains(m_Username))
		return false;

	auto& clientDetails = m_ConnectedClients.at(m_Username);

	m_AuthToken = rootNode["AuthToken"].as<std::string>();

	return true;
}

