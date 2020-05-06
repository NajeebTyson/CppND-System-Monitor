#include <unistd.h>
#include <cstddef>
#include <set>
#include <string>
#include <vector>

#include "process.h"
#include "processor.h"
#include "system.h"
#include "linux_parser.h"

using std::set;
using std::size_t;
using std::string;
using std::vector;


System::System() {
  this->kernel_ = LinuxParser::Kernel();
  this->os_ = LinuxParser::OperatingSystem();
}

// DONE: Return the system's CPU
Processor& System::Cpu() {
  return cpu_;
}

// DONE: Return a container composed of the system's processes
vector<Process>& System::Processes() {
  vector<int> pids = LinuxParser::Pids();

  // remove processes which are closed
  processes_.erase(std::remove_if(processes_.begin(), processes_.end(), [&pids](const Process& p){
    return std::find(pids.begin(), pids.end(), p.Pid()) == pids.end();
  }), processes_.end());

  set<int> existing_pids;
  for (const Process &process: processes_) {
    existing_pids.insert(process.Pid());
  }

  // put new processes
  for (int pid: pids) {
    if (existing_pids.find(pid) == existing_pids.end()) {
      processes_.emplace_back(pid);
    }
  }

  // refresh CPU usage
  for (Process &process: processes_) {
    process.refresh();
  }

  // sort the processes by their cpu utilization
  std::sort(processes_.begin(), processes_.end());

  return processes_;
}

// DONE: Return the system's kernel identifier (string)
std::string System::Kernel() {
  return kernel_;
}

// DONE: Return the system's memory utilization
float System::MemoryUtilization() {
  return LinuxParser::MemoryUtilization();
}

// DONE: Return the operating system name
std::string System::OperatingSystem() {
  return os_;
}

// DONE: Return the number of processes actively running on the system
int System::RunningProcesses() {
  return LinuxParser::RunningProcesses();
}

// DONE: Return the total number of processes on the system
int System::TotalProcesses() {
  return LinuxParser::TotalProcesses();
}

// DONE: Return the number of seconds since the system started running
long int System::UpTime() {
  return LinuxParser::UpTime();
}
