#include "ncurses_display.h"
#include "system.h"

#include "linux_parser.h"
#include <iostream>
using std::cout;
int main() {
  cout << LinuxParser::OperatingSystem() << std::endl;
  cout << LinuxParser::Kernel() << std::endl;
  cout << LinuxParser::UpTime() << std::endl;
  LinuxParser::MemoryUtilization();
  auto cpu_data = LinuxParser::GetCpuData();
  cout << "CPU data" << std::endl;
  for (auto &data: cpu_data) {
    std::cout << data.second << " ";
  }
  cout << "\nTotal jiffies: " << LinuxParser::Jiffies() << std::endl;
  cout << "Active jiffies: " << LinuxParser::ActiveJiffies() << std::endl;
  cout << "Total processes: " << LinuxParser::TotalProcesses() << std::endl;
  cout << "Running processes: " << LinuxParser::RunningProcesses() << std::endl;

//  System system;
//  NCursesDisplay::Display(system);
}