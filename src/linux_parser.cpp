#include <unistd.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <numeric>
#include <experimental/filesystem>
#include <iostream>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;
namespace filesystem = std::experimental::filesystem;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream file_stream(kOSPath);
  if (file_stream.is_open()) {
    while (std::getline(file_stream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream line_stream(line);
      while (line_stream >> key >> value) {
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
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream line_stream(line);
    line_stream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
// DONE: By using std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  for (const auto &entry: filesystem::directory_iterator(kProcDirectory)) {
    // Is this a directory?
    if (filesystem::is_directory(entry.path())) {
      // Is every character of the name a digit?
      string filename{filesystem::path(entry.path()).filename()};
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  return pids;
}

// DONE: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  float memory_utilized = 0.0;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if (stream.is_open()) {
    string line, temp;
    std::getline(stream, line);
    std::istringstream total_mem_line_stream(line);
    float total_memory, free_memory;
    total_mem_line_stream >> temp >> total_memory;
    std::getline(stream, line);
    std::istringstream free_mem_line_stream(line);
    free_mem_line_stream >> temp >> free_memory;
    memory_utilized = total_memory - free_memory;
  }
  return memory_utilized;
}

// DONE: Read and return the system uptime
long LinuxParser::UpTime() {
  long uptime = 0;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    string line;
    std::getline(stream, line);
    std::istringstream line_stream(line);
    line_stream >> uptime;
  }
  return uptime;
}

// DONE: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  return sysconf(_SC_CLK_TCK) * UpTime();
}

// DONE: Read and return the number of active jiffies for a PID
/**
 * From Udacity Knowledge q&a, I came to know that
 * active jiffies for a process is actually the sum of
 * four attributes of a process, which are:
 * utime, stime, cutime, cstime
 */
long LinuxParser::ActiveJiffies(int pid) {
  long active_jiffies = 0;
  std::ifstream stream(kProcDirectory + to_string(pid) + "/stat");
  if (stream.is_open()) {
    string line;
    std::getline(stream, line);
    /**
     * Active Jiffies of any process is the sum of
     * 14th to 17th attributes in /proc/[pid]/stat file
     */
     std::istringstream line_stream(line);
     // ignoring first 13 values
     for(int i=0; i<13; ++i) {
       line_stream.ignore(256, ' ');
     }
     long utime, stime, cutime, cstime;
     line_stream >> utime >> stime >> cutime >> cstime;
     active_jiffies = utime + stime + cutime + cstime;
  }
  return active_jiffies;
}

// DONE: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  auto cpu_data = GetCpuData();
  return std::accumulate(cpu_data.begin(), cpu_data.end(), 0,
                         [](const long previous, const std::pair<LinuxParser::CPUStates, long>& p){
                           return previous + p.second;
                         });
}

// DONE: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  auto cpu_data = GetCpuData();
  return cpu_data[CPUStates::kIdle_] + cpu_data[CPUStates::kIOwait_];
}

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() { return {}; }

// DONE: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  //processes information is at line# 17 starting from 0
  return std::stoi(GetProcStatLineData(17));
}

// DONE: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  //processes information is at line# 18 starting from 0
  return std::stoi(GetProcStatLineData(18));
}

// DONE: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) {
  string command;
  std::ifstream stream(kProcDirectory + to_string(pid) + "/cmdline");
  if (stream.is_open()) {
    std::getline(stream, command);
  }
  return command;
}

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid[[maybe_unused]]) { return string(); }

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid[[maybe_unused]]) { return string(); }

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid[[maybe_unused]]) { return string(); }

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid[[maybe_unused]]) { return 0; }

// helper functions
std::unordered_map<LinuxParser::CPUStates, long> LinuxParser::GetCpuData() {
  std::ifstream stream(kProcDirectory + kStatFilename);
  std::unordered_map<LinuxParser::CPUStates, long> cpu_data;
  if (stream.is_open()) {
    string line, temp;
    long c_user, c_nice, c_system, c_idle, c_iowait;
    long c_irq, c_softirq, c_steal, c_guest, c_guestnice;

    std::getline(stream, line);
    std::istringstream line_stream(line);
    line_stream >> temp >> c_user >> c_nice >> c_system >> c_idle >> c_iowait;
    line_stream >> c_irq >> c_softirq >> c_steal >> c_guest >> c_guestnice;

    cpu_data[LinuxParser::CPUStates::kUser_] = c_user;
    cpu_data[LinuxParser::CPUStates::kNice_] = c_nice;
    cpu_data[LinuxParser::CPUStates::kSystem_] = c_system;
    cpu_data[LinuxParser::CPUStates::kIdle_] = c_idle;
    cpu_data[LinuxParser::CPUStates::kIOwait_] = c_iowait;
    cpu_data[LinuxParser::CPUStates::kIRQ_] = c_irq;
    cpu_data[LinuxParser::CPUStates::kSoftIRQ_] = c_softirq;
    cpu_data[LinuxParser::CPUStates::kSteal_] = c_steal;
    cpu_data[LinuxParser::CPUStates::kGuest_] = c_guest;
    cpu_data[LinuxParser::CPUStates::kGuestNice_] = c_guestnice;
  }
  return cpu_data;
}


std::string LinuxParser::GetProcStatLineData(unsigned int line_no) {
  // line index start from 0
  string line_data;
  // because /proc/stat file is only 20 lines long
  if (line_no >= 20) {
    return line_data;
  }
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    string line;
    for (unsigned int i = 0;  i < line_no; ++i) {
      std::getline(stream, line);
    }
    // line contains the data at line_no
    string temp;
    std::istringstream line_stream(line);
    line_stream >> temp >> line_data;
  }
  return line_data;
}