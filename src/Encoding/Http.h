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

#ifndef HTTP_H_
#define HTTP_H_

#include "../Variable.h"
#include "../Exception.h"
#include "../HelperFunctions/Math.h"

#include <string>
#include <map>
#include <cstring>
#include <memory>
#include <vector>
#include <set>
#include <unordered_set>

namespace BaseLib {
class HttpException : public BaseLib::Exception {
 private:
  int32_t _responseCode = -1;
 public:
  HttpException(std::string message) : Exception(message) {}
  HttpException(std::string message, int32_t responseCode) : Exception(message), _responseCode(responseCode) {}

  int32_t responseCode() const { return _responseCode; }
};

class Http {
 public:
  struct Type {
    enum Enum { none, request, response };
  };
  struct AcceptEncoding {
    enum Enum { none = 0, deflate = 4, gzip = 8, br = 32 };
  };
  struct ContentEncoding {
    enum Enum { none = 0, gzip = 8 };
  };
  struct TransferEncoding {
    enum Enum { none = 0, chunked = 1, compress = 2, deflate = 4, gzip = 8, identity = 16 };
  };
  struct Connection {
    enum Enum { none = 0, keepAlive = 1, close = 2, upgrade = 4 };
  };
  struct Protocol {
    enum Enum { none, http10, http11, http20 };
  };
  struct Header {
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
    AcceptEncoding::Enum acceptEncoding = AcceptEncoding::Enum::none;
    ContentEncoding::Enum contentEncoding = ContentEncoding::Enum::none;
    TransferEncoding::Enum transferEncoding = TransferEncoding::Enum::none;
    Connection::Enum connection = Connection::Enum::none;
    std::string authorization;
    std::string cookie;
    std::unordered_map<std::string, std::string> cookies;
    std::string remoteAddress;
    int32_t remotePort = 0;
    std::map<std::string, std::string> fields;
  };

  struct FormData {
    std::string contentDisposition;
    std::string name;
    std::string filename;
    std::string contentType;
    std::string contentTypeFull;
    std::unordered_map<std::string, std::string> header;
    std::shared_ptr<std::vector<char>> data;
    std::set<std::shared_ptr<FormData>> multipartMixed;
  };

  Http();
  virtual ~Http();

  Type::Enum getType() { return _type; }
  bool headerIsFinished() { return _header.parsed; }
  bool isFinished() { return _finished; }
  std::string getRedirectUrl() { return _redirectUrl; }
  void setRedirectUrl(std::string value) { _redirectUrl = value; }
  std::string getRedirectQueryString() { return _redirectQueryString; }
  void setRedirectQueryString(std::string value) { _redirectQueryString = value; }
  int32_t getRedirectStatus() { return _redirectStatus; }
  void setRedirectStatus(int32_t value) { _redirectStatus = value; }
  size_t getMaxHeaderSize() { return _maxHeaderSize; }
  void setMaxHeaderSize(size_t value) { _maxHeaderSize = value; }
  size_t getMaxContentSize() { return _maxContentSize; }
  void setMaxContentSize(size_t value) { _maxContentSize = value; }

  /**
   * This method sets _finished and terminates _content with a null character. Use it, when the header does not contain "Content-Length".
   *
   * @see isFinished()
   * @see _finished
   */
  void setFinished();
  const std::vector<char> &getRawHeader() const { return _rawHeader; }
  const std::vector<char> &getContent() const { return _content; }
  uint32_t getContentSize() const { return _content.empty() ? 0 : (_finished ? _content.size() - 1 : _content.size()); }
  Header &getHeader() { return _header; }
  std::unordered_map<std::string, std::string> getParsedQueryString();
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
  int32_t process(char *buffer, int32_t bufferLength, bool checkForChunkedXml = false, bool checkForChunkedJson = false);
  bool headerProcessingStarted() { return _headerProcessingStarted; }
  bool dataProcessingStarted() { return _dataProcessingStarted; }
  static std::string encodeURL(const std::string &url);
  static std::string decodeURL(const std::string &url);
  size_t readStream(char *buffer, size_t requestLength);
  size_t readContentStream(char *buffer, size_t requestLength);
  size_t readFirstContentLine(char *buffer, size_t requestLength);
  std::string getMimeType(std::string extension);
  std::string getStatusText(int32_t code);
  std::set<std::shared_ptr<FormData>> decodeMultipartFormdata();
  std::set<std::shared_ptr<FormData>> decodeMultipartMixed(std::string &boundary, char *buffer, size_t bufferSize, char **pos);
  static void constructHeader(uint32_t contentLength, std::string contentType, int32_t code, std::string codeDescription, const std::vector<std::string> &additionalHeaders, std::string &header);
  /**
   * Strips a HTTP header of fields.
   * @param header
   * @param fieldsToStrip The fields to remove. The entries need to be lower case.
   * @param fieldsToAdd Fields to append to the header after it has been stripped. Make sure the string ends with "\r\n".
   * @return Returns the new header.
   */
  static std::string stripHeader(const std::string &header, const std::unordered_set<std::string> &fieldsToStrip, const std::string &fieldsToAdd);
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
  bool _chunkNewLineMissing = false;
  bool _finished = false;
  int32_t _chunkSize = -1;
  int32_t _endChunkSizeBytes = -1;
  std::string _partialChunkSize;
  size_t _streamPos = 0;
  size_t _contentStreamPos = 0;
  static const std::map<std::string, std::string> _extMimeTypeMap;
  static const std::map<int32_t, std::string> _statusCodeMap;
  std::string _redirectUrl;
  std::string _redirectQueryString;
  int32_t _redirectStatus = -1;
  size_t _maxHeaderSize = 102400;
  size_t _maxContentSize = 104857600;

  int32_t processHeader(char **buffer, int32_t &bufferLength);
  void processHeaderField(char *name, uint32_t nameSize, char *value, uint32_t valueSize);
  int32_t processContent(char *buffer, int32_t bufferLength);
  int32_t processChunkedContent(char *buffer, int32_t bufferLength);
  void readChunkSize(char **buffer, int32_t &bufferLength);

  char *findNextString(std::string &needle, char *buffer, size_t bufferSize);

  int32_t strnaicmp(char const *a, char const *b, uint32_t size);
};
}
#endif
