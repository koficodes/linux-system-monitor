#include "linux_parser.h"
#include <dirent.h>
#include <unistd.h>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// Read and return Operating system name
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// Read and return Kernel version
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// Read and return process pids
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  std::string totalMemory, freeMemory, line, unit, infoName;
  std::string path = kProcDirectory + kMeminfoFilename;
  std::ifstream stream(path);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> infoName >> totalMemory >> unit;

    std::getline(stream, line);
    std::istringstream linestream2(line);
    linestream2 >> infoName >> freeMemory >> unit;
    float remainingMemory = std::stof(totalMemory) - std::stof(freeMemory);
    stream.close();
    return (remainingMemory / std::stof(totalMemory));
  }

  return 0.0;
}

// Read and return the system uptime
long LinuxParser::UpTime() {
  std::string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    float uptime, idletime;
    linestream >> uptime >> idletime;
    stream.close();
    return uptime;
  }
  return 0;
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { return UpTime() * sysconf(_SC_CLK_TCK); }

// Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  string line, token;
  vector<string> values;
  std::ifstream filestream(LinuxParser::kProcDirectory + to_string(pid) +
                           LinuxParser::kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    while (linestream >> token) {
      values.push_back(token);
    }
  }
  long jiffies{0};
  if (values.size() > 21) {
    long user = stol(values[13]);
    long kernel = stol(values[14]);
    long children_user = stol(values[15]);
    long children_kernel = stol(values[16]);
    jiffies = user + kernel + children_user + children_kernel;
  }
  return jiffies;
}

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  vector<string> time = CpuUtilization();
  return (stol(time[CPUStates::kUser_]) + stol(time[CPUStates::kNice_]) +
          stol(time[CPUStates::kSystem_]) + stol(time[CPUStates::kIRQ_]) +
          stol(time[CPUStates::kSoftIRQ_]) + stol(time[CPUStates::kSteal_]) +
          stol(time[CPUStates::kGuest_]) + stol(time[CPUStates::kGuestNice_]));
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  vector<string> time = CpuUtilization();
  return (stol(time[CPUStates::kIdle_]) + stol(time[CPUStates::kIOwait_]));
}

// Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  std::string cpu, line, user, nice, system, idle, iowait, irq, softirq, steal,
      guest, guest_nice;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> cpu >> user >> nice >> system >> idle >> iowait >> irq >>
        softirq >> steal >> guest >> guest_nice;
    stream.close();
    return std::vector{user, nice,    system, idle,  iowait,
                       irq,  softirq, steal,  guest, guest_nice};
  }
  return {};
}

// Read and return the total number of processes

int LinuxParser::GetValueWithKey(std::string key, std::string path) {
  std::string line, keyInFile;
  int valueInFile;
  std::ifstream stream(path);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> keyInFile >> valueInFile) {
        if (keyInFile == key) {
          stream.close();
          return valueInFile;
        }
      }
    }
  }
  return valueInFile;
}
int LinuxParser::TotalProcesses() {
  return GetValueWithKey(
      "processes", LinuxParser::kProcDirectory + LinuxParser::kStatFilename);
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  return GetValueWithKey("procs_running", LinuxParser::kProcDirectory +
                                              LinuxParser::kStatFilename);
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  string line;
  std::ifstream stream(LinuxParser::kProcDirectory + to_string(pid) +
                       LinuxParser::kCmdlineFilename);
  std::getline(stream, line);
  return line;
}

// Read and return the memory used by a process

string LinuxParser::Ram(int pid) {
  string path = LinuxParser::kProcDirectory + to_string(pid) +
                LinuxParser::kStatusFilename;

  // VmData is used here instead VmSize because
  // VmData gives the exact physical memory being used as a part of Physical RAM
  // ref: http://man7.org/linux/man-pages/man5/proc.5.html

  return to_string(LinuxParser::GetValueWithKey("VmSize:", path) /
                   float(1024 * 1024));
}

// Read and return the user ID associated with a process

string LinuxParser::Uid(int pid) {
  string path = LinuxParser::kProcDirectory + to_string(pid) +
                LinuxParser::kStatusFilename;
  return to_string(LinuxParser::GetValueWithKey("Uid:", path));
}

// Read and return the user associated with a process

string LinuxParser::User(int pid) {
  string uid = LinuxParser::Uid(pid);
  string path = LinuxParser::kPasswordPath;
  string line;
  std::ifstream stream(path);
  while (std::getline(stream, line)) {
    std::replace(line.begin(), line.end(), ':', ' ');
    std::istringstream linestream(line);
    string name, x, uid_;
    linestream >> name >> x >> uid_;
    if (uid == uid_) {
      return name;
    }
  }
  return "";
}

// Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  string line;
  string path =
      LinuxParser::kProcDirectory + to_string(pid) + LinuxParser::kStatFilename;
  std::ifstream stream(path);
  std::istream_iterator<string> start(stream), end;
  std::vector<string> pidValues{start, end};
  int utimeIndex = 13;
  float clockTicks = sysconf(_SC_CLK_TCK);
  float utime = std::stof(pidValues[utimeIndex]);
  return (long)utime / clockTicks;
}