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
#include "../BaseLib.h"
#include "../HelperFunctions/HelperFunctions.h"
#include "../HelperFunctions/Io.h"

#include <array>
#include <atomic>
#include <condition_variable>
#include <cstring>
#include <iostream>
#include <list>
#include <map>
#include <mutex>
#include <thread>
#include <unordered_map>

#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace BaseLib {
class ProcessManager::OpaquePointer {
public:
  static int32_t _currentId;
  static std::mutex _callbackHandlersMutex;
  static std::unordered_map<
      int32_t,
      std::function<void(pid_t pid, int exitCode, int signal, bool coreDumped)>>
      _callbackHandlers;
  static std::atomic_bool _stopSignalHandlerThread;
  static std::thread _signalHandlerThread;

  static std::mutex _lastExitStatusMutex;
  static std::condition_variable _lastExitStatusConditionVariable;
  struct ExitInfo {
    int64_t time = 0;
    int exitCode = -1;
  };
  static std::unordered_map<pid_t, ExitInfo> _lastExitStatus;

  static void signalHandler() {
    sigset_t set{};
    timespec timeout{};
    timeout.tv_sec = 0;
    timeout.tv_nsec = 100000000;
    int signalNumber = -1;
    int exitCode = -1;
    sigemptyset(&set);
    sigaddset(&set, SIGCHLD);
    pid_t pid = -1;
    int status = 0;

    while (!_stopSignalHandlerThread) {
      try {
        pid = waitpid(-1, &status, WNOHANG);
        if (pid == -1 ||
            pid == 0) // Possible errors: ECHILD, EINTR and EINVAL => No error
                      // or not possible => Don't output error message.
        {
          siginfo_t info{};
          signalNumber = sigtimedwait(&set, &info, &timeout);
          if (signalNumber != SIGCHLD)
            continue;
          pid = info.si_pid;

          auto result = waitpid(pid, &status, 0);
          if (result == -1)
            continue; // Signal was already handled
        }

        exitCode = WEXITSTATUS(status);
        bool coreDumped = false;
        int childSignalNumber = -1;
        if (WIFSIGNALED(status)) {
          childSignalNumber = WTERMSIG(status);
          if (WCOREDUMP(status))
            coreDumped = true;
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
          for (auto &entry : _lastExitStatus) {
            if (time - entry.second.time > 60000)
              entriesToErase.emplace(entry.first);
          }
          for (auto entry : entriesToErase) {
            _lastExitStatus.erase(entry);
          }
        }

        _lastExitStatusConditionVariable.notify_all();

        {
          std::lock_guard<std::mutex> callbackHandlersGuard(
              _callbackHandlersMutex);
          for (auto &callbackHandler : _callbackHandlers) {
            callbackHandler.second(pid, exitCode, childSignalNumber,
                                   coreDumped);
          }
        }
      } catch (const std::exception &ex) {
        std::cerr << "Error in ProcessManager::signalHandler: " << ex.what()
                  << std::endl;
      }
    }
  }
};

int32_t ProcessManager::OpaquePointer::_currentId = 0;
std::mutex ProcessManager::OpaquePointer::_callbackHandlersMutex;
std::unordered_map<int32_t, std::function<void(pid_t pid, int exitCode,
                                               int signal, bool coreDumped)>>
    ProcessManager::OpaquePointer::_callbackHandlers;
std::mutex ProcessManager::OpaquePointer::_lastExitStatusMutex;
std::condition_variable
    ProcessManager::OpaquePointer::_lastExitStatusConditionVariable;
std::unordered_map<pid_t, ProcessManager::OpaquePointer::ExitInfo>
    ProcessManager::OpaquePointer::_lastExitStatus;
std::atomic_bool ProcessManager::OpaquePointer::_stopSignalHandlerThread{true};
std::thread ProcessManager::OpaquePointer::_signalHandlerThread;

void ProcessManager::startSignalHandler() {
  OpaquePointer::_stopSignalHandlerThread = false;

  sigset_t set{};
  sigemptyset(&set);
  pthread_sigmask(SIG_BLOCK, nullptr, &set);
  sigaddset(&set, SIGCHLD);
  pthread_sigmask(SIG_BLOCK, &set, nullptr);

  OpaquePointer::_signalHandlerThread =
      std::thread(&OpaquePointer::signalHandler);
}

void ProcessManager::startSignalHandler(BaseLib::ThreadManager &threadManager) {
  OpaquePointer::_stopSignalHandlerThread = false;

  sigset_t set{};
  sigemptyset(&set);
  pthread_sigmask(SIG_BLOCK, nullptr, &set);
  sigaddset(&set, SIGCHLD);
  pthread_sigmask(SIG_BLOCK, &set, nullptr);

  threadManager.start(OpaquePointer::_signalHandlerThread, true,
                      &ProcessManager::OpaquePointer::signalHandler);
}

void ProcessManager::stopSignalHandler() {
  OpaquePointer::_stopSignalHandlerThread = true;
  if (OpaquePointer::_signalHandlerThread.joinable())
    OpaquePointer::_signalHandlerThread.join();
  OpaquePointer::_lastExitStatusConditionVariable.notify_all();
}

void ProcessManager::stopSignalHandler(BaseLib::ThreadManager &threadManager) {
  OpaquePointer::_stopSignalHandlerThread = true;
  threadManager.join(OpaquePointer::_signalHandlerThread);
  OpaquePointer::_lastExitStatusConditionVariable.notify_all();
}

int32_t ProcessManager::registerCallbackHandler(
    std::function<void(pid_t pid, int exitCode, int signal, bool coreDumped)>
        callbackHandler) {
  std::lock_guard<std::mutex> callbackHandlersGuard(
      OpaquePointer::_callbackHandlersMutex);
  int32_t currentId = -1;
  while (currentId == -1 || OpaquePointer::_callbackHandlers.find(currentId) !=
                                OpaquePointer::_callbackHandlers.end())
    currentId = OpaquePointer::_currentId++;
  OpaquePointer::_callbackHandlers[currentId].swap(callbackHandler);
  return currentId;
}

void ProcessManager::unregisterCallbackHandler(int32_t id) {
  if (id == -1)
    return;
  std::lock_guard<std::mutex> callbackHandlersGuard(
      OpaquePointer::_callbackHandlersMutex);
  OpaquePointer::_callbackHandlers.erase(id);
}

std::vector<std::string>
ProcessManager::splitArguments(const std::string &arguments) {
  std::list<std::string> argumentList;
  std::string currentArgument;
  currentArgument.reserve(1024);
  bool doubleQuoted = false;
  bool singleQuoted = false;
  bool escaped = false;
  for (int32_t i = 0; i < (signed)arguments.size(); i++) {
    if (escaped) {
      escaped = false;
      currentArgument.push_back(arguments[i]);
    } else if (!doubleQuoted && !singleQuoted && arguments[i] == '"')
      doubleQuoted = true;
    else if (!doubleQuoted && !singleQuoted && arguments[i] == '\'')
      singleQuoted = true;
    else if (doubleQuoted && arguments[i] == '"')
      doubleQuoted = false;
    else if (singleQuoted && arguments[i] == '\'')
      singleQuoted = false;
    else if ((doubleQuoted || singleQuoted) && arguments[i] == '\\')
      escaped = true;
    else if (!singleQuoted && !doubleQuoted && arguments[i] == ' ') {
      if (!currentArgument.empty())
        argumentList.push_back(currentArgument);
      currentArgument.clear();
    } else
      currentArgument.push_back(arguments[i]);

    if (currentArgument.size() + 1 > currentArgument.capacity())
      currentArgument.reserve(currentArgument.size() + 1024);
  }

  if (!currentArgument.empty())
    argumentList.push_back(currentArgument);

  std::vector<std::string> argumentVector;
  argumentVector.reserve(argumentList.size());
  for (auto &argument : argumentList) {
    argumentVector.push_back(argument);
  }
  return argumentVector;
}

std::string ProcessManager::findProgramInPath(const std::string &relativePath) {
  if (relativePath.empty())
    return "";
  if (Io::fileExists(relativePath))
    return relativePath;

  if (relativePath.front() != '/') {
    auto path = HelperFunctions::splitAll(Environment::get("PATH"), ':');
    for (auto &element : path) {
      HelperFunctions::trim(element);
      if (element.empty())
        continue;

      auto absolutePath =
          element.append(element.back() == '/' ? "" : "/").append(relativePath);

      if (Io::fileExists(absolutePath))
        return absolutePath;
    }
  }

  return "";
}

pid_t ProcessManager::system(const std::string &command,
                             const std::vector<std::string> &arguments,
                             int maxFd) {
  if (command.empty() || command.back() == '/')
    return -1;
  const std::string absolute_filename = findProgramInPath(command);
  if (absolute_filename.empty())
    return -1;

  const pid_t pid = fork();

  if (pid == -1)
    return pid;
  else if (pid == 0) {
    // Child process
    pthread_sigmask(SIG_SETMASK, &SharedObjects::defaultSignalMask, nullptr);

    // Close all non standard descriptors
    close_range(3, ~0U, CLOSE_RANGE_CLOEXEC | CLOSE_RANGE_UNSHARE);

    // Note that setsid() only returns -1 if the process already is a process
    // group leader, so we don't check for errors.
    setsid();
    const std::string program_name =
        (absolute_filename.find('/') == std::string::npos)
            ? absolute_filename
            : absolute_filename.substr(absolute_filename.find_last_of('/') + 1);
    if (program_name.empty())
      _exit(1);
    char *argv[arguments.size() + 2];
    argv[0] = const_cast<char *>(
        program_name
            .c_str()); // Dirty, but as argv is not modified, there are no
                       // problems. Since C++11 the data is null terminated.
    for (auto i = 0; i < arguments.size(); i++) {
      argv[i + 1] = const_cast<char *>(arguments[i].c_str());
    }
    argv[arguments.size() + 1] = nullptr;
    if (execv(absolute_filename.c_str(), argv) == -1)
      _exit(1);
  }

  // Parent process

  return pid;
}

pid_t ProcessManager::systemp(const std::string &command,
                              const std::vector<std::string> &arguments,
                              int maxFd, int &std_in, int &std_out,
                              int &std_err) {
  std_in = -1;
  std_out = -1;
  std_err = -1;

  if (command.empty() || command.back() == '/')
    return -1;
  const std::string absolute_filename = findProgramInPath(command);
  if (absolute_filename.empty())
    return -1;

  int pipe_in[2];
  int pipe_out[2];
  int pipe_err[2];

  if (pipe(pipe_in) == -1)
    throw ProcessException("Couln't create pipe for STDIN.");

  if (pipe(pipe_out) == -1) {
    close(pipe_in[0]);
    close(pipe_in[1]);
    throw ProcessException("Couln't create pipe for STDOUT.");
  }

  if (pipe(pipe_err) == -1) {
    close(pipe_in[0]);
    close(pipe_in[1]);
    close(pipe_out[0]);
    close(pipe_out[1]);
    throw ProcessException("Couln't create pipe for STDERR.");
  }

  const pid_t pid = fork();

  if (pid == -1) {
    close(pipe_in[0]);
    close(pipe_in[1]);
    close(pipe_out[0]);
    close(pipe_out[1]);
    close(pipe_err[0]);
    close(pipe_err[1]);

    return pid;
  } else if (pid == 0) {
    // Child process
    pthread_sigmask(SIG_SETMASK, &SharedObjects::defaultSignalMask, nullptr);

    if (dup2(pipe_in[0], STDIN_FILENO) == -1)
      _exit(1);

    if (dup2(pipe_out[1], STDOUT_FILENO) == -1)
      _exit(1);

    if (dup2(pipe_err[1], STDERR_FILENO) == -1)
      _exit(1);

    // Close pipes for child
    close(pipe_in[0]);
    close(pipe_in[1]);
    close(pipe_out[0]);
    close(pipe_out[1]);
    close(pipe_err[0]);
    close(pipe_err[1]);

    // Close all non standard descriptors. Don't do this before calling dup2
    // otherwise dup2 will fail.
    close_range(3, ~0U, CLOSE_RANGE_CLOEXEC | CLOSE_RANGE_UNSHARE);

    // Note that setsid() only returns -1 if the process already is a process
    // group leader, so we don't check for errors.
    setsid();
    const std::string program_name =
        (absolute_filename.find('/') == std::string::npos)
            ? absolute_filename
            : absolute_filename.substr(absolute_filename.find_last_of('/') + 1);
    if (program_name.empty())
      _exit(1);
    char *argv[arguments.size() + 2];
    argv[0] = const_cast<char *>(
        program_name
            .c_str()); // Dirty, but as argv is not modified, there are no
                       // problems. Since C++11 the data is null terminated.
    for (auto i = 0; i < arguments.size(); i++) {
      argv[i + 1] = const_cast<char *>(arguments[i].c_str());
    }
    argv[arguments.size() + 1] = nullptr;
    if (execv(absolute_filename.c_str(), argv) == -1)
      _exit(1);
  }

  // Parent process
  close(pipe_in[0]);
  close(pipe_out[1]);
  close(pipe_err[1]);

  std_in = pipe_in[1];
  std_out = pipe_out[0];
  std_err = pipe_err[0];

  return pid;
}

int32_t ProcessManager::exec(const std::string &command, int maxFd,
                             std::string &output) {
  pid_t pid = 0;
  FILE *pipe = popen2(command, "r", maxFd, pid);
  if (!pipe)
    return -1;
  try {
    std::array<char, 512> buffer{};
    output.reserve(1024);
    while (!feof(pipe)) {
      if (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        if (output.size() + buffer.size() > output.capacity())
          output.reserve(output.capacity() + 1024);
        output.insert(output.end(), buffer.begin(),
                      buffer.begin() + strnlen(buffer.data(), buffer.size()));
      }
    }
  } catch (...) {
  }
  fclose(pipe);

  if (std::this_thread::get_id() ==
      OpaquePointer::_signalHandlerThread.get_id()) {
    // Prevent deadlock when exec is called from signalHandlerThread.
    throw ProcessException("Error: exec called from signal handler thread. The "
                           "process was executed, but can't return exit code.");
  }

  while (!OpaquePointer::_stopSignalHandlerThread) {
    std::unique_lock<std::mutex> lock(OpaquePointer::_lastExitStatusMutex);
    OpaquePointer::_lastExitStatusConditionVariable.wait_for(
        lock, std::chrono::milliseconds(1000), [&] {
          return OpaquePointer::_stopSignalHandlerThread ||
                 OpaquePointer::_lastExitStatus.find(pid) !=
                     OpaquePointer::_lastExitStatus.end();
        });

    auto entry = OpaquePointer::_lastExitStatus.find(pid);
    if (entry != OpaquePointer::_lastExitStatus.end()) {
      return entry->second.exitCode;
    }
  }

  return -1;
}

bool ProcessManager::exec(const std::string &command, int maxFd) {
  // Start the new session without controlling terminals
  // Note that setsid() only returns -1 if the process already is a process
  // group leader, so we don't check for errors.
  setsid();

  const pid_t pid = fork();
  if (pid == -1) {
    // Parent => error
    return false;
  }
  if (pid > 0) {
    // Parent
    return true;
  }

  // Child
  pthread_sigmask(SIG_SETMASK, &SharedObjects::defaultSignalMask, nullptr);

  // Close all non standard descriptors.
  close_range(3, ~0U, CLOSE_RANGE_CLOEXEC | CLOSE_RANGE_UNSHARE);

  execl("/bin/sh", "/bin/sh", "-c", command.c_str(), nullptr);

  exit(0);
}

FILE *ProcessManager::popen2(const std::string &command,
                             const std::string &type, int maxFd, pid_t &pid) {
  int fd[2];
  if (pipe(fd) == -1)
    throw ProcessException("Couln't create pipe.");

  pid = fork();
  if (pid == -1) {
    close(fd[0]);
    close(fd[1]);

    return nullptr;
  }

  if (pid == 0) {
    // Child process
    pthread_sigmask(SIG_SETMASK, &SharedObjects::defaultSignalMask, nullptr);

    if (type == "r") {
      if (dup2(fd[1], STDOUT_FILENO) == -1)
        _exit(1);
    } else {
      if (dup2(fd[0], STDIN_FILENO) == -1)
        _exit(1);
    }

    close(fd[0]);
    close(fd[1]);

    // Close all non-standard descriptors. Don't do this before calling dup2
    // otherwise dup2 will fail.
    close_range(3, ~0U, CLOSE_RANGE_CLOEXEC | CLOSE_RANGE_UNSHARE);

    // Note that setsid() only returns -1 if the process already is a process
    // group leader, so we don't check for errors.
    setsid();
    execl("/bin/sh", "/bin/sh", "-c", command.c_str(), nullptr);
    exit(0);
  } else {
    if (type == "r") {
      close(fd[1]);
    } else {
      close(fd[0]);
    }
  }

  if (type == "r") {
    return fdopen(fd[0], "r");
  }

  return fdopen(fd[1], "w");
}

} // namespace BaseLib