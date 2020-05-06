#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>

#include "process.h"
#include "../include/linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid): pid_(pid), prev_proc_cpu_time(0), prev_system_cpu_time(0) {
  user_ = LinuxParser::User(pid);
  command_ = LinuxParser::Command(pid);
  CpuUtilization();
}

// DONE: Return this process's ID
int Process::Pid() {
  return pid_;
}

// DONE: Return this process's CPU utilization
float Process::CpuUtilization() {
//  long total_time = LinuxParser::ActiveJiffies(pid_);
//  cpu_utilization_ = 100.0 * ((total_time / float(sysconf(_SC_CLK_TCK))) / float(LinuxParser::UpTime(pid_)));
//  return cpu_utilization_;
    long system_cpu_time = LinuxParser::ActiveJiffies();
    long proc_cpu_time = LinuxParser::ActiveJiffies(pid_);
    cpu_utilization_ = 100.0 * (float(proc_cpu_time) - prev_proc_cpu_time);
    cpu_utilization_ /= (float(system_cpu_time) - prev_system_cpu_time);
    prev_proc_cpu_time = proc_cpu_time;
    prev_system_cpu_time = system_cpu_time;
    return cpu_utilization_;
}

// DONE: Return the command that generated this process
string Process::Command() {
  return command_;
}

// DONE: Return this process's memory utilization
string Process::Ram() {
  // converting the ram to MB from KB
  std::stringstream stream;
  stream << std::fixed << std::setprecision(2) << (LinuxParser::Ram(pid_) / 1024.0);
  return stream.str();
}

// DONE: Return the user (name) that generated this process
string Process::User() {
  return user_;
}

// DONE: Return the age of this process (in seconds)
long int Process::UpTime() {
  return LinuxParser::UpTime(pid_);
}

// DONE: Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const {
  return cpu_utilization_ > a.cpu_utilization_;
}
