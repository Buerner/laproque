//
//  TimeKeeper.cpp
//  laproque - https://github.com/Buerner/laproque
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#include "misc/include/timekeeper.hpp"
#include <math.h>

void TimeKeeper::start()
{
    if ( _paused ) {
        duration<double, std::micro> diff = duration_cast<microseconds>( high_resolution_clock::now() - _pause_point );
        _pause_offset += diff.count();
    }
    else _start = high_resolution_clock::now();    
    
    _paused = false;
    _is_running = true;
}

void TimeKeeper::stop()
{
    _elapsed = duration_cast<microseconds>( high_resolution_clock::now() - _start );
    _pause_offset = 0;
    _is_running = false;
}

void TimeKeeper::pause()
{
    _pause_point = high_resolution_clock::now();
    _paused = true;
}

void TimeKeeper::lap()
{
    if (_is_running) {
        duration<double, std::micro> lap = high_resolution_clock::now() - _start ;
        _laps.push_back( lap );
    }
    else {
        printf("Lap can not be saved. TimeKeeper is not running.\n");
    }
}

unsigned long long TimeKeeper::get_hours_elapsed()
{
    return floor(_elapsed.count() * 1e-6 / 3600.);
}

unsigned long long TimeKeeper::get_minutes_elapsed()
{
    return floor(_elapsed.count() * 1e-6 / 60.) ;
}

unsigned long long TimeKeeper::get_seconds_elapsed()
{
    return _elapsed.count() / 1000000;
}

unsigned long long TimeKeeper::get_microseconds_elapsed()
{
    if ( !_is_running ) {
        return _elapsed.count() - _pause_offset;
    }
    else {
        auto elapsed = duration_cast<microseconds>( high_resolution_clock::now() - _start );
        return elapsed.count() - _pause_offset;
    }
}

double TimeKeeper::get_avarage()
{
    double average = 0.;
    
    if (_laps.size() > 0) {
        for ( auto lap : _laps )
            average += lap.count();
    }
    average /= _laps.size();
    return average;
}

void TimeKeeper::print_elapsed()
{   unsigned long long hours = get_hours_elapsed();
    unsigned long long minutes = get_minutes_elapsed();
    unsigned long long seconds = get_seconds_elapsed();
    unsigned long long centiseconds = get_microseconds_elapsed();

    centiseconds %= 10000;
    seconds -= minutes * 60;
    minutes -= hours * 60;

    printf( "%04llu:%02llu:%02llu.%04llu\n", hours, minutes, seconds, centiseconds  );
}

void TimeKeeper::print_estimate( unsigned int n_remaining_laps )
{
    double remaining = get_avarage() * n_remaining_laps;
    double hours = remaining / 3600.f * 1e-6;
    double minutes = (hours - floor(hours)) * 60.f ;
    double seconds = (minutes - floor(minutes)) * 60.f;
    double centiseconds = (seconds - floor(seconds)) *1e4;

    printf( "%04i:%02i:%02i.%04i", unsigned(hours), unsigned(minutes), unsigned(seconds), unsigned(centiseconds) );
}

void TimeKeeper::reset()
{
    _is_running = false;
    _laps.clear();
}

