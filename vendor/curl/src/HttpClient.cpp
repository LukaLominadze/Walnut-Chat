#include "HttpClient.h"
#include "curl/curl.h"

namespace Curl
{
	void HttpClient::AddHeader(const std::string& header)
	{
		m_Headers.push_back(header);
	}

	HttpResponse HttpClient::Get(const std::string& callUrl, const std::vector<Parameter>& params, std::vector<std::string> headers)
	{
		headers.insert(headers.begin(), m_Headers.begin(), m_Headers.end());
		return Curl::Get(callUrl, params, headers);
	}

	HttpResponse HttpClient::Post(const std::string& callUrl, const std::string& data, const std::vector<Parameter>& params, std::vector<std::string> headers)
	{
		headers.insert(headers.begin(), m_Headers.begin(), m_Headers.end());
		return Curl::Post(callUrl, data, params, headers);
	}

	HttpResponse HttpClient::Put(const std::string& callUrl, const std::string& data, const std::vector<Parameter>& params, std::vector<std::string> headers)
	{
		headers.insert(headers.begin(), m_Headers.begin(), m_Headers.end());
		return Curl::Put(callUrl, data, params, headers);
	}

	HttpResponse HttpClient::Delete(const std::string& callUrl, const std::vector<Parameter>& params, std::vector<std::string> headers)
	{
		headers.insert(headers.begin(), m_Headers.begin(), m_Headers.end());
		return Curl::Delete(callUrl, params, headers);
	}
}
