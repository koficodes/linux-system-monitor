#include <string>

#include "format.h"

using std::string;
using std::to_string;

// TODO: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
// REMOVE: [[maybe_unused]] once you define the function
string Format::ElapsedTime(long seconds) {
    long remaining = 0;
    long second = 1;
    long minute_s = 60 * second;
    long hour_s = 60 * minute_s;
    long hours = seconds / hour_s;
    remaining = seconds - (hours * hour_s);
    long minutes = remaining / minute_s;
    remaining = remaining- (minutes * minute_s);
    
    return to_string(hours)+ ":"  + to_string(minutes) + ":"  + to_string(remaining); 
}