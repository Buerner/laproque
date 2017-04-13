//
//  Filter.cpp
//  laproque - https://github.com/Buerner/laproque
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//


#include "Filter.hpp"
#include "misc/include/timekeeper.hpp"

#include <math.h>
#include <iostream>

#include <pmmintrin.h>


// constructor
Filter::Filter( filter_type type,
                float cutoff_freq,
                unsigned sample_rate
               )
{
    _f_type = type;
    _cutoff_freq = cutoff_freq;
    _sample_rate = sample_rate;
    
    _compute_coeffs();
    
}

void Filter::set_sample_rate( unsigned int sample_rate )
{
    _sample_rate = sample_rate;
    _compute_coeffs();
}

float Filter::get_cutoff_freq()
{
    return _cutoff_freq;
    
}

// function which calculates the filter coefficient
void Filter::_compute_coeffs()
{
    float c = 1.f / tanf(M_PI * (_cutoff_freq/_sample_rate));
    
    _b_coeffs[0] = 1.f / (1.f+c);
    _b_coeffs[1] = _b_coeffs[0];
    
    _a_coeffs[0] = 1;
    _a_coeffs[1] = (1.f-c) / (1.f+c);

    
    if (_f_type == HIGH) {
        _b_coeffs[0] = 1 - _b_coeffs[0];
        _b_coeffs[1] = - _b_coeffs[0];
    }
    
};

// overload braces operator to compute the next output value
float Filter::operator()(float in_sample)
{
    float non_recursive = in_sample * _b_coeffs[0];
    float recursive = 0;
    
    non_recursive += _b_coeffs[1] * _in_dlyline;
    recursive += _a_coeffs[1] * _out_dlyline;
    
    float out_sample = non_recursive - recursive;
    
    _in_dlyline = in_sample;
    _out_dlyline = out_sample;
    
    return out_sample * _a_coeffs[0];
}

// Function which processes muliple samples to reduce function calls.
void Filter::process( float* input, float* output, unsigned long long n_frames )
{
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
    
    output[0] = input[0]*_b_coeffs[0] + _in_dlyline*_b_coeffs[1] - _out_dlyline*_a_coeffs[1];

    // No need to store delay line values internally in this part.
    for ( unsigned long long idx = 1; idx < n_frames; idx++ ) {
        output[idx] = input[idx]*_b_coeffs[0] + input[idx-1]*_b_coeffs[1] - output[idx-1]*_a_coeffs[1];
    }
    
    // Backup delayline states.
    _in_dlyline_backup = _in_dlyline;
    _out_dlyline_backup = _out_dlyline;
    
    // Store last input and output values to delayline.
    _in_dlyline = input[n_frames-1];
    _out_dlyline = output[n_frames-1];
}

void Filter::set_cutoff_freq( float co_freq )
{
    _cutoff_freq = co_freq;
    _compute_coeffs();
}

void Filter::reset()
{
    _in_dlyline = 0.f;
    _out_dlyline = 0.f;
}

void Filter::reverse()
{
    _in_dlyline = _in_dlyline_backup;
    _out_dlyline = _out_dlyline_backup;
}
