//
//  SincLP.cpp
//  laproque - https://github.com/Buerner/laproque
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#include "SincLP.hpp"
#include <math.h>
#include <cstring>



laproque::SincLP::SincLP(float cutoff_freq, float sample_rate, unsigned block_size, unsigned length) :
    _co_freq(cutoff_freq), _sample_rate(sample_rate), _block_size(block_size), _length(length)
{
    _dly_comp = unsigned( _length/2 );
    
    _in_buffer = new float[block_size + _dly_comp];
    _out_buffer = new float[block_size + _dly_comp];
    for ( unsigned idx = block_size + _dly_comp; idx--; ) {
        _in_buffer[idx] = 0.;
        _out_buffer[idx] = 0.;
    }
    
    float* imp_resp = new float[length];
    float cutoff_norm = cutoff_freq / sample_rate;
    
    // Compute winodw.
    float* window = new float[length];
    blackman(window, length);
    
    // Compute sinc values.
    float x_val;
    for ( unsigned idx = 0; idx < length ; idx++ ) {
        x_val = ( (float(idx) - floorf(length/2.)) * 2. * cutoff_norm ) * M_PI;
        
        if( idx - unsigned(length/2) == 0 ) {
            imp_resp[idx] = 2. * cutoff_norm * 1.;
        }
        else {
            imp_resp[idx] = 2. * cutoff_norm * ( sinf(x_val) / x_val );
        }
        
        // Apply window.
        imp_resp[idx] *= window[idx];
    }
    
    _convolver = new Convolver(imp_resp, length, block_size+_dly_comp);
    
    // Clean up.
    delete [] imp_resp;
    delete [] window;
    
    // Reset input buffer fo first use
    reset();
}


void laproque::SincLP::process(float *input, float *output)
{
    memcpy( _in_buffer, input, _block_size*sizeof(float) );
    _convolver->process( _in_buffer, _out_buffer );
    memcpy( output, _out_buffer+_dly_comp, _block_size*sizeof(float) );
}

void laproque::SincLP::reset()
{
    _convolver->reset_input_buffer();
}

laproque::SincLP::~SincLP()
{
    delete [] _in_buffer;
    delete [] _out_buffer;
    delete _convolver;
}
