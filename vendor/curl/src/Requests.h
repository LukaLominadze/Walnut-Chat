#pragma once
#include <curl/curl.h>

#include <vector>
#include <string>

#include "StatusCodes.h"

namespace Curl 
{
	struct HttpResponse {
		StatusCodes Code;
		std::string Response;
	};

	struct Parameter {
		std::string Name;
		std::string Value;
	};

	std::string AddParamsToUrl(const std::string& callUrl, const std::vector<Parameter>& params);

	HttpResponse Get(const std::string& callUrl, const std::vector<Parameter>& params, const std::vector<std::string>& headers);
	HttpResponse Post(const std::string& callUrl, const std::string& data, const std::vector<Parameter>& params, const std::vector<std::string>& headers);
	HttpResponse Put(const std::string& callUrl, const std::string& data, const std::vector<Parameter>& params, const std::vector<std::string>& headers);
	HttpResponse Delete(const std::string& callUrl, const std::vector<Parameter>& params, const std::vector<std::string>& headers);
}
