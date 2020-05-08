#include "processor.h"
#include "../include/linux_parser.h"

// DONE: Return the aggregate CPU utilization
float Processor::Utilization() {
  float total_time = LinuxParser::Jiffies();
  float active_time = LinuxParser::ActiveJiffies();
  return 1.0 * (active_time/total_time);
}