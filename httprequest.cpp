/**
 * vim: set ts=4 :
 * =============================================================================
 * SourceMod REST in Pawn Extension
 * Copyright 2017-2021 Erik Minekus
 * =============================================================================
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "httprequest.h"
#include "httprequestcontext.h"
#include "httpfilecontext.h"
#include "httpformcontext.h"

HTTPRequest::HTTPRequest(const std::string &url)
	: url(url)
{
}

void HTTPRequest::Perform(const char *method, json_t *data, IChangeableForward *forward, cell_t value)
{
	HTTPRequestContext *context = new HTTPRequestContext(std::string(method), BuildURL(), data, BuildHeaders("application/json", "application/json"), forward, value,
		connectTimeout, maxRedirects, timeout, maxSendSpeed, maxRecvSpeed, useBasicAuth, username, password);

	g_RipExt.AddRequestToQueue(context);
}

void HTTPRequest::DownloadFile(const char *path, IChangeableForward *forward, cell_t value)
{
	HTTPFileContext *context = new HTTPFileContext(false, BuildURL(), path, BuildHeaders("*/*", "application/octet-stream"), forward, value,
		connectTimeout, maxRedirects, timeout, maxSendSpeed, maxRecvSpeed, useBasicAuth, username, password);

	g_RipExt.AddRequestToQueue(context);
}

void HTTPRequest::UploadFile(const char *path, IChangeableForward *forward, cell_t value)
{
	HTTPFileContext *context = new HTTPFileContext(true, BuildURL(), path, BuildHeaders("*/*", "application/octet-stream"), forward, value,
		connectTimeout, maxRedirects, timeout, maxSendSpeed, maxRecvSpeed, useBasicAuth, username, password);

	g_RipExt.AddRequestToQueue(context);
}

void HTTPRequest::PostForm(IChangeableForward *forward, cell_t value)
{
	HTTPFormContext *context = new HTTPFormContext(BuildURL(), formData, BuildHeaders("application/json", "application/x-www-form-urlencoded"), forward, value,
		connectTimeout, maxRedirects, timeout, maxSendSpeed, maxRecvSpeed, useBasicAuth, username, password);

	g_RipExt.AddRequestToQueue(context);
}

const std::string HTTPRequest::BuildURL() const
{
	std::string url(this->url);
	url.append(query);

	return url;
}

void HTTPRequest::AppendQueryParam(const char *name, const char *value)
{
	CURL *curl = curl_easy_init();
	if (curl == NULL)
	{
		return;
	}

	char *escapedName = curl_easy_escape(curl, name, 0);
	char *escapedValue = curl_easy_escape(curl, value, 0);

	if (escapedName != NULL && escapedValue != NULL)
	{
		query.append(query.size() == 0 ? "?" : "&");
		query.append(escapedName);
		query.append("=");
		query.append(escapedValue);
	}

	curl_free(escapedName);
	curl_free(escapedValue);
	curl_easy_cleanup(curl);
}

void HTTPRequest::AppendFormParam(const char *name, const char *value)
{
	CURL *curl = curl_easy_init();
	if (curl == NULL)
	{
		return;
	}

	char *escapedName = curl_easy_escape(curl, name, 0);
	char *escapedValue = curl_easy_escape(curl, value, 0);

	if (escapedName != NULL && escapedValue != NULL)
	{
		formData.append(formData.size() == 0 ? "" : "&");
		formData.append(escapedName);
		formData.append("=");
		formData.append(escapedValue);
	}

	curl_free(escapedName);
	curl_free(escapedValue);
	curl_easy_cleanup(curl);
}

struct curl_slist *HTTPRequest::BuildHeaders(const char *acceptTypes, const char *contentType)
{
	// Accept and Content-Type headers must be located at the beginning!
	// If they are not located correctly, sending data will be lost.
	struct curl_slist *headers = NULL;
	char header[8192];

	auto result = this->headers.find("Accept");
	bool result_is_found = result.found();

	snprintf(header, sizeof(header), "Accept: %s", result_is_found ? (*result).value.c_str() : acceptTypes);
	headers = curl_slist_append(headers, header);

	if(result_is_found)
	{
		this->headers.remove(result);
	}

	result = this->headers.find("Content-Type");
	result_is_found = result.found();

	snprintf(header, sizeof(header), "Content-Type: %s", result_is_found ? (*result).value.c_str() : contentType);
	headers = curl_slist_append(headers, header);

	if(result_is_found)
	{
		this->headers.remove(result);
	}

	for (HTTPHeaderMap::iterator iter = this->headers.iter(); !iter.empty(); iter.next())
	{
		snprintf(header, sizeof(header), "%s: %s", iter->key.chars(), iter->value.c_str());
		headers = curl_slist_append(headers, header);
	}

	return headers;
}

void HTTPRequest::SetHeader(const char *name, const char *value)
{
	std::string vstr(value);
	headers.replace(name, std::move(vstr));
}

bool HTTPRequest::UseBasicAuth() const
{
	return useBasicAuth;
}

const std::string HTTPRequest::GetUsername() const
{
	return username;
}

const std::string HTTPRequest::GetPassword() const
{
	return password;
}

void HTTPRequest::SetBasicAuth(const char *username, const char *password)
{
	this->useBasicAuth = true;
	this->username = username;
	this->password = password;
}

int HTTPRequest::GetConnectTimeout() const
{
	return connectTimeout;
}

void HTTPRequest::SetConnectTimeout(int connectTimeout)
{
	this->connectTimeout = connectTimeout;
}

int HTTPRequest::GetMaxRedirects() const
{
	return maxRedirects;
}

void HTTPRequest::SetMaxRedirects(int maxRedirects)
{
	this->maxRedirects = maxRedirects;
}

int HTTPRequest::GetMaxRecvSpeed() const
{
	return maxRecvSpeed;
}

void HTTPRequest::SetMaxRecvSpeed(int maxSpeed)
{
	this->maxRecvSpeed = maxSpeed;
}

int HTTPRequest::GetMaxSendSpeed() const
{
	return maxSendSpeed;
}

void HTTPRequest::SetMaxSendSpeed(int maxSpeed)
{
	this->maxSendSpeed = maxSpeed;
}

int HTTPRequest::GetTimeout() const
{
	return timeout;
}

void HTTPRequest::SetTimeout(int timeout)
{
	this->timeout = timeout;
}
