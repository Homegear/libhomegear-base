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

#ifndef HELPERFUNCTIONS_H_
#define HELPERFUNCTIONS_H_

#include <algorithm>
#include <ctime>
#include <map>
#include <fstream>
#include <sstream>
#include <mutex>
#include <random>
#include <vector>
#include <regex>

#include <dirent.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>

#include <gcrypt.h>
#include <unordered_set>

namespace BaseLib
{
class SharedObjects;

/**
 * This class provides functions to make your life easier.
 */
class HelperFunctions
{
public:
	/**
	 * Constructor.
	 * It does nothing. You need to call init() to initialize the object.
	 */
	HelperFunctions();

	/**
	 * Destructor.
	 * Does nothing.
	 */
	virtual ~HelperFunctions();

	/**
	 * Initializes the object.
	 *
	 * @param baseLib Pointer to the common base library object.
	 */
	void init(SharedObjects* baseLib);

	/**
	 * Gets the current unix time stamp in milliseconds.
	 *
	 * @see getTimeSeconds()
	 * @see getTimeMicroseconds()
	 * @return The current unix time stamp in milliseconds.
	 */
	static int64_t getTime();

	/**
	 * Gets the current unix time stamp in microseconds.
	 *
	 * @see getTimeSeconds()
	 * @see getTime()
	 * @return The current unix time stamp in microseconds.
	 */
	static int64_t getTimeMicroseconds();

	/**
	 * Gets the current unix time stamp in seconds.
	 *
	 * @see getTime()
	 * @see getTimeMicroseconds()
	 * @return The current unix time stamp in seconds.
	 */
	static int64_t getTimeSeconds();

	/**
	 * Gets the last modified time of a file.
	 *
	 * @param filename The file to get the last modified time for.
	 * @return The unix time stamp of the last modified time.
	 */
	static int32_t getFileLastModifiedTime(const std::string& filename);

	/**
	 * Gets the current time as a string like "08/27/14 14:13:53.471".
	 *
	 * @param time The unix time stamp in milliseconds to get the time string for. If "0" the current time is returned.
	 * @return Returns a time string like "08/27/14 14:13:53.471".
	 */
	static std::string getTimeString(int64_t time = 0);

	/**
	 * Gets the current time as a string with custom formatting.
	 *
	 * @param format The format like described for strftime
	 * @param time The unix time stamp in milliseconds to get the time string for. If "0" the current time is returned.
	 * @return Returns a time string like "08/27/14 14:13:53.471".
	 */
	static std::string getTimeString(std::string format, int64_t time = 0);

	/**
	 * Calculates a UUID including a time stamp in microseconds.
	 *
	 * @param time The time in microseconds to calculate the UUID for or "0" to use the current time.
	 * @return Returns a time UUID.
	 */
	static std::string getTimeUuid(int64_t time = 0);

	/**
	 * Left trims a string.
	 *
	 * @see rtrim()
	 * @see trim()
	 * @param[in,out] s The string to left trim.
	 * @return Returns a reference to the left trimmed string.
	 */
	static inline std::string &ltrim(std::string &s)
	{
			s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
			return s;
	}

	/**
	 * Right trims a string.
	 *
	 * @see ltrim()
	 * @see trim()
	 * @param[in,out] s The string to right trim.
	 * @return Returns a reference to the right trimmed string.
	 */
	static inline std::string &rtrim(std::string &s)
	{
			s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
			return s;
	}

	/**
	 * Trims a string.
	 *
	 * @see ltrim()
	 * @see rtrim()
	 * @param[in,out] s The string to trim.
	 * @return Returns a reference to the trimmed string.
	 */
	static inline std::string& trim(std::string& s)
	{
			return ltrim(rtrim(s));
	}

	/**
	 * Converts all characters of a string to lower case.
	 *
	 * @see toUpper()
	 * @param[in,out] s The string to convert all characters to lower case for.
	 * @return Returns a reference to the lower case string.
	 */
	static inline std::string& toLower (std::string& s)
	{
		std::transform(s.begin(), s.end(), s.begin(), ::tolower);
		return s;
	}

	/**
	 * Converts all characters of a string to upper case.
	 *
	 * @see toLower()
	 * @param[in,out] s The string to convert all characters to upper case for.
	 * @return Returns a reference to the upper case string.
	 */
	static inline std::string& toUpper (std::string& s)
	{
		std::transform(s.begin(), s.end(), s.begin(), ::toupper);
		return s;
	}

	/**
	 * Returns the size of a UTF-8 string. This method is needed, because std::string::size() and std::string::length() return the size in bytes.
	 *
	 * @param s The string to get the size for.
	 * @return Returns the size of the UTF-8 encoded string.
	 */
	static inline size_t utf8StringSize(const std::string& s)
	{
		if(s.empty()) return 0;
		const char* pS = s.c_str();
		size_t len = 0;
		while (*pS) len += (*pS++ & 0xc0) != 0x80;
		return len;
	}

	/**
	 * Returns an UTF-8 substring. This method is needed, because std::string::substr() counts bytes and not characters.
	 *
	 * @param s The string to get the substring from.
	 * @param start The start position of the substring.
	 * @param length The substring size.
	 * @return Returns the substring of the UTF-8 encoded string.
	 */
	static std::string utf8Substring(const std::string& s, uint32_t start, uint32_t length)
	{
		//From http://www.zedwood.com/article/cpp-utf-8-mb_substr-function
		if(length == 0) return "";
		uint32_t c, i, ix, q, min = (unsigned)std::string::npos, max = (unsigned)std::string::npos;
		for (q = 0, i = 0, ix = s.length(); i < ix; i++, q++)
		{
			if(q == start) min = i;
			if(q <= start + length || length == std::string::npos) max = i;

			c = (unsigned char)s[i];
			if      (c >= 0   && c <= 127) i += 0;
			else if ((c & 0xE0) == 0xC0) i += 1;
			else if ((c & 0xF0) == 0xE0) i += 2;
			else if ((c & 0xF8) == 0xF0) i += 3;
			else return ""; //invalid utf8
		}
		if (q <= start + length || length == (unsigned)std::string::npos) max = i;
		if (min == (unsigned)std::string::npos || max == (unsigned)std::string::npos) return "";
		return s.substr(min, max);
	}

	/**
	 * Replaces substrings within a string.
	 *
	 * @param[in,out] haystack The string to search the substring in.
	 * @param search The substring to search.
	 * @param replace The substring to replace "search" with.
	 * @return Returns a reference to the modified string.
	 */
	static std::string& stringReplace(std::string& haystack, std::string search, std::string replace)
	{
		if(search.empty()) return haystack;
		int32_t pos = 0;
		while(true)
		{
			 pos = haystack.find(search, pos);
			 if (pos == (signed)std::string::npos) break;
			 haystack.replace(pos, search.size(), replace);
			 pos += replace.size();
		}
		return haystack;
	}

	/**
	 * Replaces substrings within a string using regex.
	 *
	 * @param[in,out] haystack The string to search the substring in.
	 * @param regex The regex to search.
	 * @param replace The substring to replace "search" with.
	 * @param ignoreCase Set to true, to ignore the case.
	 * @return Returns a reference to the modified string.
	 */
	static std::string& regexReplace(std::string& haystack, std::string regex, std::string replace, bool ignoreCase);

	/**
	 * Splits a string at the first occurrence of a delimiter.
	 *
	 * @see splitAll()
	 * @see splitLast
	 * @param string The string to split.
	 * @param delimiter The delimiter to split the string at.
	 * @return Returns a pair with the two parts.
	 */
	static std::pair<std::string, std::string> splitFirst(std::string string, char delimiter);

	/**
	 * Splits a string at the last occurrence of a delimiter.
	 *
	 * @see splitAll()
	 * @see splitFirst()
	 * @param string The string to split.
	 * @param delimiter The delimiter to split the string at.
	 * @return Returns a pair with the two parts.
	 */
	static std::pair<std::string, std::string> splitLast(std::string string, char delimiter);

	/**
	 * Splits a string at all occurrences of a delimiter.
	 *
	 * @see split()
	 * @param string The string to split.
	 * @param delimiter The delimiter to split the string at.
	 * @return Returns an array with all split parts.
	 */
	static std::vector<std::string> splitAll(std::string string, char delimiter);

	/**
	 * Checks if a character is not alphanumeric ('_' and '-' are also regarded alphanumeric).
	 *
	 * @see isAlphaNumeric()
	 * @see stripNonAlphaNumeric
	 * @param c The character to check.
	 * @return Returns false if the character is alphanumeric, '_' or '-', otherwise true.
	 */
	static inline bool isNotAlphaNumeric(char c)
	{
		return !(isalpha(c) || isdigit(c) || (c == '_') || (c == '-'));
	}

	/**
	 * Checks if a string is alphanumeric
	 *
	 * @see isNotAlphaNumeric()
	 * @see stripNonAlphaNumeric
	 * @param s The string to check.
	 * @param additionalCharacters Additional characters to accept.
	 * @return Returns true if the string is alphanumeric, or contains '_' or '-', otherwise false.
	 */
	static bool isAlphaNumeric(std::string& s, std::unordered_set<char> additionalCharacters = std::unordered_set<char>())
	{
		return find_if
		(
			s.begin(),
			s.end(),
			[&](const char c){ return !(isalpha(c) || isdigit(c) || additionalCharacters.find(c) != additionalCharacters.end()); }
		) == s.end();
	}

	/**
	 * Strips all non alphanumeric characters from a string ('_' and '-' are also regarded alphanumeric).
	 *
	 * @see isNotAlphaNumeric()
	 * @see isAlphaNumeric()
	 * @param s The string to strip.
	 * @return Returns the stripped string.
	 */
	static std::string stripNonAlphaNumeric(const std::string& s);

	/**
	 * Strips all non printable characters from a string.
	 *
	 * @param s The string to strip.
	 * @return Returns the stripped string.
	 */
	static std::string stripNonPrintable(const std::string& s);

	/**
	 * Returns the endianess of the system.
	 *
	 * @return Returns true if the system is big endian, otherwise false.
	 */
	bool getBigEndian() { return _isBigEndian; }

	/**
	 * Generates a random integer.
	 *
	 * @param min The minimal value of the random integer.
	 * @param max The maximum value of the random integer.
	 * @return Returns a random integer between (and including) min and max.
	 */
	static int32_t getRandomNumber(int32_t min, int32_t max);

	/**
	 * Generates a random byte vector.
	 *
	 * @param size The number of bytes to generate.
	 * @return Returns a random byte vector of size "size".
	 */
	static std::vector<uint8_t> getRandomBytes(uint32_t size);

	/**
	 * Copies binary values from one memory location to another reversing the byte order when the system is little endian.
	 *
	 * @param[out] to The destination array. No memory is allocated, so make sure, the array is large enough.
	 * @param[in] from The source array.
	 * @param length The number of bytes to copy.
	 */
	void memcpyBigEndian(char* to, const char* from, const uint32_t& length);

	/**
	 * Copies binary values from one memory location to another reversing the byte order when the system is little endian.
	 *
	 * @param[out] to The destination array. No memory is allocated, so make sure, the array is large enough.
	 * @param[in] from The source array.
	 * @param length The number of bytes to copy.
	 */
	void memcpyBigEndian(uint8_t* to, const uint8_t* from, const uint32_t& length);

	/**
	 * Copies binary values from a vector to an integer reversing the byte order when the system is little endian.
	 *
	 * @param[out] to The destination integer.
	 * @param[in] from The source array. A length less than 4 bytes is allowed.
	 */
	void memcpyBigEndian(int32_t& to, const std::vector<uint8_t>& from);

	/**
	 * Copies binary values from an integer to a vector reversing the byte order when the system is little endian.
	 *
	 * @param[out] to The destination array.
	 * @param[in] from The source integer.
	 */
	void memcpyBigEndian(std::vector<uint8_t>& to, const int32_t& from);

	/**
	 * Copies binary values from a vector to an integer reversing the byte order when the system is little endian.
	 *
	 * @param[out] to The destination integer.
	 * @param[in] from The source array. A length less than 4 bytes is allowed.
	 */
	void memcpyBigEndian(int64_t& to, const std::vector<uint8_t>& from);

	/**
	 * Copies binary values from an integer to a vector reversing the byte order when the system is little endian.
	 *
	 * @param[out] to The destination array.
	 * @param[in] from The source integer.
	 */
	void memcpyBigEndian(std::vector<uint8_t>& to, const int64_t& from);

	/**
	 * Converts a hex string to a byte array.
	 *
	 * @param data The hex string to convert.
	 * @return Returns the byte array encoded by the hex string.
	 */
	static std::vector<uint8_t> hexToBin(const std::string& data);

	/**
	 * Converts a byte array to a hex string.
	 *
	 * @param buffer The byte array to convert.
	 * @param size The size of the buffer.
	 * @return Returns the hex string of the byte array.
	 */
	static std::string getHexString(const uint8_t* buffer, uint32_t size);

	/**
	 * Converts a byte array to a hex string.
	 *
	 * @param buffer The byte array to convert.
	 * @param size The size of the buffer.
	 * @return Returns the hex string of the byte array.
	 */
	static std::string getHexString(const char* buffer, uint32_t size);

	/**
	 * Converts a byte array to a hex string.
	 *
	 * @param data The byte array to convert.
	 * @return Returns the hex string of the byte array.
	 */
	static std::string getHexString(const std::vector<char>& data);

	/**
	 * Converts a byte array to a hex string.
	 *
	 * @param data The byte array to convert.
	 * @return Returns the hex string of the byte array.
	 */
	static std::string getHexString(const std::string& data);

	/**
	 * Converts a byte array to a hex string.
	 *
	 * @param data The byte array to convert.
	 * @return Returns the hex string of the byte array.
	 */
	static std::string getHexString(const std::vector<uint8_t>& data);

	/**
	 * Converts an int16 array to a hex string.
	 *
	 * @param data The array to convert.
	 * @return Returns the hex string of the array.
	 */
	static std::string getHexString(const std::vector<uint16_t>& data);

	/**
	 * Converts an integer to a hex string.
	 *
	 * @param number The integer to convert.
	 * @param width The minimal width of the hex string (default -1). If the hex string is smaller, it is prefixed with zeros.
	 * @return Returns the hex string of the integer.
	 */
	static std::string getHexString(int32_t number, int32_t width = -1);

    /**
     * Converts an integer to a hex string.
     *
     * @param number The integer to convert.
     * @param width The minimal width of the hex string (default -1). If the hex string is smaller, it is prefixed with zeros.
     * @return Returns the hex string of the integer.
     */
    static std::string getHexString(uint32_t number, int32_t width = -1);

	/**
	 * Converts an integer to a hex string.
	 *
	 * @param number The integer to convert.
	 * @param width The minimal width of the hex string (default -1). If the hex string is smaller, it is prefixed with zeros.
	 * @return Returns the hex string of the integer.
	 */
	static std::string getHexString(int64_t number, int32_t width = -1);

    /**
     * Converts an integer to a hex string.
     *
     * @param number The integer to convert.
     * @param width The minimal width of the hex string (default -1). If the hex string is smaller, it is prefixed with zeros.
     * @return Returns the hex string of the integer.
     */
    static std::string getHexString(uint64_t number, int32_t width = -1);

	/**
	 * Converts a nibble to a hex character.
	 *
	 * @param nibble The nibble to convert.
	 * @return Returns the hex character of the nibble.
	 */
	char getHexChar(int32_t nibble);

	/**
	 * Converts a hex string to a char vector.
	 *
	 * @param hexString The string to convert.
	 * @return Returns a vector of type char.
	 */
	std::vector<char> getBinary(const std::string& hexString);

	/**
	 * Converts a hex string to an unsigned char vector.
	 *
	 * @see getUBinary()
	 * @param hexString The string to convert.
	 * @return Returns a vector of type unsigned char.
	 */
	std::vector<uint8_t> getUBinary(const std::string& hexString);

	/**
	 * Converts a hex string to an unsigned char vector.
	 *
	 * @see getBinary()
	 * @param hexString The string to convert.
	 * @param size The maximum number of characters to convert.
	 * @param binary The unsigned char vector to append the converted bytes to. Already existing elements will not be cleared.
	 * @return Returns a reference to "binary".
	 */
	std::vector<uint8_t>& getUBinary(const std::string& hexString, uint32_t size, std::vector<uint8_t>& binary);

	/**
	 * Converts an unsigned char vector filled with ASCII characters to an unsigned char vector.
	 *
	 * @see getBinary()
	 * @param hexData The vector with ASCII characters to convert.
	 * @return Returns a vector of type unsigned char.
	 */
	std::vector<uint8_t> getUBinary(const std::vector<uint8_t>& hexData);

	/**
	 * Converts a hex string to a binary array of type string.
	 *
	 * @param hexString The hex string to convert.
	 * @return Returns a string containing the binary data.
	 */
	std::string getBinaryString(const std::string& hexString);

	/**
	 * Gets the UID of a user.
	 *
	 * @param username The name of the user to get the UID for.
	 * @return Returns the user ID or "-1" on error.
	 */
	uid_t userId(std::string username);

	/**
	 * Gets the GID of a group.
	 *
	 * @param groupname The name of the group to get the GID for.
	 * @return Returns the group ID or "-1" on error.
	 */
	gid_t groupId(std::string groupname);

	/**
	 * Starts a program and returns the process id.
	 *
	 * @param path The program to start.
	 * @param arguments The arguments to pass.
	 * @throws Exception
	 * @return Returns the PID on success and "-1" on error.
	 */
    static pid_t system(std::string path, std::vector<std::string> arguments);

	/**
	 * Starts a program with redirected pipe and returns the process id.
	 *
	 * @param path The program to start.
	 * @param arguments The arguments to pass.
	 * @param[out] stdIn Will be filled with the input file descriptor. Close it when the process finishes.
	 * @param[out] stdOut Will be filled with the output file descriptor. Close it when the process finishes.
	 * @param[out] stdErr Will be filled with the error file descriptor. Close it when the process finishes.
	 * @throws Exception
	 * @return Returns the PID on success and "-1" on error.
	 */
	static pid_t systemp(std::string path, std::vector<std::string> arguments, int& stdIn, int& stdOut, int& stdErr);

	/**
	 * Starts a program and returns the output.
	 *
	 * @param command The command to execute (passed to sh with "-c").
	 * @param[out] output The program output.
	 * @return Returns the programs exit code.
	 */
	static int32_t exec(std::string command, std::string& output);

	/**
	 * Converts GNUTLS certificate verification error codes to human readable error messages.
	 *
	 * @param errorCode The GNUTLS certificate verification error code.
	 * @return Returns the error message for the provided error code.
	 */
	static std::string getGNUTLSCertVerificationError(uint32_t errorCode);

	/**
	 * Checks if a command is a short CLI command.
	 *
	 * @param command The CLI command.
	 * @return Returns "true" when the command is a short command, otherwise "false"
	 */
	static bool isShortCliCommand(const std::string& command);

	/**
	 * Checks if a string contains a CLI command and checks and returns the arguments.
	 *
	 * @param command The CLI command.
	 * @param longCommand The long CLI command to search for at the beginning of command.
	 * @param shortCommand1 The short CLI command to search for at the beginning of command.
	 * @param shortCommand2 An optional second short CLI command to search for at the beginning of command. Set to empty string if not needed.
	 * @param minArgumentCount The minimum number of arguments.
	 * @param[out] arguments A string vector which is filled with the arguments.
	 * @param[out] showHelp Set to true when help is requested.
	 * @return Returns true on success and false if "command" doesn't start with "longCommand", "shortCommand1" or "shortCommand2".
	 */
	static bool checkCliCommand(const std::string& command, const std::string& longCommand, const std::string& shortCommand1, const std::string& shortCommand2, uint32_t minArgumentCount, std::vector<std::string>& arguments, bool& showHelp);

	/**
	 * Reverse memchr(). Searches the initial n bytes of buffer s for the last occurrence of 'c'.
	 */
	static void* memrchr(const void* s, int c, size_t n);
private:
	/**
	 * Pointer to the common base library object.
	 */
	BaseLib::SharedObjects* _bl = nullptr;

	/**
	 * The result of checkEndianness() is stored in this variable. This is done through calling "init".
	 */
	bool _isBigEndian = true;

    /**
     * Map to faster convert hexadecimal numbers.
     */
    static constexpr std::array<int32_t, 23> _asciiToBinaryTable{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0, 0, 10 ,11 ,12, 13, 14, 15};

    /**
	 * Map to faster convert hexadecimal numbers.
	 */
    static constexpr std::array<int32_t, 16> _binaryToASCIITable{0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46};

	/**
	 * Checks if the system is little or big endian.
	 */
	void checkEndianness();
};
}
#endif
