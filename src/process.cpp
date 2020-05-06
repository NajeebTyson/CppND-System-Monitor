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

Process::Process(int pid): pid_(pid) {
  user_ = LinuxParser::User(pid);
  command_ = LinuxParser::Command(pid);
}

// DONE: Return this process's ID
int Process::Pid() {
  return pid_;
}

// TODO: Return this process's CPU utilization
float Process::CpuUtilization() {
  long process_active_jiffies = LinuxParser::ActiveJiffies(pid_);
  return 100.0 * ((process_active_jiffies / float(sysconf(_SC_CLK_TCK))) / float(LinuxParser::UpTime(pid_)));
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

// TODO: Overload the "less than" comparison operator for Process objects
// REMOVE: [[maybe_unused]] once you define the function
bool Process::operator<(Process const& a[[maybe_unused]]) const { return true; }
