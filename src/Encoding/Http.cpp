/* Copyright 2013-2019 Homegear GmbH
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

#include "Http.h"
#include "../HelperFunctions/Math.h"
#include "../HelperFunctions/HelperFunctions.h"
#include "JsonDecoder.h"

#include <iomanip>

namespace BaseLib {

const std::map<std::string, std::string> Http::_extMimeTypeMap = {
    {"html", "text/html"},
    {"htm", "text/html"},
    {"js", "text/javascript"},
    {"css", "text/css"},
    {"gif", "image/gif"},
    {"jpg", "image/jpeg"},
    {"jpeg", "image/jpeg"},
    {"jpe", "image/jpeg"},
    {"pdf", "application/pdf"},
    {"png", "image/png"},
    {"svg", "image/svg+xml"},
    {"txt", "text/plain"},
    {"webm", "video/webm"},
    {"ogv", "video/ogg"},
    {"ogg", "video/ogg"},
    {"3gp", "video/3gpp"},
    {"apk", "application/vnd.android.package-archive"},
    {"avi", "video/x-msvideo"},
    {"bmp", "image/x-ms-bmp"},
    {"csv", "text/comma-separated-values"},
    {"doc", "application/msword"},
    {"docx", "application/msword"},
    {"flac", "audio/flac"},
    {"gz", "application/x-gzip"},
    {"gzip", "application/x-gzip"},
    {"ics", "text/calendar"},
    {"kml", "application/vnd.google-earth.kml+xml"},
    {"kmz", "application/vnd.google-earth.kmz"},
    {"m4a", "audio/mp4"},
    {"mp3", "audio/mpeg"},
    {"mp4", "video/mp4"},
    {"mpg", "video/mpeg"},
    {"mpeg", "video/mpeg"},
    {"mov", "video/quicktime"},
    {"odp", "application/vnd.oasis.opendocument.presentation"},
    {"ods", "application/vnd.oasis.opendocument.spreadsheet"},
    {"odt", "application/vnd.oasis.opendocument.text"},
    {"oga", "audio/ogg"},
    {"pptx", "application/vnd.ms-powerpoint"},
    {"pps", "application/vnd.ms-powerpoint"},
    {"qt", "video/quicktime"},
    {"swf", "application/x-shockwave-flash"},
    {"tar", "application/x-tar"},
    {"text", "text/plain"},
    {"tif", "image/tiff"},
    {"tiff", "image/tiff"},
    {"wav", "audio/wav"},
    {"wmv", "video/x-ms-wmv"},
    {"xls", "application/vnd.ms-excel"},
    {"xlsx", "application/vnd.ms-excel"},
    {"zip", "application/zip"},
    {"xml", "application/xml"},
    {"xsl", "application/xml"},
    {"xsd", "application/xml"},

    {"xhtml", "application/xhtml+xml"},
    {"json", "application/json"},
    {"dtd", "application/xml-dtd"},
    {"xslt", "application/xslt+xml"},
    {"java", "text/x-java-source,java"}
};

const std::map<int32_t, std::string> Http::_statusCodeMap = {
    {100, "Continue"},
    {101, "Switching Protocols"},
    {200, "OK"},
    {201, "Created"},
    {202, "Accepted"},
    {203, "Non-Authoritative Information"},
    {204, "No Content"},
    {205, "Reset Content"},
    {206, "Partial Content"},
    {300, "Multiple Choices"},
    {301, "Moved Permanently"},
    {302, "Found"},
    {303, "See Other"},
    {304, "Not Modified"},
    {305, "Use Proxy"},
    {307, "Temporary Redirect"},
    {308, "Permanent Redirect"},
    {400, "Bad Request"},
    {401, "Unauthorized"},
    {402, "Payment Required"},
    {403, "Forbidden"},
    {404, "Not Found"},
    {405, "Method Not Allowed"},
    {406, "Not Acceptable"},
    {407, "Proxy Authentication Required"},
    {408, "Request Timeout"},
    {409, "Conflict"},
    {410, "Gone"},
    {411, "Length Required"},
    {412, "Precondition Failed"},
    {413, "Request Entity Too Large"},
    {414, "Request-URI Too Long"},
    {415, "Unsupported Media Type"},
    {416, "Requested Range Not Satisfiable"},
    {417, "Expectation Failed"},
    {426, "Upgrade Required"},
    {428, "Precondition Required"},
    {429, "Too Many Requests"},
    {431, "Request Header Fields Too Large"},
    {500, "Internal Server Error"},
    {501, "Not Implemented"},
    {502, "Bad Gateway"},
    {503, "Service Unavailable"},
    {504, "Gateway Timeout"},
    {505, "HTTP Version Not Supported"},
    {511, "Network Authentication Required"}
};

std::string Http::getMimeType(std::string extension) {
  auto extMimeTypeIterator = _extMimeTypeMap.find(extension);
  if (extMimeTypeIterator != _extMimeTypeMap.end()) return extMimeTypeIterator->second;
  return "";
}

std::string Http::getStatusText(int32_t code) {
  auto statusCodeIterator = _statusCodeMap.find(code);
  if (statusCodeIterator != _statusCodeMap.end()) return statusCodeIterator->second;
  return "";
}

std::set<std::shared_ptr<Http::FormData>> Http::decodeMultipartFormdata() {
  std::set<std::shared_ptr<FormData>> formData;
  if (_header.contentType != "multipart/form-data") return formData;

  std::string temp(_content.data(), _content.size());

  std::string boundary;
  std::vector<std::string> parts = HelperFunctions::splitAll(_header.contentTypeFull, ';');
  for (auto &part : parts) {
    auto arg = HelperFunctions::splitFirst(part, '=');
    HelperFunctions::trim(arg.first);
    if (arg.first == "boundary") {
      boundary = HelperFunctions::trim(arg.second);
      break;
    }
  }

  if (boundary.empty()) return formData;

  char *pos = _content.data();
  formData = decodeMultipartMixed(boundary, _content.data(), _content.size(), &pos);

  return formData;
}

char *Http::findNextString(std::string &needle, char *buffer, size_t bufferSize) {
  if (needle.size() > bufferSize) return nullptr;
  char *pos = buffer;
  while (pos < buffer + bufferSize) {
    pos = (char *)memchr((void *)pos, needle.at(0), bufferSize - (pos - buffer));
    if (pos == nullptr) return pos;

    size_t remainingBytes = bufferSize - (pos - buffer);
    if (needle.size() <= remainingBytes) {
      if (memcmp(pos, needle.data(), needle.size()) == 0) return pos;
    }
    pos++;
  }
  return pos;
}

std::set<std::shared_ptr<Http::FormData>> Http::decodeMultipartMixed(std::string &boundary, char *buffer, size_t bufferSize, char **pos) {
  std::set<std::shared_ptr<FormData>> formData;

  if (*pos < buffer) *pos = buffer;
  *pos = findNextString(boundary, *pos, bufferSize - (*pos - buffer));
  if (*pos == nullptr) return formData;
  *pos += boundary.size() + 2; //boundary + CRLF

  while (*pos < buffer + bufferSize) {
    char *startPos = *pos;
    char *endPos = findNextString(boundary, *pos, bufferSize - (*pos - buffer));
    if (endPos == nullptr) return formData;
    endPos -= 4; //CRLF + "--"

    //{{{ Decode block
    std::shared_ptr<FormData> blockData = std::make_shared<FormData>();

    uint32_t headerSize = 0;
    int32_t crlfOffset = 2;
    char *blockHeaderEnd = (char *)memmem(startPos, bufferSize - (startPos - buffer), "\r\n\r\n", 4);
    if (blockHeaderEnd == nullptr) {
      blockHeaderEnd = (char *)memmem(startPos, bufferSize - (startPos - buffer), "\n\n", 2);
      if (blockHeaderEnd == nullptr) return formData;
      crlfOffset = 1;
      headerSize = ((blockHeaderEnd + 1) - startPos) + 1;
    } else headerSize = ((blockHeaderEnd + 3) - startPos) + 1;

    char *newlinePos = startPos;
    char *colonPos = nullptr;
    while (*pos < startPos + headerSize) {
      newlinePos = (crlfOffset == 2) ? (char *)memchr(*pos, '\r', endPos - *pos) : (char *)memchr(*pos, '\n', endPos - *pos);
      if (!newlinePos || newlinePos > endPos) break;
      colonPos = (char *)memchr(*pos, ':', newlinePos - *pos);
      if (!colonPos || colonPos > newlinePos) {
        *pos = newlinePos + crlfOffset;
        continue;
      }

      if (colonPos < newlinePos - 1) {
        char *valuePos = colonPos + 1;
        uint32_t valueSize = newlinePos - colonPos - 1;
        while (*valuePos == ' ' && valueSize > 0) {
          //Skip whitespace
          valuePos++;
          valueSize--;
        }

        std::string name(*pos, (uint32_t)(colonPos - *pos));
        HelperFunctions::toLower(name);
        std::string value(valuePos, valueSize);

        blockData->header.emplace(name, value);
        if (name == "content-disposition") {
          blockData->contentDisposition = value;

          std::vector<std::string> parts;
          std::string args = HelperFunctions::splitFirst(value, ';').second;
          if (args.empty()) {
            args = HelperFunctions::splitFirst(value, ',').second;
            parts = HelperFunctions::splitAll(value, ',');
          } else parts = HelperFunctions::splitAll(value, ';');

          for (auto &part : parts) {
            auto arg = HelperFunctions::splitFirst(part, '=');
            HelperFunctions::trim(arg.first);
            HelperFunctions::toLower(arg.first);
            HelperFunctions::trim(arg.second);
            if (arg.second.size() > 1 && arg.second.front() == '"' && arg.second.back() == '"') arg.second = arg.second.substr(1, arg.second.size() - 2);
            if (arg.first == "name") blockData->name = arg.second;
            if (arg.first == "filename") blockData->filename = arg.second;
          }
        } else if (name == "content-type") {
          blockData->contentTypeFull = value;
          blockData->contentType = HelperFunctions::splitFirst(value, ',').first;
          blockData->contentType = HelperFunctions::splitFirst(value, ';').first;
          HelperFunctions::toLower(blockData->contentType);
        }
      }

      *pos = newlinePos + crlfOffset;
    }

    if (blockData->contentType == "multipart/mixed") {
      std::string innerBoundary;
      std::vector<std::string> parts = HelperFunctions::splitAll(blockData->contentTypeFull, blockData->contentTypeFull.find(',') == std::string::npos ? ';' : ',');
      for (auto &part : parts) {
        auto arg = HelperFunctions::splitFirst(part, '=');
        HelperFunctions::trim(arg.first);
        if (arg.first == "boundary") {
          innerBoundary = HelperFunctions::trim(arg.second);
          break;
        }
      }

      if (innerBoundary.empty()) continue;

      blockData->multipartMixed = decodeMultipartMixed(innerBoundary, startPos + headerSize, (endPos - (startPos + headerSize)) + 1, pos);
    } else {
      blockData->data = std::make_shared<std::vector<char>>(startPos + headerSize, endPos);
    }

    formData.emplace(blockData);
    //}}}

    *pos = endPos + 4 + boundary.size();
    if (*pos + 2 >= buffer + bufferSize) return formData;

    if (*(*pos) == '-' && *(*pos + 1) == '-') {
      *pos += 4; //"--" + CRLF
      return formData; //End
    } else *pos += 2; //CRLF
  }

  return formData;
}

void Http::constructHeader(uint32_t contentLength, std::string contentType, int32_t code, std::string codeDescription, const std::vector<std::string> &additionalHeaders, std::string &header) {
  std::string additionalHeader;
  additionalHeader.reserve(1024);
  for (auto &entry : additionalHeaders) {
    if (entry.find("Location: ") == 0) {
      code = 301;
      codeDescription = "Moved Permanently";
    }
    if (additionalHeader.size() + entry.size() > additionalHeader.capacity()) additionalHeader.reserve(additionalHeader.size() + entry.size() + 1024);
    if (!entry.empty()) additionalHeader.append(entry + "\r\n");
  }

  header.reserve(1024);
  header.append("HTTP/1.1 " + std::to_string(code) + " " + codeDescription + "\r\n");
  if (!contentType.empty()) header.append("Content-Type: " + contentType + "\r\n");
  header.append(additionalHeader);
  header.append("Content-Length: ").append(std::to_string(contentLength)).append("\r\n\r\n");
}

Http::Http() {
}

Http::~Http() {
}

PVariable Http::serialize() {
  PVariable data(new Variable(VariableType::tArray));
  data->arrayValue->reserve(11);
  data->arrayValue->emplace_back(std::make_shared<Variable>((int32_t)_type));                        // 0
  data->arrayValue->emplace_back(std::make_shared<Variable>(_finished));                            // 1
  data->arrayValue->emplace_back(std::make_shared<Variable>(_headerProcessingStarted));            // 2
  data->arrayValue->emplace_back(std::make_shared<Variable>(_dataProcessingStarted));                // 3
  data->arrayValue->emplace_back(std::make_shared<Variable>(_content));                            // 4
  data->arrayValue->emplace_back(std::make_shared<Variable>(_rawHeader));                            // 5
  data->arrayValue->emplace_back(std::make_shared<Variable>(_header.remoteAddress));                // 6
  data->arrayValue->emplace_back(std::make_shared<Variable>(_header.remotePort));                    // 7
  data->arrayValue->emplace_back(std::make_shared<Variable>(_redirectUrl));                        // 8
  data->arrayValue->emplace_back(std::make_shared<Variable>(_redirectQueryString));                // 9
  data->arrayValue->emplace_back(std::make_shared<Variable>(_redirectStatus));                    // 10
  return data;
}

void Http::unserialize(PVariable data) {
  if (!data) return;
  _type = (Type::Enum)data->arrayValue->at(0)->integerValue;
  _finished = data->arrayValue->at(1)->booleanValue;
  _headerProcessingStarted = data->arrayValue->at(2)->booleanValue;
  _dataProcessingStarted = data->arrayValue->at(3)->booleanValue;
  _content.insert(_content.end(), data->arrayValue->at(4)->binaryValue.begin(), data->arrayValue->at(4)->binaryValue.end());
  _rawHeader.insert(_rawHeader.end(), data->arrayValue->at(5)->binaryValue.begin(), data->arrayValue->at(5)->binaryValue.end());
  _header.remoteAddress = data->arrayValue->at(6)->stringValue;
  _header.remotePort = data->arrayValue->at(7)->integerValue;
  _redirectUrl = data->arrayValue->at(8)->stringValue;
  _redirectQueryString = data->arrayValue->at(9)->stringValue;
  _redirectStatus = data->arrayValue->at(10)->integerValue;

  int32_t headerSize = _rawHeader.size();
  char *pHeader = &(_rawHeader.at(0));
  processHeader(&pHeader, headerSize);
}

int32_t Http::process(char *buffer, int32_t bufferLength, bool checkForChunkedXml, bool checkForChunkedJson) {
  if (bufferLength <= 0) return 0;
  if (_finished) reset();
  _headerProcessingStarted = true;
  int32_t processedBytes = 0;
  if (!_header.parsed) processedBytes = processHeader(&buffer, bufferLength);
  if (!_header.parsed) return processedBytes;
  if ((_header.method == "GET" && _header.contentLength == 0) ||
      (_header.method == "DELETE" && _header.contentLength == 0) ||
      (_header.method == "OPTIONS" && _header.contentLength == 0) ||
      _header.method == "M-SEARCH" ||
      (_header.method == "NOTIFY" && _header.contentLength == 0) ||
      (_contentLengthSet && _header.contentLength == 0) ||
      (_header.responseCode >= 300 && _header.responseCode <= 399)) {
    _dataProcessingStarted = true;
    setFinished();
    return processedBytes;
  }
  if (!_dataProcessingStarted) {
    if (checkForChunkedXml || checkForChunkedJson) {
      if (bufferLength + _partialChunkSize.length() < 8) //Not enough data.
      {
        _partialChunkSize.append(buffer, bufferLength);
        return processedBytes + bufferLength;
      }
      std::string chunk = _partialChunkSize + std::string(buffer, bufferLength);
      std::string::size_type pos = 0;
      if (checkForChunkedXml) pos = chunk.find('<');
      else {
        pos = chunk.find('[');
        std::string::size_type pos2 = chunk.find('{');
        if (pos2 != std::string::npos && (pos == std::string::npos || pos != 0)) pos = pos2;
      }
      if (pos != std::string::npos && pos != 0) {
        if (BaseLib::Math::isNumber(BaseLib::HelperFunctions::trim(chunk), true)) _header.transferEncoding = BaseLib::Http::TransferEncoding::chunked;
      }
    }
    if (_header.contentLength > _maxContentSize) throw HttpException("Data is larger than " + std::to_string(_maxContentSize) + " bytes.");
    _content.reserve(_header.contentLength);
  }
  _dataProcessingStarted = true;

  if (_header.transferEncoding & TransferEncoding::Enum::chunked) {
    processedBytes += processChunkedContent(buffer, bufferLength);
  } else {
    processedBytes += processContent(buffer, bufferLength);
  }

  return processedBytes;
}

int32_t Http::processHeader(char **buffer, int32_t &bufferLength) {
  char *end = (char *)memmem(*buffer, bufferLength, "\r\n\r\n", 4);
  uint32_t headerSize = 0;
  int32_t crlfOffset = 2;
  if (!end || ((end + 3) - *buffer) + 1 > bufferLength) {
    end = (char *)memmem(*buffer, bufferLength, "\n\n", 2);
    if (!end || ((end + 1) - *buffer) + 1 > bufferLength) {
      if (_rawHeader.size() > 2 && (
          (_rawHeader.back() == '\n' && **buffer == '\n') ||
              (_rawHeader.back() == '\r' && **buffer == '\n' && *(*buffer + 1) == '\r') ||
              (_rawHeader.at(_rawHeader.size() - 2) == '\r' && _rawHeader.back() == '\n' && **buffer == '\r') ||
              (_rawHeader.at(_rawHeader.size() - 2) == '\n' && _rawHeader.back() == '\r' && **buffer == '\n')
      )) {
        //Special case: The two new lines are split between _rawHeader and buffer
        //Cases:
        //	For crlf:
        //		rawHeader = ...\n, buffer = \n...
        //	For lf:
        //		rawHeader = ...\r, buffer = \n\r\n...
        //		rawHeader = ...\r\n, buffer = \r\n...
        //		rawHeader = ...\r\n\r, buffer = \n...
        if (**buffer == '\n' && *(*buffer + 1) != '\r') //rawHeader = ...\r\n\r, buffer = \n... or rawHeader = ...\n, buffer = \n...
        {
          headerSize = 1;
          if (_rawHeader.back() == '\r') crlfOffset = 2;
          else crlfOffset = 1;
        } else if (**buffer == '\n' && *(*buffer + 1) == '\r' && *(*buffer + 2) == '\n') //rawHeader = ...\r, buffer = \n\r\n...
        {
          headerSize = 3;
          crlfOffset = 2;
        } else if (**buffer == '\r' && *(*buffer + 1) == '\n') //rawHeader = ...\r\n, buffer = \r\n...
        {
          headerSize = 2;
          crlfOffset = 2;
        }
      } else {
        if (_rawHeader.size() + bufferLength > _maxHeaderSize) throw HttpException("Header is larger than " + std::to_string(_maxHeaderSize) + " bytes.");
        _rawHeader.insert(_rawHeader.end(), *buffer, *buffer + bufferLength);
        return bufferLength;
      }
    } else {
      crlfOffset = 1;
      _crlf = false;
      headerSize = ((end + 1) - *buffer) + 1;
    }
  } else headerSize = ((end + 3) - *buffer) + 1;

  if (_rawHeader.size() + headerSize > _maxHeaderSize) throw HttpException("Header is larger than " + std::to_string(_maxHeaderSize) + " bytes.");
  _rawHeader.insert(_rawHeader.end(), *buffer, *buffer + headerSize);

  char *headerBuffer = _rawHeader.data();
  end = _rawHeader.data() + _rawHeader.size();
  *buffer += headerSize;
  bufferLength -= headerSize;

  if (_rawHeader.size() > 10 && !strncmp(headerBuffer, "HTTP/", 5)) {
    _type = Type::Enum::response;
    _header.responseCode = strtol(headerBuffer + 9, nullptr, 10);
  } else if (_rawHeader.size() > 10) {
    char *endPos = (char *)memchr(headerBuffer, ' ', 10);
    if (!endPos) throw HttpException("Your client sent a request that this server could not understand (1).");
    _type = Type::Enum::request;
    _header.method = std::string(headerBuffer, endPos);
  }

  char *newlinePos = nullptr;

  if (!_header.method.empty()) {
    int32_t startPos = _header.method.size() + 1;
    newlinePos = (crlfOffset == 2) ? (char *)memchr(headerBuffer, '\r', end - headerBuffer) : (char *)memchr(headerBuffer, '\n', end - headerBuffer);
    if (!newlinePos || newlinePos > end) throw HttpException("Could not parse HTTP header.");

    char *endPos = (char *)HelperFunctions::memrchr(headerBuffer + startPos, ' ', newlinePos - (headerBuffer + startPos));
    if (!endPos) throw HttpException("Your client sent a request that this server could not understand (2).");

    _header.path = std::string(headerBuffer + startPos, (int32_t)(endPos - headerBuffer - startPos));
    int32_t pos = _header.path.find('?');
    if (pos != (signed)std::string::npos) {
      if ((unsigned)pos + 1 < _header.path.size()) _header.args = _header.path.substr(pos + 1);
      _header.path = _header.path.substr(0, pos);
    }
    pos = _header.path.find(".php");
    if (pos == (signed)std::string::npos) pos = _header.path.find(".hgs");
    if (pos != (signed)std::string::npos) {
      pos = _header.path.find('/', pos);
      if (pos != (signed)std::string::npos) {
        _header.pathInfo = _header.path.substr(pos);
        _header.path = _header.path.substr(0, pos);
      }
    }
    _header.path = decodeURL(_header.path);
    HelperFunctions::stringReplace(_header.path, "../", "");

    if (!strncmp(endPos + 1, "HTTP/2.0", 8)) _header.protocol = Http::Protocol::http20;
    else if (!strncmp(endPos + 1, "HTTP/1.1", 8)) _header.protocol = Http::Protocol::http11;
    else if (!strncmp(endPos + 1, "HTTP/1.0", 8)) _header.protocol = Http::Protocol::http10;
    else throw HttpException("Your client is using a HTTP protocol version that this server cannot understand.");
  }

  char *colonPos = nullptr;
  newlinePos = (char *)memchr(headerBuffer, '\n', _rawHeader.size());
  if (!newlinePos || newlinePos > end) throw HttpException("Could not parse HTTP header.");
  headerBuffer = newlinePos + 1;

  while (headerBuffer < end) {
    newlinePos = (crlfOffset == 2) ? (char *)memchr(headerBuffer, '\r', end - headerBuffer) : (char *)memchr(headerBuffer, '\n', end - headerBuffer);
    if (!newlinePos || newlinePos > end) break;
    colonPos = (char *)memchr(headerBuffer, ':', newlinePos - headerBuffer);
    if (!colonPos || colonPos > newlinePos) {
      headerBuffer = newlinePos + crlfOffset;
      continue;
    }

    if (colonPos < newlinePos - 1) processHeaderField(headerBuffer, (uint32_t)(colonPos - headerBuffer), colonPos + 1, (uint32_t)(newlinePos - colonPos - 1));
    headerBuffer = newlinePos + crlfOffset;
  }
  _header.parsed = true;
  return headerSize;
}

void Http::processHeaderField(char *name, uint32_t nameSize, char *value, uint32_t valueSize) {
  if (nameSize == 0 || valueSize == 0 || !name || !value) return;
  while (*value == ' ' && valueSize > 0) {
    //Skip whitespace
    value++;
    valueSize--;
  }
  if (nameSize == 14 && !strnaicmp(name, "content-length", nameSize)) {
    //Ignore Content-Length when Transfer-Encoding is present. See: http://greenbytes.de/tech/webdav/rfc2616.html#rfc.section.4.4
    if (_header.transferEncoding == TransferEncoding::Enum::none) {
      _contentLengthSet = true;
      _header.contentLength = strtol(value, nullptr, 10);
    }
  } else if (nameSize == 4 && !strnaicmp(name, "host", nameSize)) {
    _header.host = std::string(value, valueSize);
    HelperFunctions::toLower(_header.host);
    HelperFunctions::stringReplace(_header.host, "../", "");
  } else if (nameSize == 12 && !strnaicmp(name, "content-type", nameSize)) {
    _header.contentType = std::string(value, valueSize);
    _header.contentTypeFull = _header.contentType;
    _header.contentType = HelperFunctions::splitFirst(_header.contentType, ';').first;
    HelperFunctions::toLower(_header.contentType);
  } else if (nameSize == 15 && !strnaicmp(name, "accept-encoding", nameSize)) {
    std::string s(value, valueSize);
    s = s.substr(0, s.find(';'));
    int32_t pos = 0;
    while ((pos = s.find(',')) != (signed)std::string::npos || !s.empty()) {
      std::string ae = (pos == (signed)std::string::npos) ? s : s.substr(0, pos);
      HelperFunctions::trim(BaseLib::HelperFunctions::toLower(ae));
      if (ae == "br") _header.acceptEncoding = (AcceptEncoding::Enum)(_header.acceptEncoding | AcceptEncoding::Enum::br);
      else if (ae == "gzip") _header.acceptEncoding = (AcceptEncoding::Enum)(_header.acceptEncoding | AcceptEncoding::Enum::gzip);
      else if (ae == "deflate") _header.acceptEncoding = (AcceptEncoding::Enum)(_header.acceptEncoding | AcceptEncoding::Enum::deflate);
      if (pos == (signed)std::string::npos) s.clear(); else s.erase(0, pos + 1);
    }
  } else if (nameSize == 16 && !strnaicmp(name, "content-encoding", nameSize)) {
    std::string s(value, valueSize);
    s = s.substr(0, s.find(';'));
    int32_t pos = 0;
    while ((pos = s.find(',')) != (signed)std::string::npos || !s.empty()) {
      std::string ce = (pos == (signed)std::string::npos) ? s : s.substr(0, pos);
      HelperFunctions::trim(BaseLib::HelperFunctions::toLower(ce));
      if (ce == "gzip") _header.contentEncoding = (ContentEncoding::Enum)(_header.contentEncoding | ContentEncoding::Enum::gzip);
      else throw HttpException("Unknown value for HTTP header \"Content-Encoding\": " + std::string(value, valueSize));
      if (pos == (signed)std::string::npos) s.clear(); else s.erase(0, pos + 1);
    }
  } else if ((nameSize == 17 && !strnaicmp(name, "transfer-encoding", nameSize)) || (nameSize == 2 && !strnaicmp(name, "te", nameSize))) {
    if (_header.contentLength > 0) _header.contentLength = 0; //Ignore Content-Length when Transfer-Encoding is present. See: http://greenbytes.de/tech/webdav/rfc2616.html#rfc.section.4.4
    std::string s(value, valueSize);
    s = s.substr(0, s.find(';'));
    int32_t pos = 0;
    while ((pos = s.find(',')) != (signed)std::string::npos || !s.empty()) {
      std::string te = (pos == (signed)std::string::npos) ? s : s.substr(0, pos);
      HelperFunctions::trim(BaseLib::HelperFunctions::toLower(te));
      if (te == "chunked") _header.transferEncoding = (TransferEncoding::Enum)(_header.transferEncoding | TransferEncoding::Enum::chunked);
      else if (te == "compress") _header.transferEncoding = (TransferEncoding::Enum)(_header.transferEncoding | TransferEncoding::Enum::compress | TransferEncoding::Enum::chunked);
      else if (te == "deflate") _header.transferEncoding = (TransferEncoding::Enum)(_header.transferEncoding | TransferEncoding::Enum::deflate | TransferEncoding::Enum::chunked);
      else if (te == "gzip") _header.transferEncoding = (TransferEncoding::Enum)(_header.transferEncoding | TransferEncoding::Enum::gzip | TransferEncoding::Enum::chunked);
      else if (te == "identity") _header.transferEncoding = (TransferEncoding::Enum)(_header.transferEncoding | TransferEncoding::Enum::identity);
      else if (te == "trailers") _header.transferEncoding = (TransferEncoding::Enum)(_header.transferEncoding | TransferEncoding::Enum::trailers);
      else throw HttpException("Unknown value for HTTP header \"Transfer-Encoding\": " + std::string(value, valueSize));
      if (pos == (signed)std::string::npos) s.clear(); else s.erase(0, pos + 1);
    }
  } else if (nameSize == 10 && !strnaicmp(name, "connection", nameSize)) {
    std::string s(value, valueSize);
    s = s.substr(0, s.find(';'));
    int32_t pos = 0;
    while ((pos = s.find(',')) != (signed)std::string::npos || !s.empty()) {
      std::string c = (pos == (signed)std::string::npos) ? s : s.substr(0, pos);
      HelperFunctions::trim(BaseLib::HelperFunctions::toLower(c));
      if (c == "keep-alive") _header.connection = (Connection::Enum)(_header.connection | Connection::Enum::keepAlive);
      else if (c == "close") _header.connection = (Connection::Enum)(_header.connection | Connection::Enum::close);
      else if (c == "upgrade") _header.connection = (Connection::Enum)(_header.connection | Connection::Enum::upgrade);
      else if (c == "te") {} //ignore
      else throw HttpException("Unknown value for HTTP header \"Connection\": " + std::string(value, valueSize));
      if (pos == (signed)std::string::npos) s.clear(); else s.erase(0, pos + 1);
    }
  } else if (nameSize == 6 && !strnaicmp(name, "cookie", nameSize)) {
    _header.cookie = std::string(value, valueSize);
    std::vector<std::string> cookies = HelperFunctions::splitAll(_header.cookie, ';');
    for (auto &cookie : cookies) {
      auto data = HelperFunctions::splitFirst(cookie, '=');
      _header.cookies.emplace(HelperFunctions::trim(data.first), HelperFunctions::trim(data.second));
    }
  } else if (nameSize == 13 && !strnaicmp(name, "authorization", nameSize)) _header.authorization = std::string(value, valueSize);
  std::string lowercaseName(name, nameSize);
  HelperFunctions::toLower(lowercaseName);
  _header.fields[lowercaseName] = std::string(value, valueSize);
}

int32_t Http::strnaicmp(char const *a, char const *b, uint32_t size) {
  if (size == 0) return 0;
  for (uint32_t pos = 0;; a++, b++, pos++) {
    int32_t d = tolower(*a) - *b;
    if (d != 0 || pos == size - 1) return d;
  }
  return 0;
}

void Http::reset() {
  _header = Header();
  _content.clear();
  _rawHeader.clear();
  _chunk.clear();
  _content.shrink_to_fit();
  _rawHeader.shrink_to_fit();
  _chunk.shrink_to_fit();
  _chunkNewLineMissing = false;
  _type = Type::Enum::none;
  _finished = false;
  _dataProcessingStarted = false;
  _headerProcessingStarted = false;
}

void Http::setFinished() {
  if (_finished) return;
  _finished = true;
  _content.push_back('\0');
}

int32_t Http::processContent(char *buffer, int32_t bufferLength) {
  if (_content.size() + bufferLength > _maxContentSize) throw HttpException("Data is larger than " + std::to_string(_maxContentSize) + " bytes.");
  int32_t processedBytes = bufferLength;
  if (_header.contentLength == 0) {
    _content.insert(_content.end(), buffer, buffer + bufferLength);
    if (_header.contentType == "application/json") {
      bool finished = true;
      try {
        Rpc::JsonDecoder::decode(_content);
      } catch(...) {
        finished = false;
      }
      if (finished) {
        setFinished();
      }
    }
  }
  else {
    if (_content.size() + bufferLength > _header.contentLength) processedBytes -= (_content.size() + bufferLength) - _header.contentLength;
    _content.insert(_content.end(), buffer, buffer + processedBytes);
    if (_content.size() == _header.contentLength) {
      setFinished();
    }
  }
  if (processedBytes < bufferLength) {
    buffer += processedBytes;
    while (processedBytes < bufferLength && (*buffer == '\r' || *buffer == '\n' || *buffer == '\0')) {
      buffer++;
      processedBytes++;
    }
  }
  return processedBytes;
}

int32_t Http::processChunkedContent(char *buffer, int32_t bufferLength) {
  int32_t initialBufferLength = bufferLength;
  while (true) {
    if (_content.size() + bufferLength > _maxContentSize) throw HttpException("Data is larger than " + std::to_string(_maxContentSize) + " bytes.");
    if (_chunkSize == -1) {
      if (_chunkNewLineMissing) {
        _chunkNewLineMissing = false;
        if (*buffer == '\r' && bufferLength > 0) {
          buffer++;
          bufferLength--;
        }
        if (*buffer == '\n' && bufferLength > 0) {
          buffer++;
          bufferLength--;
        }
      }
      readChunkSize(&buffer, bufferLength);
      if (_chunkSize == -1) break;
    } else {
      if (_chunkSize == 0) {
        setFinished();
        break;
      }
      if (bufferLength <= 0) break;
      int32_t sizeToInsert = bufferLength;
      if ((signed)_chunk.size() + sizeToInsert > _chunkSize) sizeToInsert -= (_chunk.size() + sizeToInsert) - _chunkSize;
      _chunk.insert(_chunk.end(), buffer, buffer + sizeToInsert);
      if ((signed)_chunk.size() == _chunkSize) {
        _content.insert(_content.end(), _chunk.begin(), _chunk.end());
        _chunk.clear();
        _chunkSize = -1;
      }
      bufferLength -= _crlf ? sizeToInsert + 2 : sizeToInsert + 1;
      if (bufferLength < 0) {
        _chunkNewLineMissing = true;
        break;
      }
      buffer = _crlf ? buffer + sizeToInsert + 2 : buffer + sizeToInsert + 1;
    }
  }
  if (bufferLength < 0) bufferLength = 0;
  while (bufferLength > 0 && (*buffer == '\r' || *buffer == '\n' || *buffer == '\0')) {
    buffer++;
    bufferLength--;
  }
  return initialBufferLength - bufferLength;
}

void Http::readChunkSize(char **buffer, int32_t &bufferLength) {
  char *newlinePos;
  if (_chunkSize == -1 && _endChunkSizeBytes == 0) {
    newlinePos = strchr(*buffer, '\n');
    if (_partialChunkSize.empty() && newlinePos == *buffer) newlinePos = strchr(*buffer + 1, '\n'); //\n is first character
    if (_partialChunkSize.empty() && newlinePos == *buffer + 1 && **buffer == '\r') newlinePos = strchr(*buffer + 2, '\n'); //\r is first character
    if (!newlinePos || newlinePos >= *buffer + bufferLength) throw BaseLib::Exception("Could not parse chunk size (1).");
    std::string chunkSize = _partialChunkSize + std::string(*buffer, newlinePos - *buffer);
    HelperFunctions::trim(_partialChunkSize);
    if (!Math::isNumber(chunkSize, true)) throw BaseLib::Exception("Chunk size is no number.");
    _chunkSize = Math::getNumber(chunkSize, true);
    _partialChunkSize = "";
    bufferLength -= (newlinePos + 1) - *buffer;
    *buffer = newlinePos + 1;
  }
  _endChunkSizeBytes = -1;
  if (_chunkSize > -1) return;

  newlinePos = strchr(*buffer, '\n');
  if (!newlinePos || newlinePos >= *buffer + bufferLength) {
    _endChunkSizeBytes = 0;
    char *semicolonPos;
    semicolonPos = strchr(*buffer, ';');
    if (!semicolonPos || semicolonPos >= *buffer + bufferLength) {
      _partialChunkSize = std::string(*buffer, bufferLength);
      if (_partialChunkSize.size() > 8) throw HttpException("Could not parse chunk size (2).");
    } else {
      _chunkSize = strtol(*buffer, NULL, 16);
      if (_chunkSize < 0) throw HttpException("Could not parse chunk size. Chunk size is negative.");
    }
  } else {
    _chunkSize = strtol(*buffer, NULL, 16);
    if (_chunkSize < 0) throw HttpException("Could not parse chunk size. Chunk size is negative.");
    bufferLength -= (newlinePos + 1) - *buffer;
    if (bufferLength == -1) {
      bufferLength = 0;
      _endChunkSizeBytes = 1;
    }
    *buffer = newlinePos + 1;
  }
}

std::string Http::encodeURL(const std::string &url) {
  std::ostringstream encoded;
  encoded.fill('0');
  encoded << std::hex;

  for (std::string::const_iterator i = url.begin(); i != url.end(); ++i) {
    if (isalnum(*i) || *i == '-' || *i == '_' || *i == '.' || *i == '~') {
      encoded << *i;
      continue;
    }

    encoded << '%' << std::setw(2) << int((unsigned char)(*i));
  }

  return encoded.str();
}

std::string Http::decodeURL(const std::string &url) {
  std::ostringstream decoded;
  char character;
  for (std::string::const_iterator i = url.begin(); i != url.end(); ++i) {
    if (*i == '%') {
      i++;
      if (i == url.end()) return decoded.str();
      character = (char)(Math::getNumber(*i) << 4);
      i++;
      if (i == url.end()) return decoded.str();
      character += (char)Math::getNumber(*i);
      decoded << character;
    } else if (*i == '+') {
      decoded << ' ';
    } else decoded << *i;
  }
  return decoded.str();
}

size_t Http::readStream(char *buffer, size_t requestLength) {
  size_t bytesRead = 0;
  if (_streamPos < _rawHeader.size()) {
    size_t length = requestLength;
    if (_streamPos + length > _rawHeader.size()) length = _rawHeader.size() - _streamPos;
    memcpy(buffer, &_rawHeader.at(_streamPos), length);
    _streamPos += length;
    bytesRead += length;
    requestLength -= length;
  }
  if (_content.size() == 0) return bytesRead;
  size_t contentSize = _content.size() - 1; //Ignore trailing "0"
  if (requestLength > 0 && (_streamPos - _rawHeader.size()) < contentSize) {
    size_t length = requestLength;
    if ((_streamPos - _rawHeader.size()) + length > contentSize) length = _content.size() - (_streamPos - _rawHeader.size());
    memcpy(buffer + bytesRead, &_content.at(_streamPos - _rawHeader.size()), length);
    _streamPos += length;
    bytesRead += length;
    requestLength -= length;
  }
  return bytesRead;
}

size_t Http::readContentStream(char *buffer, size_t requestLength) {
  size_t bytesRead = 0;
  size_t contentSize = _content.size() - 1; //Ignore trailing "0"
  if (_contentStreamPos < contentSize) {
    size_t length = requestLength;
    if (_contentStreamPos + length > contentSize) length = contentSize - _contentStreamPos;
    memcpy(buffer, &_content.at(_contentStreamPos), length);
    _contentStreamPos += length;
    bytesRead += length;
    requestLength -= length;
  }
  return bytesRead;
}

size_t Http::readFirstContentLine(char *buffer, size_t requestLength) {
  if (_content.size() == 0) return 0;
  if (_contentStreamPos >= _content.size() - 1) return 0;
  size_t bytesRead = 0;
  char *posTemp = (char *)memchr(&_content.at(_contentStreamPos), '\n', _content.size() - 1 - _contentStreamPos);
  int32_t newlinePos = 0;
  if (posTemp != nullptr) newlinePos = posTemp - &_content.at(0);
  if (newlinePos > 0 && _content.at(newlinePos - 1) == '\r') newlinePos--;
  else if (newlinePos <= 0) newlinePos = _content.size() - 1;
  if (_contentStreamPos < (unsigned)newlinePos) {
    size_t length = requestLength;
    if (_contentStreamPos + length > (unsigned)newlinePos) length = newlinePos - _contentStreamPos;
    memcpy(buffer, &_content.at(_contentStreamPos), length);
    _contentStreamPos += length;
    bytesRead += length;
    requestLength -= length;
  }
  return bytesRead;
}

std::unordered_map<std::string, std::string> Http::getParsedQueryString() {
  std::unordered_map<std::string, std::string> parsedQueryString;

  auto parts = BaseLib::HelperFunctions::splitAll(_header.args, '&');
  for (auto &part : parts) {
    auto pair = BaseLib::HelperFunctions::splitFirst(part, '=');
    parsedQueryString.emplace(decodeURL(pair.first), decodeURL(pair.second));
  }
  return parsedQueryString;
}

std::string Http::stripHeader(const std::string &header, const std::unordered_set<std::string> &fieldsToStrip, const std::string &fieldsToAdd) {
  if (header.empty()) return "";
  std::string newHeader;
  newHeader.reserve(header.size());
  auto lines = HelperFunctions::splitAll(header, '\n');
  bool firstLine = true;
  for (auto &line : lines) {
    if (firstLine) {
      firstLine = false;
      newHeader.append(line + "\n");
      continue;
    }
    auto pair = HelperFunctions::splitFirst(line, ':');
    if (pair.first.empty() || pair.second.empty()) continue;
    HelperFunctions::toLower(pair.first);
    if (fieldsToStrip.find(pair.first) != fieldsToStrip.end()) continue;
    newHeader.append(line + "\n");
  }
  newHeader.append(fieldsToAdd);
  newHeader.append("\r\n");
  return newHeader;
}

}
