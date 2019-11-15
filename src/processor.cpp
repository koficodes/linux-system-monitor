#include "processor.h"
#include <chrono>
#include <iostream>
#include <thread>
#include "linux_parser.h"
#include "tuple"
using namespace LinuxParser;
using std::stoi;

std::vector<int> fromStringsToInts(const std::vector<std::string>& cpuValues) {
  std::vector<int> cpuValues_;
  for (auto& i : cpuValues) {
    cpuValues_.push_back(stoi(i));
  }
  return cpuValues_;
}
std::tuple<float, float> sumCpuValues(
    const std::vector<std::string>& cpuValues) {
  auto cpuValuesInts = fromStringsToInts(cpuValues);
  float idle =
      cpuValuesInts[CPUStates::kIdle_] + cpuValuesInts[CPUStates::kIOwait_];
  float nonIdle =
      cpuValuesInts[CPUStates::kUser_] + cpuValuesInts[CPUStates::kNice_] +
      cpuValuesInts[CPUStates::kSteal_] + cpuValuesInts[CPUStates::kIRQ_] +
      cpuValuesInts[CPUStates::kSoftIRQ_] + cpuValuesInts[CPUStates::kSteal_] +
      cpuValuesInts[CPUStates::kGuest_] + cpuValuesInts[CPUStates::kGuestNice_];
  float total = idle + nonIdle;
  return {total, idle};
}

// TODO: Return the aggregate CPU utilization
float Processor::Utilization() {
  auto [total, idle] = sumCpuValues(LinuxParser::CpuUtilization());
  std::this_thread::sleep_for(std::chrono::microseconds(100000));
  auto [total2, idle2] = sumCpuValues(LinuxParser::CpuUtilization());

  return 100.0 * (((total2 - total) - (idle2 - idle)) / (total2 - total));
}