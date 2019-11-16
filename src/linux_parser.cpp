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

// DONE: An example of how to read data from the filesystem
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

// DONE: An example of how to read data from the filesystem
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

// BONUS: Update this to use std::filesystem
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

// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  std::string total_memory, free_memory, line, unit, info_name;
  std::string path = kProcDirectory + kMeminfoFilename;
  std::ifstream stream(path);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> info_name >> total_memory >> unit;

    std::getline(stream, line);
    std::istringstream linestream2(line);
    linestream2 >> info_name >> free_memory >> unit;
    float rem_memory = std::stof(total_memory) - std::stof(free_memory);
    return 100 * (rem_memory / std::stof(total_memory));
  }

  return 0.0;
}

// TODO: Read and return the system uptime
long LinuxParser::UpTime() {
  std::string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    float uptime, idletime;
    linestream >> uptime >> idletime;
    return uptime;
  }
  return 0;
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { return 0; }

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid[[maybe_unused]]) { return 0; }

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() { return 0; }

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() { return 0; }

// TODO: Read and return CPU utilization

vector<string> LinuxParser::CpuUtilization() {
  std::string cpu, line, user, nice, system, idle, iowait, irq, softirq, steal,
      guest, guest_nice;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> cpu >> user >> nice >> system >> idle >> iowait >> irq >>
        softirq >> steal >> guest >> guest_nice;
    return std::vector{user, nice,    system, idle,  iowait,
                       irq,  softirq, steal,  guest, guest_nice};
  }
  return {};
}

// TODO: Read and return the total number of processes

int LinuxParser::GetValueWithKey(std::string key, std::string path) {
  std::string line, keyInFile;
  int valueInFile;
  std::ifstream stream(path);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> keyInFile >> valueInFile) {
        if (keyInFile == key) {
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

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  return GetValueWithKey("procs_running", LinuxParser::kProcDirectory +
                                              LinuxParser::kStatFilename);
}

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) {
  string line;
  std::ifstream stream(LinuxParser::kProcDirectory + to_string(pid) +
                       LinuxParser::kCmdlineFilename);
  std::getline(stream, line);
  return line;
}

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid) {
  string path = LinuxParser::kProcDirectory + to_string(pid) +
                LinuxParser::kStatusFilename;
  return to_string(LinuxParser::GetValueWithKey("VmSize:", path) /
                   float(1024 * 1024));
}

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) {
  string path = LinuxParser::kProcDirectory + to_string(pid) +
                LinuxParser::kStatusFilename;
  return to_string(LinuxParser::GetValueWithKey("Uid:", path));
}

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function

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

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
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