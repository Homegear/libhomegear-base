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

#include "Io.h"
#include "../BaseLib.h"
#include "../Security/SecureVector.h"

#include <sys/stat.h>

namespace BaseLib
{

Io::Io()
{
}

void Io::init(SharedObjects* baseLib)
{
	_bl = baseLib;
}

Io::~Io()
{

}

bool Io::fileExists(const std::string& filename)
{
	std::ifstream in(filename.c_str());
	return in.rdstate() != std::ios_base::failbit;
}

bool Io::directoryExists(const std::string& path)
{
	struct stat s{};
	if(stat(path.c_str(), &s) == 0 && (s.st_mode & S_IFDIR)) return true;
	return false;
}

int32_t Io::isDirectory(const std::string& path, bool& result)
{
	struct stat s{};
	result = false;
	if(stat(path.c_str(), &s) == 0)
	{
		if(s.st_mode & S_IFDIR) result = true;
		return 0;
	}
	return -1;
}

bool Io::createDirectory(const std::string& path, uint32_t /* Don't change to mode_t as that doesn't work on BSD systems */ mode)
{
	int32_t result = mkdir(path.c_str(), mode);
	if(result != 0) return result;
	return chmod(path.c_str(), mode) == 0; //Passing mode to mkdir doesn't set the permissions on the directory itself, so we call chmod.
}

int32_t Io::getFileLastModifiedTime(const std::string& filename)
{
	struct stat attributes{};
	if(stat(filename.c_str(), &attributes) == -1) return -1;
	return attributes.st_mtim.tv_sec;
}

std::string Io::getFileContent(const std::string& filename)
{
	std::ifstream in(filename.c_str(), std::ios::in | std::ios::binary);
	if(in)
	{
		std::string contents;
		in.seekg(0, std::ios::end);
		contents.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read((char*)contents.data(), contents.size());
		in.close();
		return contents;
	}
	throw Exception(strerror(errno));
}

std::vector<char> Io::getBinaryFileContent(const std::string& filename, uint32_t maxBytes)
{
	std::ifstream in(filename.c_str(), std::ios::in | std::ios::binary);
	if(in)
	{
		std::vector<char> contents;
		in.seekg(0, std::ios::end);
		uint32_t size = in.tellg();
		if(maxBytes > size || maxBytes == 0) maxBytes = size;
		contents.resize(maxBytes);
		in.seekg(0, std::ios::beg);
		in.read(contents.data(), contents.size());
		in.close();
		return(contents);
	}
	throw Exception(strerror(errno));
}

std::vector<uint8_t> Io::getUBinaryFileContent(const std::string& filename)
{
	std::ifstream in(filename.c_str(), std::ios::in | std::ios::binary);
	if(in)
	{
		std::vector<uint8_t> contents;
		in.seekg(0, std::ios::end);
		contents.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read((char*)contents.data(), contents.size());
		in.close();
		return(contents);
	}
	throw Exception(strerror(errno));
}

BaseLib::Security::SecureVector<uint8_t> Io::getUBinaryFileContentSecure(const std::string& filename)
{
    std::ifstream in(filename.c_str(), std::ios::in | std::ios::binary);
    if(in)
    {
        Security::SecureVector<uint8_t> contents;
        in.seekg(0, std::ios::end);
        contents.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read((char*)contents.data(), contents.size());
        in.close();
        return(contents);
    }
    throw Exception(strerror(errno));
}

void Io::writeFile(const std::string& filename, const std::string& content)
{
	std::ofstream file;
	file.open(filename);
	if(!file.is_open()) throw Exception("Could not open file.");
	file << content;
	file.close();
}

void Io::writeFile(const std::string& filename, const std::vector<char>& content, uint32_t length)
{
	std::ofstream file;
	file.open(filename);
	if(!file.is_open()) throw Exception("Could not open file.");
	file.write(content.data(), length);
	file.close();
}

void Io::writeFile(const std::string& filename, const std::vector<uint8_t>& content, uint32_t length)
{
	std::ofstream file;
	file.open(filename);
	if(!file.is_open()) throw Exception("Could not open file.");
	file.write((char*)content.data(), length);
	file.close();
}

void Io::appendToFile(const std::string& filename, const std::string& content)
{
	std::ofstream file;
	file.open(filename, std::ios_base::app);
	if(!file.is_open()) throw Exception("Could not open file.");
	file.write(content.c_str(), content.size());
	file.close();
}

void Io::appendToFile(const std::string& filename, const std::vector<char>& content, uint32_t length)
{
	std::ofstream file;
	file.open(filename, std::ios_base::app);
	if(!file.is_open()) throw Exception("Could not open file.");
	file.write(content.data(), length);
	file.close();
}

void Io::appendToFile(const std::string& filename, const std::vector<uint8_t>& content, uint32_t length)
{
	std::ofstream file;
	file.open(filename, std::ios_base::app);
	if(!file.is_open()) throw Exception("Could not open file.");
	file.write((char*)content.data(), length);
	file.close();
}

std::vector<std::string> Io::getFiles(const std::string& path, bool recursive)
{
	std::vector<std::string> files;
	DIR* directory;
	struct dirent* entry;
	struct stat statStruct{};

	std::string fixedPath = path;
	if(fixedPath.back() != '/') fixedPath += '/';
	if((directory = opendir(fixedPath.c_str())) != 0)
	{
		files.reserve(100);
		while((entry = readdir(directory)) != 0)
		{
			std::string name(entry->d_name);
			if(name == "." || name == "..") continue;
			if(stat((fixedPath + name).c_str(), &statStruct) == -1)
			{
				_bl->out.printWarning("Warning: Could not stat file \"" + fixedPath + name + "\": " + std::string(strerror(errno)));
				continue;
			}
			//Don't use dirent::d_type as it is not supported on all file systems. See http://nerdfortress.com/2008/09/19/linux-xfs-does-not-support-direntd_type/
			//Thanks to telkamp (https://github.com/Homegear/Homegear/issues/223)
			if(S_ISREG(statStruct.st_mode))
			{
				files.push_back(name);
				if(files.size() == files.capacity()) files.reserve(files.size() + 100);
			}
			else if(recursive && S_ISDIR(statStruct.st_mode))
			{
				std::vector<std::string> subdirFiles = getFiles(fixedPath + name, recursive);
				for(std::vector<std::string>::iterator i = subdirFiles.begin(); i != subdirFiles.end(); ++i)
				{
					files.push_back(name + '/' + *i);
					if(files.size() == files.capacity()) files.reserve(files.size() + 100);
				}
			}
		}
		closedir(directory);
	}
	else throw(Exception("Could not open directory \"" + fixedPath + "\""));
	return files;
}

std::vector<std::string> Io::getDirectories(const std::string& path, bool recursive)
{
	std::vector<std::string> directories;
	DIR* directory;
	struct dirent* entry;
	struct stat statStruct{};

	std::string fixedPath = path;
	if(fixedPath.back() != '/') fixedPath += '/';
	if((directory = opendir(fixedPath.c_str())) != nullptr)
	{
		directories.reserve(100);
		while((entry = readdir(directory)) != nullptr)
		{
			std::string name(entry->d_name);
			if(name == "." || name == "..") continue;
			if(stat((fixedPath + name).c_str(), &statStruct) == -1)
			{
				_bl->out.printWarning("Warning: Could not stat file \"" + fixedPath + name + "\": " + std::string(strerror(errno)));
				continue;
			}
			//Don't use dirent::d_type as it is not supported on all file systems. See http://nerdfortress.com/2008/09/19/linux-xfs-does-not-support-direntd_type/
			//Thanks to telkamp (https://github.com/Homegear/Homegear/issues/223)
			if(S_ISDIR(statStruct.st_mode))
			{
				directories.push_back(name + '/');
				if(directories.size() == directories.capacity()) directories.reserve(directories.size() + 100);
				if(recursive)
				{
					std::vector<std::string> subdirs = getDirectories(fixedPath + name, recursive);
					for(std::vector<std::string>::iterator i = subdirs.begin(); i != subdirs.end(); ++i)
					{
						directories.push_back(name + '/' + *i);
						if(directories.size() == directories.capacity()) directories.reserve(directories.size() + 100);
					}
				}
			}
		}
		closedir(directory);
	}
	else throw(Exception("Could not open directory \"" + fixedPath + "\""));
	return directories;
}

bool Io::copyFile(const std::string& source, const std::string& dest)
{
	try
	{
		int32_t in_fd = open(source.c_str(), O_RDONLY);
		if(in_fd == -1)
		{
			_bl->out.printError("Error copying file " + source + ": " + strerror(errno));
			return false;
		}

		unlink(dest.c_str());

		int32_t out_fd = open(dest.c_str(), O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP);
		if(out_fd == -1)
		{
			close(in_fd);
			_bl->out.printError("Error copying file " + source + ": " + strerror(errno));
			return false;
		}
		char buf[8192];

		while (true)
		{
			ssize_t result = read(in_fd, &buf[0], sizeof(buf));
			if (!result) break;
			if(result == -1)
			{
				close(in_fd);
				close(out_fd);
				_bl->out.printError("Error reading file " + source + ": " + strerror(errno));
				return false;
			}
			if(write(out_fd, &buf[0], result) != result)
			{
				close(in_fd);
				close(out_fd);
				_bl->out.printError("Error writing file " + dest + ": " + strerror(errno));
				return false;
			}
		}
		close(in_fd);
		close(out_fd);
		return true;
	}
	catch(const std::exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return false;
}

bool Io::moveFile(const std::string& source, const std::string& dest)
{
	if(rename(source.c_str(), dest.c_str()) == 0) return true;
    return false;
}

bool Io::deleteFile(const std::string& file)
{
	if(remove(file.c_str()) == 0) return true;
	return false;
}

std::string Io::sha512(const std::string& file)
{
	try
	{
		gcry_error_t result;
		gcry_md_hd_t stribogHandle = nullptr;
		if((result = gcry_md_open(&stribogHandle, GCRY_MD_SHA512, 0)) != GPG_ERR_NO_ERROR)
		{
			_bl->out.printError("Error: Could not initialize SHA512 handle: " + Security::Gcrypt::getError(result));
			return "";
		}

		std::string data = getFileContent(file);
		if(data.empty())
		{
			_bl->out.printError("Error: " + file + " is empty.");
			return "";
		}
		gcry_md_write(stribogHandle, &data.at(0), data.size());
		gcry_md_final(stribogHandle);
		uint8_t* digest = gcry_md_read(stribogHandle, GCRY_MD_SHA512);
		if(!digest)
		{
			_bl->out.printError("Error Could not generate SHA-512 of file: " + Security::Gcrypt::getError(result));
			gcry_md_close(stribogHandle);
			return "";
		}
		std::string sha512 = HelperFunctions::getHexString(digest, gcry_md_get_algo_dlen(GCRY_MD_SHA512));
		gcry_md_close(stribogHandle);
		return sha512;
	}
	catch(const std::exception& ex)
    {
    	_bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return "";
}

bool Io::writeLockFile(int fileDescriptor, bool wait)
{
    struct flock lock{};
    lock.l_type = F_WRLCK;
    lock.l_start = 0;
    lock.l_whence = SEEK_SET;
    lock.l_len = 0;
    return fcntl(fileDescriptor, wait ? F_SETLKW : F_SETLK, &lock) != -1;
}

bool Io::readLockFile(int fileDescriptor, bool wait)
{
    struct flock lock{};
    lock.l_type = F_RDLCK;
    lock.l_start = 0;
    lock.l_whence = SEEK_SET;
    lock.l_len = 0;
    return fcntl(fileDescriptor, wait ? F_SETLKW : F_SETLK, &lock) != -1;
}

}
