#include <unistd.h>
#include <string>
#include <vector>
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
    memory_utilized = (total_memory - free_memory) / total_memory;
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
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
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
  auto cpu_data = CpuUtilization();
  return std::accumulate(cpu_data.begin(), cpu_data.end(), 0,
                         [](const long previous, const string& p){
                           return previous + std::stol(p);
                         });
}

// DONE: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  auto cpu_data = CpuUtilization();
  return std::stol(cpu_data[CPUStates::kIdle_]) + std::stol(cpu_data[CPUStates::kIOwait_]);
}

// DONE: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  vector<string> cpu_utilization(10);
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    string line, temp;
    string c_user, c_nice, c_system, c_idle, c_iowait;
    string c_irq, c_softirq, c_steal, c_guest, c_guestnice;

    std::getline(stream, line);
    std::istringstream line_stream(line);
    line_stream >> temp >> c_user >> c_nice >> c_system >> c_idle >> c_iowait;
    line_stream >> c_irq >> c_softirq >> c_steal >> c_guest >> c_guestnice;

    cpu_utilization[LinuxParser::CPUStates::kUser_] = c_user;
    cpu_utilization[LinuxParser::CPUStates::kNice_] = c_nice;
    cpu_utilization[LinuxParser::CPUStates::kSystem_] = c_system;
    cpu_utilization[LinuxParser::CPUStates::kIdle_] = c_idle;
    cpu_utilization[LinuxParser::CPUStates::kIOwait_] = c_iowait;
    cpu_utilization[LinuxParser::CPUStates::kIRQ_] = c_irq;
    cpu_utilization[LinuxParser::CPUStates::kSoftIRQ_] = c_softirq;
    cpu_utilization[LinuxParser::CPUStates::kSteal_] = c_steal;
    cpu_utilization[LinuxParser::CPUStates::kGuest_] = c_guest;
    cpu_utilization[LinuxParser::CPUStates::kGuestNice_] = c_guestnice;
  }
  return cpu_utilization;
}

// DONE: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  //processes information is at line# 16 starting from 0
  return std::stoi(GetProcStatLineData(16));
}

// DONE: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  //processes information is at line# 17 starting from 0
  return std::stoi(GetProcStatLineData(17));
}

// DONE: Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  string command_filename = kProcDirectory + to_string(pid) + kCmdlineFilename;
  std::istringstream line_stream(GetFileLineData(command_filename, 0));
  std::string command;
  line_stream >> command;
  return command;
}

// DONE: Read and return the memory used by a process
unsigned int LinuxParser::Ram(int pid) {
  unsigned int ram;
  string line_data, temp;
  string command_filename = kProcDirectory + to_string(pid) + kStatusFilename;
  //memory utilisation information is at line# 17 starting from 0
  line_data = GetFileLineData(command_filename, 17);
  std::istringstream line_stream(line_data);
  line_stream >> temp >> ram;
  return ram;
}

// DONE: Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  string uid;
  string line_data, temp;
  string command_filename = kProcDirectory + to_string(pid) + kStatusFilename;
  //process uid information is at line# 8 starting from 0
  line_data = GetFileLineData(command_filename, 8);
  std::istringstream line_stream(line_data);
  line_stream >> temp >> uid;
  return uid;
}

// DONE: Read and return the user associated with a process
string LinuxParser::User(int pid) {
  string username;
  string user_data = GetPasswdUserData(std::stoi(Uid(pid)));
  if (user_data.empty()) {
    return username;
  }
  std::istringstream line_stream(user_data);
  std::getline(line_stream, username, ':');
  return username;
}

// DONE: Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  long uptime = 0;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    string line;
    std::getline(stream, line);
    /**
     * Up time is the 21st attribute in /proc/[pid]/stat file, starting from 0
     */
    std::istringstream line_stream(line);
    // ignoring first 21 values
    for(int i=0; i<21; ++i) {
      line_stream.ignore(256, ' ');
    }
    long start_time;
    line_stream >> start_time;
    // converting clock ticks to seconds
    start_time /= sysconf(_SC_CLK_TCK);
    uptime = UpTime() - start_time;
  }
  return uptime;
}

// helper functions
std::string LinuxParser::GetProcStatLineData(unsigned int line_no) {
  string line_data;
  // because /proc/stat file is only 20 lines long
  if (line_no >= 20) {
    return line_data;
  }
  line_data = GetFileLineData(kProcDirectory + kStatFilename, line_no);
  // line contains the data at line_no
  string temp;
  std::istringstream line_stream(line_data);
  line_stream >> temp >> line_data;
  return line_data;
}

std::string LinuxParser::GetFileLineData(const std::string &filename, unsigned int line_no) {
  // line index start from 0
  string line_data;
  std::ifstream stream(filename);
  if (stream.is_open()) {
    unsigned int i = 0;
    while(getline(stream, line_data) && i < line_no) {
      ++i;
    }
    if (i != line_no) {
      line_data = "";
    }
  }
  return line_data;
}

std::string LinuxParser::GetPasswdUserData(unsigned int uid) {
  // line index start from 0
  string user_data, user_id_str, _;
  std::ifstream stream(kPasswordPath);
  if (stream.is_open()) {
    unsigned int user_id = -999;
    while(user_id != uid && getline(stream, user_data)) {
      std::istringstream line_stream(user_data);
      std::getline(line_stream, _, ':');
      std::getline(line_stream, _, ':');
      std::getline(line_stream, user_id_str, ':');
      user_id = std::stoi(user_id_str);
    }
    if (user_id != uid) {
      user_data = "";
    }
  }
  return user_data;
}