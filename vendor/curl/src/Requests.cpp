#include "Requests.h"

namespace Curl
{
	size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
	{
		((std::string*)userp)->append((char*)contents, size * nmemb);
		return size * nmemb;
	}

	std::string AddParamsToUrl(const std::string& callUrl, const std::vector<Parameter>& params)
	{
		std::string _params = "";
		if (params.size() > 0) {
			const auto& param = params.at(0);
			_params += ("?" + param.Name + "=" + param.Value);
		}
		for (size_t i = 1; i < params.size(); i++) {
			const auto& param = params.at(i);
			_params += ("&" + param.Name + "=" + param.Value);
		}
		return callUrl + _params;
	}

	HttpResponse Get(const std::string& callUrl, const std::vector<Parameter>& params, const std::vector<std::string>& headers)
	{
		std::string url = AddParamsToUrl(callUrl, params);

		CURL* curl = curl_easy_init();

		std::string readBuffer;

		if (!curl) {
			return { StatusCodes::INVALID, readBuffer };
		}

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

		// Set headers
		struct curl_slist* _headers = NULL;
		if (!headers.empty()) {
			for (const std::string& header : headers) {
				_headers = curl_slist_append(_headers, header.c_str());
			}
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, _headers);
		}

		// Set write callback to capture response
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

		// Optional timeouts
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

		// SSL verification (optional: configure CA if needed)
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

		CURLcode result = curl_easy_perform(curl);

		long httpCode = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

		HttpResponse response = { static_cast<StatusCodes>(httpCode), readBuffer };

		if (result != CURLE_OK) {
			response.Response = curl_easy_strerror(result);
		}

		curl_slist_free_all(_headers);
		curl_easy_cleanup(curl);

		return response;
	}
	HttpResponse Post(const std::string& callUrl, const std::string& data, const std::vector<Parameter>& params, const std::vector<std::string>& headers)
	{
		std::string url = AddParamsToUrl(callUrl, params);

		CURL* curl = curl_easy_init();

		std::string readBuffer;

		if (!curl) {
			return { StatusCodes::INVALID, readBuffer };
		}

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

		// Set headers
		struct curl_slist* _headers = NULL;
		if (!headers.empty()) {
			for (const std::string& header : headers) {
				_headers = curl_slist_append(_headers, header.c_str());
			}
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, _headers);
		}

		// Enable POST and attach data
		curl_easy_setopt(curl, CURLOPT_POST, 1L);
		if (!data.empty()) {
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());

			// Optional: set length if needed
			curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.size());
		}

		// Set write callback to capture response
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

		// Optional timeouts
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

		// SSL verification (optional: configure CA if needed)
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

		CURLcode result = curl_easy_perform(curl);

		long httpCode = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

		HttpResponse response = { static_cast<StatusCodes>(httpCode), readBuffer };

		if (result != CURLE_OK) {
			response.Response = curl_easy_strerror(result);
		}

		curl_slist_free_all(_headers);
		curl_easy_cleanup(curl);

		return response;
	}
	HttpResponse Put(const std::string& callUrl, const std::string& data, const std::vector<Parameter>& params, const std::vector<std::string>& headers)
	{
		std::string url = AddParamsToUrl(callUrl, params);

		CURL* curl = curl_easy_init();

		std::string readBuffer;

		if (!curl) {
			return { StatusCodes::INVALID, readBuffer };
		}

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

		// Set headers
		struct curl_slist* _headers = NULL;
		if (!headers.empty()) {
			for (const std::string& header : headers) {
				_headers = curl_slist_append(_headers, header.c_str());
			}
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, _headers);
		}

		// Enable PUT and attach data
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
		if (!data.empty()) {
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());

			// Optional: set length if needed
			curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.size());
		}

		// Set write callback to capture response
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

		// Optional timeouts
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

		// SSL verification (optional: configure CA if needed)
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

		CURLcode result = curl_easy_perform(curl);

		long httpCode = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

		HttpResponse response = { static_cast<StatusCodes>(httpCode), readBuffer };

		if (result != CURLE_OK) {
			response.Response = curl_easy_strerror(result);
		}

		curl_slist_free_all(_headers);
		curl_easy_cleanup(curl);

		return response;
	}
	HttpResponse Delete(const std::string& callUrl, const std::vector<Parameter>& params, const std::vector<std::string>& headers)
	{
		std::string url = AddParamsToUrl(callUrl, params);

		CURL* curl = curl_easy_init();

		std::string readBuffer;

		if (!curl) {
			return { StatusCodes::INVALID, readBuffer };
		}

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

		// Set headers
		struct curl_slist* _headers = NULL;
		if (!headers.empty()) {
			for (const std::string& header : headers) {
				_headers = curl_slist_append(_headers, header.c_str());
			}
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, _headers);
		}

		// Set custom request to DELETE
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");

		// Set write callback to capture response
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

		// Optional timeouts
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

		// SSL verification (optional: configure CA if needed)
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

		CURLcode result = curl_easy_perform(curl);

		long httpCode = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

		HttpResponse response = { static_cast<StatusCodes>(httpCode), readBuffer };

		if (result != CURLE_OK) {
			response.Response = curl_easy_strerror(result);
		}

		curl_slist_free_all(_headers);
		curl_easy_cleanup(curl);

		return response;
	}
}