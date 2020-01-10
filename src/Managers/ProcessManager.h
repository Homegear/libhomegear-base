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

#ifndef LIBHOMEGEAR_BASE_PROCESSMANAGER_H
#define LIBHOMEGEAR_BASE_PROCESSMANAGER_H

#include "../Exception.h"
#include "ThreadManager.h"
#include "Environment.h"

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <signal.h>

namespace BaseLib
{

/**
 * Exception class for Process.
 *
 * @see Process
 */
class ProcessException : public Exception
{
public:
    explicit ProcessException(const std::string& message) : Exception(message) {}
};

class ProcessManager
{
public:
    ProcessManager() = delete;

    /**
     * Starts the signal handler thread. Must be called from main thread. You need to call stopSignalHandler() before
     * exiting the program. When using ThreadManager, pass it to this method.
     */
    static void startSignalHandler();

    /**
     * Starts the signal handler thread. Must be called from main thread. You need to call stopSignalHandler() before
     * exiting the program. When using ThreadManager, pass it to this method.
     */
    static void startSignalHandler(BaseLib::ThreadManager& threadManager);
    static void stopSignalHandler();
    static void stopSignalHandler(BaseLib::ThreadManager& threadManager);

    static int32_t registerCallbackHandler(std::function<void(pid_t pid, int exitCode, int signal, bool coreDumped)> callbackHandler);
    static void unregisterCallbackHandler(int32_t id);

    static std::vector<std::string> splitArguments(const std::string& arguments);

    static std::string findProgramInPath(const std::string& relativePath);

    /**
     * Starts a program and returns the process id.
     *
     * @param path The program to start. If path is relative, PATH is searched.
     * @param arguments The arguments to pass.
     * @param maxFd The maximum possible file descriptor. Needed to close all open file descriptors in the child process.
     * @throws Exception
     * @return Returns the PID on success and "-1" on error.
     */
    static pid_t system(const std::string& path, const std::vector<std::string>& arguments, int maxFd);

    /**
     * Starts a program with redirected pipe and returns the process id.
     *
     * @param path The program to start. If path is relative, PATH is searched.
     * @param arguments The arguments to pass.
     * @param[out] stdIn Will be filled with the input file descriptor. Close it when the process finishes.
     * @param[out] stdOut Will be filled with the output file descriptor. Close it when the process finishes.
     * @param[out] stdErr Will be filled with the error file descriptor. Close it when the process finishes.
     * @throws Exception
     * @return Returns the PID on success and "-1" on error.
     */
    static pid_t systemp(const std::string& path, const std::vector<std::string>& arguments, int maxFd, int& stdIn, int& stdOut, int& stdErr);

    /**
     * Starts a program and returns the output.
     *
     * @param command The command to execute.
     * @param maxFd The maximum number of file descriptors.
     * @param[out] output The program output.
     * @return Returns the programs exit code.
     */
    static int32_t exec(const std::string& command, int maxFd, std::string& output);

    /**
     * Starts a program and detaches it, so it continues to run when the parent process finishes.
     *
     * @param command The command to execute.
     * @param maxFd The maximum number of file descriptors.
     * @return Returns true when the process started successfully.
     */
    static bool exec(const std::string& command, int maxFd);

    /**
     * Custom implementation of popen that returns the pid.
     *
     * @param command
     * @param type
     * @param pid
     * @return
     */
    static FILE* popen2(const std::string& command, const std::string& type, int maxFd, pid_t& pid);
private:
    struct OpaquePointer;
};

}

#endif //LIBHOMEGEAR_BASE_PROCESSMANAGER_H
