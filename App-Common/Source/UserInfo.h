#pragma once

#include "Walnut/Serialization/StreamReader.h"
#include "Walnut/Serialization/StreamWriter.h"

struct UserInfo
{
	std::string Username;
	uint32_t Color;

	static void Serialize(Walnut::StreamWriter* serializer, const UserInfo& instance)
	{
		serializer->WriteString(instance.Username);
		serializer->WriteRaw(instance.Color);
	}

	static void Deserialize(Walnut::StreamReader* deserializer, UserInfo& instance)
	{
		deserializer->ReadString(instance.Username);
		deserializer->ReadRaw(instance.Color);
	}
};

struct ChatMessage
{
	std::string Username;
	std::string Message;

	ChatMessage() = default;

	ChatMessage(const std::string& username, const std::string& message)
		: Username(username), Message(message) {}

	static void Serialize(Walnut::StreamWriter* serializer, const ChatMessage& instance)
	{
		serializer->WriteString(instance.Username);
		serializer->WriteString(instance.Message);
	}

	static void Deserialize(Walnut::StreamReader* deserializer, ChatMessage& instance)
	{
		deserializer->ReadString(instance.Username);
		deserializer->ReadString(instance.Message);
	}
};

const int MaxMessageLength = 4096;
bool IsValidMessage(std::string& message);
