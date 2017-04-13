//
//  TimeKeeper.hpp
//  laproque - https://github.com/Buerner/laproque
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#ifndef timekeeper_hpp
#define timekeeper_hpp

#include <stdio.h>
#include <chrono>
#include <vector>

using namespace std::chrono;

class TimeKeeper
{
public:
    void start();
    void pause();
    void stop();
    void lap();
    void print_elapsed();
    void print_estimate( unsigned n_remaining_laps );
    
    double get_avarage();
    
    unsigned long long get_hours_elapsed();
    unsigned long long get_minutes_elapsed();
    unsigned long long get_seconds_elapsed();
    unsigned long long get_microseconds_elapsed();
    
    void reset();
    
private:
    time_point<high_resolution_clock> _start;
    duration<double, std::micro> _elapsed;
    std::vector< duration<double, std::micro> > _laps;
    
    time_point<high_resolution_clock>  _pause_point;
    unsigned long long _pause_offset = 0;
    
    bool _is_running;
    bool _paused;
};


#endif /* timekeeper_hpp */

