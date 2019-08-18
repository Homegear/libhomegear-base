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

#include "ProcessManager.h"
#include "../HelperFunctions/HelperFunctions.h"

#include <cstring>
#include <array>
#include <mutex>
#include <map>
#include <unordered_map>
#include <list>
#include <atomic>
#include <thread>
#include <iostream>
#include <condition_variable>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

namespace BaseLib
{
class ProcessManager::OpaquePointer
{
public:
    static int32_t _currentId;
    static std::mutex _callbackHandlersMutex;
    static std::unordered_map<int32_t, std::function<void(pid_t pid, int exitCode, int signal, bool coreDumped)>> _callbackHandlers;
    static std::atomic_bool _stopSignalHandlerThread;
    static std::thread _signalHandlerThread;

    static std::mutex _lastExitStatusMutex;
    static std::condition_variable _lastExitStatusConditionVariable;
    struct ExitInfo
    {
        int64_t time = 0;
        int exitCode = -1;
    };
    static std::unordered_map<pid_t, ExitInfo> _lastExitStatus;

    static void signalHandler()
    {
        sigset_t set{};
        timespec timeout{};
        timeout.tv_sec = 0;
        timeout.tv_nsec = 100000000;
        int signalNumber = -1;
        int exitCode = -1;
        sigemptyset(&set);
        sigaddset(&set, SIGCHLD);

        while(!_stopSignalHandlerThread)
        {
            try
            {
                siginfo_t info{};
                signalNumber = sigtimedwait(&set, &info, &timeout);
                if(signalNumber != SIGCHLD) continue;

                auto pid = info.si_pid;
                int status = 0;
                auto result = waitpid(pid, &status, 0);
                if(result == -1) std::cerr << "Error in waitpid for process " << pid << ": " << strerror(errno) << std::endl;
                exitCode = (result == -1 ? -1 : WEXITSTATUS(status));
                bool coreDumped = false;
                int childSignalNumber = -1;
                if(WIFSIGNALED(status))
                {
                    childSignalNumber = WTERMSIG(status);
                    if(WCOREDUMP(status)) coreDumped = true;
                }

                {
                    std::lock_guard<std::mutex> lastExitStatusGuard(_lastExitStatusMutex);
                    auto time = HelperFunctions::getTime();
                    _lastExitStatus.erase(pid);
                    ExitInfo exitInfo{};
                    exitInfo.time = time;
                    exitInfo.exitCode = exitCode;
                    _lastExitStatus.emplace(pid, std::move(exitInfo));
                    std::unordered_set<pid_t> entriesToErase;
                    for(auto& entry : _lastExitStatus)
                    {
                        if(time - entry.second.time > 60000) entriesToErase.emplace(entry.first);
                    }
                    for(auto entry : entriesToErase)
                    {
                        _lastExitStatus.erase(entry);
                    }
                }

                _lastExitStatusConditionVariable.notify_all();

                {
                    std::lock_guard<std::mutex> callbackHandlersGuard(_callbackHandlersMutex);
                    for(auto& callbackHandler : _callbackHandlers)
                    {
                        callbackHandler.second(pid, exitCode, childSignalNumber, coreDumped);
                    }
                }
            }
            catch(const std::exception& ex)
            {
                std::cerr << "Error in ProcessManager::signalHandler: " << ex.what() << std::endl;
            }
        }
    }
};

int32_t ProcessManager::OpaquePointer::_currentId = 0;
std::mutex ProcessManager::OpaquePointer::_callbackHandlersMutex;
std::unordered_map<int32_t, std::function<void(pid_t pid, int exitCode, int signal, bool coreDumped)>> ProcessManager::OpaquePointer::_callbackHandlers;
std::mutex ProcessManager::OpaquePointer::_lastExitStatusMutex;
std::condition_variable ProcessManager::OpaquePointer::_lastExitStatusConditionVariable;
std::unordered_map<pid_t, ProcessManager::OpaquePointer::ExitInfo> ProcessManager::OpaquePointer::_lastExitStatus;
std::atomic_bool ProcessManager::OpaquePointer::_stopSignalHandlerThread{true};
std::thread ProcessManager::OpaquePointer::_signalHandlerThread;

void ProcessManager::startSignalHandler()
{
    OpaquePointer::_stopSignalHandlerThread = false;

    sigset_t set{};
    sigemptyset(&set);
    sigprocmask(SIG_BLOCK, nullptr, &set);
    sigaddset(&set, SIGCHLD);
    sigprocmask(SIG_BLOCK, &set, nullptr);

    OpaquePointer::_signalHandlerThread = std::thread(&OpaquePointer::signalHandler);
}

void ProcessManager::startSignalHandler(BaseLib::ThreadManager& threadManager)
{
    OpaquePointer::_stopSignalHandlerThread = false;

    sigset_t set{};
    sigemptyset(&set);
    sigprocmask(SIG_BLOCK, nullptr, &set);
    sigaddset(&set, SIGCHLD);
    sigprocmask(SIG_BLOCK, &set, nullptr);

    threadManager.start(OpaquePointer::_signalHandlerThread, true, &ProcessManager::OpaquePointer::signalHandler);
}

void ProcessManager::stopSignalHandler()
{
    OpaquePointer::_stopSignalHandlerThread = true;
    if(OpaquePointer::_signalHandlerThread.joinable()) OpaquePointer::_signalHandlerThread.join();
    OpaquePointer::_lastExitStatusConditionVariable.notify_all();
}

void ProcessManager::stopSignalHandler(BaseLib::ThreadManager& threadManager)
{
    OpaquePointer::_stopSignalHandlerThread = true;
    threadManager.join(OpaquePointer::_signalHandlerThread);
    OpaquePointer::_lastExitStatusConditionVariable.notify_all();
}

int32_t ProcessManager::registerCallbackHandler(std::function<void(pid_t pid, int exitCode, int signal, bool coreDumped)> callbackHandler)
{
    std::lock_guard<std::mutex> callbackHandlersGuard(OpaquePointer::_callbackHandlersMutex);
    int32_t currentId = -1;
    while(currentId == -1 || OpaquePointer::_callbackHandlers.find(currentId) != OpaquePointer::_callbackHandlers.end()) currentId = OpaquePointer::_currentId++;
    OpaquePointer::_callbackHandlers[currentId].swap(callbackHandler);
    return currentId;
}

void ProcessManager::unregisterCallbackHandler(int32_t id)
{
    if(id == -1) return;
    std::lock_guard<std::mutex> callbackHandlersGuard(OpaquePointer::_callbackHandlersMutex);
    OpaquePointer::_callbackHandlers.erase(id);
}

std::vector<std::string> ProcessManager::splitArguments(const std::string& arguments)
{
    std::list<std::string> argumentList;
    std::string currentArgument;
    currentArgument.reserve(1024);
    bool doubleQuoted = false;
    bool singleQuoted = false;
    bool escaped = false;
    for(int32_t i = 0; i < (signed)arguments.size(); i++)
    {
        if(escaped)
        {
            escaped = false;
            currentArgument.push_back(arguments[i]);
        }
        else if(!doubleQuoted && !singleQuoted && arguments[i] == '"') doubleQuoted = true;
        else if(!doubleQuoted && !singleQuoted && arguments[i] == '\'') singleQuoted = true;
        else if(doubleQuoted && arguments[i] == '"') doubleQuoted = false;
        else if(singleQuoted && arguments[i] == '\'') singleQuoted = false;
        else if((doubleQuoted || singleQuoted) && arguments[i] == '\\') escaped = true;
        else if(!singleQuoted && !doubleQuoted && arguments[i] == ' ')
        {
            if(!currentArgument.empty()) argumentList.push_back(currentArgument);
            currentArgument.clear();
        }
        else currentArgument.push_back(arguments[i]);

        if(currentArgument.size() + 1 > currentArgument.capacity()) currentArgument.reserve(currentArgument.size() + 1024);
    }

    if(!currentArgument.empty()) argumentList.push_back(currentArgument);

    std::vector<std::string> argumentVector;
    argumentVector.reserve(argumentList.size());
    for(auto& argument : argumentList)
    {
        argumentVector.push_back(argument);
    }
    return argumentVector;
}

pid_t ProcessManager::system(const std::string& command, const std::vector<std::string>& arguments, int maxFd)
{
    pid_t pid;

    if(command.empty() || command.back() == '/') return -1;

    pid = fork();

    if(pid == -1) return pid;
    else if(pid == 0)
    {
        //Child process

        sigset_t set{};
        sigemptyset(&set);
        sigaddset(&set, SIGCHLD);
        sigaddset(&set, SIGHUP);
        sigaddset(&set, SIGTERM);
        sigaddset(&set, SIGINT);
        sigaddset(&set, SIGABRT);
        sigaddset(&set, SIGSEGV);
        sigaddset(&set, SIGQUIT);
        sigaddset(&set, SIGILL);
        sigaddset(&set, SIGFPE);
        sigaddset(&set, SIGALRM);
        sigaddset(&set, SIGUSR1);
        sigaddset(&set, SIGUSR2);
        sigaddset(&set, SIGTSTP);
        sigaddset(&set, SIGTTIN);
        sigaddset(&set, SIGTTOU);
        sigprocmask(SIG_UNBLOCK, &set, nullptr);

        // Close all non standard descriptors
        for(int32_t i = 3; i < maxFd; ++i)
        {
            close(i);
        }

        setsid();
        std::string programName = (command.find('/') == std::string::npos) ? command : command.substr(command.find_last_of('/') + 1);
        if(programName.empty()) _exit(1);
        char* argv[arguments.size() + 2];
        argv[0] = (char*) programName.c_str(); //Dirty, but as argv is not modified, there are no problems. Since C++11 the data is null terminated.
        for(int32_t i = 0; i < (signed)arguments.size(); i++)
        {
            argv[i + 1] = (char*)arguments[i].c_str();
        }
        argv[arguments.size() + 1] = nullptr;
        if(execv(command.c_str(), argv) == -1) _exit(1);
    }

    //Parent process

    return pid;
}

pid_t ProcessManager::systemp(const std::string& command, const std::vector<std::string>& arguments, int maxFd, int& stdIn, int& stdOut, int& stdErr)
{
    pid_t pid;

    stdIn = -1;
    stdOut = -1;
    stdErr = -1;

    if(command.empty() || command.back() == '/') return -1;

    int pipeIn[2];
    int pipeOut[2];
    int pipeErr[2];

    if(pipe(pipeIn) == -1) throw ProcessException("Error: Couln't create pipe for STDIN.");

    if(pipe(pipeOut) == -1)
    {
        close(pipeIn[0]);
        close(pipeIn[1]);
        throw ProcessException("Error: Couln't create pipe for STDOUT.");
    }

    if(pipe(pipeErr) == -1)
    {
        close(pipeIn[0]);
        close(pipeIn[1]);
        close(pipeOut[0]);
        close(pipeOut[1]);
        throw ProcessException("Error: Couln't create pipe for STDERR.");
    }

    pid = fork();

    if(pid == -1)
    {
        close(pipeIn[0]);
        close(pipeIn[1]);
        close(pipeOut[0]);
        close(pipeOut[1]);
        close(pipeErr[0]);
        close(pipeErr[1]);

        return pid;
    }
    else if(pid == 0)
    {
        //Child process

        sigset_t set{};
        sigemptyset(&set);
        sigaddset(&set, SIGCHLD);
        sigaddset(&set, SIGHUP);
        sigaddset(&set, SIGTERM);
        sigaddset(&set, SIGINT);
        sigaddset(&set, SIGABRT);
        sigaddset(&set, SIGSEGV);
        sigaddset(&set, SIGQUIT);
        sigaddset(&set, SIGILL);
        sigaddset(&set, SIGFPE);
        sigaddset(&set, SIGALRM);
        sigaddset(&set, SIGUSR1);
        sigaddset(&set, SIGUSR2);
        sigaddset(&set, SIGTSTP);
        sigaddset(&set, SIGTTIN);
        sigaddset(&set, SIGTTOU);
        sigprocmask(SIG_UNBLOCK, &set, nullptr);

        if(dup2(pipeIn[0], STDIN_FILENO) == -1) _exit(1);

        if(dup2(pipeOut[1], STDOUT_FILENO) == -1) _exit(1);

        if(dup2(pipeErr[1], STDERR_FILENO) == -1) _exit(1);

        //Close pipes for child
        close(pipeIn[0]);
        close(pipeIn[1]);
        close(pipeOut[0]);
        close(pipeOut[1]);
        close(pipeErr[0]);
        close(pipeErr[1]);

        // Close all non standard descriptors. Don't do this before calling dup2 otherwise dup2 will fail.
        for(int32_t i = 3; i < maxFd; ++i) close(i);

        setsid();
        std::string programName = (command.find('/') == std::string::npos) ? command : command.substr(command.find_last_of('/') + 1);
        if(programName.empty()) _exit(1);
        char* argv[arguments.size() + 2];
        argv[0] = (char*)programName.c_str(); //Dirty, but as argv is not modified, there are no problems. Since C++11 the data is null terminated.
        for(int32_t i = 0; i < (signed)arguments.size(); i++)
        {
            argv[i + 1] = (char*)arguments[i].c_str();
        }
        argv[arguments.size() + 1] = nullptr;
        if(execv(command.c_str(), argv) == -1) _exit(1);
    }

    //Parent process
    close(pipeIn[0]);
    close(pipeOut[1]);
    close(pipeErr[1]);

    stdIn = pipeIn[1];
    stdOut = pipeOut[0];
    stdErr = pipeErr[0];

    return pid;
}


int32_t ProcessManager::exec(const std::string& command, int maxFd, std::string& output)
{
    pid_t pid = 0;
    FILE* pipe = popen2(command.c_str(), "r", maxFd, pid);
    if(!pipe) return -1;
    try
    {
        std::array<char, 512> buffer{};
        output.reserve(1024);
        while(!feof(pipe))
        {
            if(fgets(buffer.data(), buffer.size(), pipe) != nullptr)
            {
                if(output.size() + buffer.size() > output.capacity()) output.reserve(output.capacity() + 1024);
                output.insert(output.end(), buffer.begin(), buffer.begin() + strnlen(buffer.data(), buffer.size()));
            }
        }
    }
    catch(...)
    {
    }
    fclose(pipe);

    if(std::this_thread::get_id() == OpaquePointer::_signalHandlerThread.get_id())
    {
        //Prevent deadlock when exec is called from signalHandlerThread.
        throw ProcessException("Error: exec called from signal handler thread. The process was executed, but can't return exit code.");
    }

    while(!OpaquePointer::_stopSignalHandlerThread)
    {
        std::unique_lock<std::mutex> lock(OpaquePointer::_lastExitStatusMutex);
        OpaquePointer::_lastExitStatusConditionVariable.wait_for(lock, std::chrono::milliseconds(1000), [&] { return OpaquePointer::_stopSignalHandlerThread || OpaquePointer::_lastExitStatus.find(pid) != OpaquePointer::_lastExitStatus.end(); });

        auto entry = OpaquePointer::_lastExitStatus.find(pid);
        if(entry != OpaquePointer::_lastExitStatus.end())
        {
            return entry->second.exitCode;
        }
    }

    return -1;
}

FILE* ProcessManager::popen2(const std::string& command, const std::string& type, int maxFd, pid_t& pid)
{
    int fd[2];
    if(pipe(fd) == -1) throw ProcessException("Error: Couln't create pipe.");

    pid = fork();
    if(pid == -1)
    {
        close(fd[0]);
        close(fd[1]);

        return nullptr;
    }

    if(pid == 0)
    {
        //Child process

        sigset_t set{};
        sigemptyset(&set);
        sigaddset(&set, SIGCHLD);
        sigaddset(&set, SIGHUP);
        sigaddset(&set, SIGTERM);
        sigaddset(&set, SIGINT);
        sigaddset(&set, SIGABRT);
        sigaddset(&set, SIGSEGV);
        sigaddset(&set, SIGQUIT);
        sigaddset(&set, SIGILL);
        sigaddset(&set, SIGFPE);
        sigaddset(&set, SIGALRM);
        sigaddset(&set, SIGUSR1);
        sigaddset(&set, SIGUSR2);
        sigaddset(&set, SIGTSTP);
        sigaddset(&set, SIGTTIN);
        sigaddset(&set, SIGTTOU);
        sigprocmask(SIG_UNBLOCK, &set, nullptr);

        if (type == "r")
        {
            if(dup2(fd[1], STDOUT_FILENO) == -1) _exit(1);
        }
        else
        {
            if(dup2(fd[0], STDIN_FILENO) == -1) _exit(1);
        }

        close(fd[0]);
        close(fd[1]);

        // Close all non standard descriptors. Don't do this before calling dup2 otherwise dup2 will fail.
        for(int32_t i = 3; i < maxFd; ++i) close(i);

        setsid();
        execl("/bin/sh", "/bin/sh", "-c", command.c_str(), nullptr);
        exit(0);
    }
    else
    {
        if (type == "r")
        {
            close(fd[1]);
        }
        else
        {
            close(fd[0]);
        }
    }

    if (type == "r")
    {
        return fdopen(fd[0], "r");
    }

    return fdopen(fd[1], "w");
}

}