#include <string>
#include <sstream>
#include <iomanip>

#include "format.h"

using std::string;
using std::to_string;

// DONE: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
// REMOVE: [[maybe_unused]] once you define the function
string Format::ElapsedTime(long seconds) {
  std::stringstream stream;
  stream << std::setw(2) << std::setfill('0');
  stream << to_string(seconds / 3600) << ':';  // hours
  stream << std::setw(2) << std::setfill('0');
  stream << to_string(seconds % 3600 / 60) << ':';  // minutes
  stream << std::setw(2) << std::setfill('0');
  stream << to_string(seconds % 60);  // seconds
  return stream.str();
}
