/* Copyright 2013-2017 Sathya Laufer
 *
 * libhomegear-base is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 * 
 * libhomegear-base is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with libhomegear-base.  If not, see
 * <http://www.gnu.org/licenses/>.
 * 
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU Lesser General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
*/

#ifndef HTTP_H_
#define HTTP_H_

#include "../Variable.h"
#include "../Exception.h"
#include "../HelperFunctions/Math.h"

#include <iostream>
#include <string>
#include <map>
#include <cstring>
#include <memory>
#include <vector>
#include <iomanip>

namespace BaseLib
{
class HttpException : public BaseLib::Exception
{
private:
	int32_t _responseCode = -1;
public:
	HttpException(std::string message) : Exception(message) {}
	HttpException(std::string message, int32_t responseCode) : Exception(message), _responseCode(responseCode) {}

	int32_t responseCode() { return _responseCode; }
};

class Http
{
public:
	struct Type
	{
		enum Enum { none, request, response };
	};
	struct TransferEncoding
	{
		enum Enum { none = 0, chunked = 1, compress = 2, deflate = 4, gzip = 8, identity = 16 };
	};
	struct Connection
	{
		enum Enum { none = 0, keepAlive = 1, close = 2, upgrade = 4 };
	};
	struct Protocol
	{
		enum Enum { none, http10, http11 };
	};
	struct Header
	{
		bool parsed = false;
		std::string method;
		Protocol::Enum protocol = Protocol::Enum::none;
		int32_t responseCode = -1;
		uint32_t contentLength = 0;
		std::string path;
		std::string pathInfo;
		std::string args;
		std::string host;
		std::string contentType;
		std::string contentTypeFull;
		TransferEncoding::Enum transferEncoding = TransferEncoding::Enum::none;
		Connection::Enum connection = Connection::Enum::none;
		std::string authorization;
		std::string cookie;
		std::unordered_map<std::string, std::string> cookies;
		std::string remoteAddress;
		int32_t remotePort = 0;
		std::map<std::string, std::string> fields;
	};

	Http();
	virtual ~Http();

	Type::Enum getType() { return _type; }
	bool headerIsFinished() { return _header.parsed; }
	bool isFinished() { return _finished; }

	/**
	 * This method sets _finished and terminates _content with a null character. Use it, when the header does not contain "Content-Length".
	 *
	 * @see isFinished()
	 * @see _finished
	 */
	void setFinished();
	std::vector<char>& getRawHeader() { return _rawHeader; }
	std::vector<char>& getContent() { return _content; }
	uint32_t getContentSize() { return _content.empty() ? 0 : (_finished ? _content.size() - 1 : _content.size()); }
	Header& getHeader() { return _header; }
	void reset();

	/**
	 * Parses HTTP data from a buffer.
	 *
	 * @param buffer The buffer to parse
	 * @param bufferLength The size of the buffer
	 * @param checkForChunkedXml (Optional, default "false") Only works for XML-like content (content needs to start with '<'). Needed when TransferEncoding is not set to chunked.
	 * @param checkForChunkedJson (Optional, default "false") Only works for JSON-like content (content needs to start with '{' or '['). Needed when TransferEncoding is not set to chunked.
	 * @return The number of processed bytes.
	 */
	int32_t process(char* buffer, int32_t bufferLength, bool checkForChunkedXml = false, bool checkForChunkedJson = false);
	bool headerProcessingStarted() { return _headerProcessingStarted; }
	bool dataProcessingStarted() { return _dataProcessingStarted; }
	static std::string encodeURL(const std::string& url);
	static std::string decodeURL(const std::string& url);
	size_t readStream(char* buffer, size_t requestLength);
	size_t readContentStream(char* buffer, size_t requestLength);
	size_t readFirstContentLine(char* buffer, size_t requestLength);
	std::string getMimeType(std::string extension);
	std::string getStatusText(int32_t code);
	static void constructHeader(uint32_t contentLength, std::string contentType, int32_t code, std::string codeDescription, std::vector<std::string>& additionalHeaders, std::string& header);
	PVariable serialize();
	void unserialize(PVariable data);
private:
	bool _contentLengthSet = false;
	bool _headerProcessingStarted = false;
	bool _dataProcessingStarted = false;
	bool _crlf = true;
	Header _header;
	std::vector<char> _rawHeader;
	Type::Enum _type = Type::Enum::none;
	std::vector<char> _content;
	std::vector<char> _chunk;
	bool _finished = false;
	int32_t _chunkSize = -1;
	int32_t _endChunkSizeBytes = -1;
	std::string _partialChunkSize;
	size_t _streamPos = 0;
	size_t _contentStreamPos = 0;
	std::map <std::string, std::string> _extMimeTypeMap;
	std::map <int32_t, std::string> _statusCodeMap;

	int32_t processHeader(char** buffer, int32_t& bufferLength);
	void processHeaderField(char* name, uint32_t nameSize, char* value, uint32_t valueSize);
	int32_t processContent(char* buffer, int32_t bufferLength);
	int32_t processChunkedContent(char* buffer, int32_t bufferLength);
	void readChunkSize(char** buffer, int32_t& bufferLength);

	int32_t strnaicmp(char const *a, char const *b, uint32_t size);
};
}
#endif
