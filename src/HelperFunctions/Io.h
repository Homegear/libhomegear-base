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

#ifndef BASELIBIO_H_
#define BASELIBIO_H_

#include <string>
#include <vector>

namespace BaseLib {
namespace Security {
template<typename T>
class SecureVector;
}

class SharedObjects;

/**
 * This class provides functions to make your life easier.
 */
class Io {
 public:
  /**
   * Constructor.
   * It does nothing. You need to call init() to initialize the object.
   */
  Io();

  /**
   * Destructor.
   * Does nothing.
   */
  virtual ~Io();

  /**
   * Initialized the object.
   *
   * @param baseLib Pointer to the common base library object.
   */
  void init(SharedObjects *baseLib);

  /**
   * Checks if a directory exists.
   *
   * @param path The path to the directory.
   * @return Returns true when the directory exists, or false if the directory couldn't be accessed.
   */
  static bool directoryExists(const std::string &path);

  /**
   * Checks if a path is a directory.
   *
   * @param path The path to check.
   * @param[out] result True when the path is a directory otherwise false
   * @return Returns 0 on success or -1 on error.
   */
  static int32_t isDirectory(const std::string &path, bool &result);

  /**
   * Checks if a link exists.
   *
   * @param path The path to check.
   * @return Returns true when path is a link.
   */
  static bool linkExists(const std::string &path);

  /**
   * Creates a new directory. Make sure, the directory doesn't exist.
   *
   * @param path The directory to create.
   * @param mode The creation mode.
   * @return Returns 0 on success or -1 on error.
   */
  static bool createDirectory(const std::string &path, uint32_t mode);

  /**
   * Reads a file and returns the content as a string.
   *
   * @param filename The path to the file to read.
   * @return Returns the content of the file as a string.
   * @throws Throws Exception on errors.
   */
  static std::string getFileContent(const std::string &filename);

  /**
   * Reads a file and returns the content as a signed binary array.
   *
   * @param filename The path to the file to read.
   * @param maxBytes Maximum number of bytes to read.
   * @return Returns the content of the file as a char array.
   */
  static std::vector<char> getBinaryFileContent(const std::string &filename, uint32_t maxBytes = 0);

  /**
   * Reads a file and returns the content as an unsigned binary array.
   *
   * @param filename The path to the file to read.
   * @return Returns the content of the file as an unsigned char array.
   */
  static std::vector<uint8_t> getUBinaryFileContent(const std::string &filename);

  /**
   * Reads a file and returns the content as an unsigned binary secure array, which is cleaned up automatically on
   * destruction.
   *
   * @param filename The path to the file to read.
   * @return Returns the content of the file as an unsigned binary secure array, which is cleaned up automatically on
   * destruction.
   */
  static Security::SecureVector<uint8_t> getUBinaryFileContentSecure(const std::string &filename);

  /**
   * Checks if a file exists.
   *
   * @param filename The path to the file.
   * @return Returns true when the file exists, or false if the file couldn't be accessed.
   */
  static bool fileExists(const std::string &filename);

  /**
   * Writes a string to a file. If the file already exists it will be overwritten.
   *
   * @param filename The path to the file to write.
   * @param content The content to write to the file.
   */
  static void writeFile(const std::string &filename, const std::string &content);

  /**
   * Writes binary data to a file. If the file already exists it will be overwritten.
   *
   * @param filename The path to the file to write.
   * @param content The content to write to the file.
   * @param length The number of bytes to write.
   */
  static void writeFile(const std::string &filename, const std::vector<char> &content, uint32_t length);

  /**
   * Writes binary data to a file. If the file already exists it will be overwritten.
   *
   * @param filename The path to the file to write.
   * @param content The content to write to the file.
   * @param length The number of bytes to write.
   */
  static void writeFile(const std::string &filename, const std::vector<uint8_t> &content, uint32_t length);

  /**
   * Writes binary data to a file. If the file doesn't exist, it will be created. If the file already exists the data will be appended.
   *
   * @param filename The path to the file to write.
   * @param content The content to write to the file.
   */
  static void appendToFile(const std::string &filename, const std::string &content);

  /**
   * Writes binary data to a file. If the file doesn't exist, it will be created. If the file already exists the data will be appended.
   *
   * @param filename The path to the file to write.
   * @param content The content to write to the file.
   * @param length The number of bytes to write.
   */
  static void appendToFile(const std::string &filename, const std::vector<char> &content, uint32_t length);

  /**
   * Writes binary data to a file. If the file doesn't exist, it will be created. If the file already exists the data will be appended.
   *
   * @param filename The path to the file to write.
   * @param content The content to write to the file.
   * @param length The number of bytes to write.
   */
  static void appendToFile(const std::string &filename, const std::vector<uint8_t> &content, uint32_t length);

  /**
   * Returns an array of all files within a path. The files are not prefixed with "path".
   *
   * @param path The path to get all files for.
   * @param recursive Also return files of subdirectories. The files are prefixed with the subdirectory.
   * @return Returns an array of all file names within path.
   */
  static std::vector<std::string> getFiles(const std::string &path, bool recursive = false);

  /**
   * Returns an array of all directories within a path. The directories are not prefixed with "path".
   *
   * @param path The path to get all directories for.
   * @param recursive Also return directories within subdirectories. The directories are prefixed with the subdirectory.
   * @return Returns an array of all directory names within path.
   */
  static std::vector<std::string> getDirectories(const std::string &path, bool recursive = false);

  /**
   * Gets the last modified time of a file.
   *
   * @param filename The file to get the last modified time for.
   * @return The unix time stamp of the last modified time.
   */
  static int32_t getFileLastModifiedTime(const std::string &filename);

  /**
   * Copys a file.
   *
   * @param source The path to the file.
   * @param dest The destination path to copy the file to.
   * @return Returns true on success.
   */
  bool copyFile(const std::string &source, const std::string &dest);

  /**
   * Moves a file.
   *
   * @param source The path to the file.
   * @param dest The destination path to move the file to.
   * @return Returns true on success.
   */
  static bool moveFile(const std::string &source, const std::string &dest);

  /**
   * Deletes a file.
   *
   * @param file The file to delete.
   * @return Returns true on success.
   */
  static bool deleteFile(const std::string &file) noexcept;

  /**
   * Write locks a file using fcntl as defined in the Single Unix Specification. Note that the file stays locked until
   * the file descriptor is closed the first time.
   *
   * @param fileDescriptor The filedescriptor of the file to lock.
   * @param wait Wait for the lock to be acquired.
   * @return Returns true if the file was locked successful and false if the file is locked already.
   */
  static bool writeLockFile(int fileDescriptor, bool wait);

  /**
   * Read locks a file using fcntl as defined in the Single Unix Specification. Note that the file stays locked until
   * the file descriptor is closed the first time.
   *
   * @param fileDescriptor The filedescriptor of the file to lock.
   * @param wait Wait for the lock to be acquired.
   * @return Returns true if the file was locked successful and false if the file is locked already.
   */
  static bool readLockFile(int fileDescriptor, bool wait);

  /**
   * Calculates the SHA-2 SHA-512 of a given file.
   *
   * @param file The file to calculate the SHA-512 for.
   * @return Returns the SHA-512 or an empty string on errors.
   */
  std::string sha512(const std::string &file);
 private:
  /**
   * Pointer to the common base library object.
   */
  BaseLib::SharedObjects *_bl = nullptr;
};
}
#endif
