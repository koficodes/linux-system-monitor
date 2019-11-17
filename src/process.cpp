#include "process.h"
#include <unistd.h>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "format.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

// TODO: Return this process's ID
int Process::Pid() { return pid_; }

// TODO: Return this process's CPU utilization
float Process::CpuUtilization() {
  string line;
  string path = LinuxParser::kProcDirectory + to_string(pid_) +
                LinuxParser::kStatFilename;
  std::ifstream stream(path);
  std::istream_iterator<string> start(stream), end;
  std::vector<string> pidValues{start, end};
  float uptime = LinuxParser::UpTime();
  float utime = std::stof(pidValues[13]);
  float stime = std::stof(pidValues[14]);
  float cutime = std::stof(pidValues[15]);
  float cstime = std::stof(pidValues[16]);
  float starttime = std::stof(pidValues[21]);
  float hertz = sysconf(_SC_CLK_TCK);

  // including child processes
  float total_time = utime + stime + cutime + cstime;

  float seconds = uptime - (starttime / hertz);
  stream.close();
  return (total_time / hertz) / seconds;
}

string Process::Command() { return LinuxParser::Command(pid_); }

// TODO: Return this process's memory utilization
string Process::Ram() { return LinuxParser::Ram(pid_); }

// TODO: Return the user (name) that generated this process
string Process::User() { return LinuxParser::User(pid_); }

// TODO: Return the age of this process (in seconds)
long int Process::UpTime() { return LinuxParser::UpTime(pid_); }

// TODO: Overload the "less than" comparison operator for Process objects
// REMOVE: [[maybe_unused]] once you define the function
bool Process::operator<(Process& a) {
  return this->CpuUtilization() > a.CpuUtilization();
}