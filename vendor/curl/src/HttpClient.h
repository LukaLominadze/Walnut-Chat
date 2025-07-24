#pragma once

#include <vector>
#include <string>

#include <curl/curl.h>
#include <functional>
#include "Requests.h"

namespace Curl
{
	class HttpClient
	{
	public:
		HttpClient() = default;
		~HttpClient() = default;

		void AddHeader(const std::string& header);

		HttpResponse Get(const std::string& callUrl, const std::vector<Parameter>& params = std::vector<Parameter>(), std::vector<std::string> headers = std::vector<std::string>());
		HttpResponse Post(const std::string& callUrl, const std::string& data, const std::vector<Parameter>& params = std::vector<Parameter>(), std::vector<std::string> headers = std::vector<std::string>());
		HttpResponse Put(const std::string& callUrl, const std::string& data, const std::vector<Parameter>& params = std::vector<Parameter>(), std::vector<std::string> headers = std::vector<std::string>());
		HttpResponse Delete(const std::string& callUrl, const std::vector<Parameter>& params = std::vector<Parameter>(), std::vector<std::string> headers = std::vector<std::string>());
	private:
		std::vector<std::string> m_Headers;

		std::string m_ReadBuffer;
	};
}

