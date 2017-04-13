//
//  TimeVarConvolver.cpp
//  laproque - https://github.com/Buerner/laproque
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#include "TimeVarConvolver.hpp"
#include <math.h>
#include <cstring>

TimeVarConvolver::TimeVarConvolver(float* imp_resp, unsigned n_samples, unsigned block_size) :
Convolver(imp_resp, n_samples, block_size)
{
    _block_size = block_size;
    _setup_ramps();
    
    _fade_buffer = new float[_block_size];
    _other_freq_resp = fftwf_alloc_complex(_spectra_size);
}

TimeVarConvolver::TimeVarConvolver( TimeVarConvolver& tvconv)
: Convolver(tvconv)
{
    _setup_ramps();
    
    _fade_buffer = new float[_block_size];
    _other_freq_resp = fftwf_alloc_complex(_spectra_size);
}

TimeVarConvolver::~TimeVarConvolver()
{
    delete [] _up_ramp;
    delete [] _down_ramp;
    delete [] _fade_buffer;
    
    fftwf_free( _other_freq_resp );
}

void TimeVarConvolver::_setup_ramps()
{
    _up_ramp = new float[_block_size];
    _down_ramp = new float[_block_size];
    
    float phase = 0.;
    float phase_increment = (M_PI/2.) / float(_block_size);
    
    for ( unsigned idx = 0; idx < _block_size; idx++ ) {
        _down_ramp[idx] = cosf( phase );
        _down_ramp[idx] *= _down_ramp[idx];
        _up_ramp[idx] = 1. - _down_ramp[idx];
        
        phase += phase_increment;
    }
}

void TimeVarConvolver::process( float *in_buffer, float *out_buffer )
{
    // Copy input into the buffer assigned to the FFT.
    memcpy( _input+_block_size, in_buffer, _block_size*sizeof(float) );
    
    _fast_conv();
    
    if ( _has_changed.load() ) {
        // Save current output for fading.
        memcpy( _fade_buffer, _result+_block_size, _block_size*sizeof(float) );
        
        // Switch buffers for frequency response partitions
        _pointer_backup = _freq_resp_parts;
        _freq_resp_parts = _other_freq_resp;
        _other_freq_resp = _pointer_backup;
        
        // Compute convolution with new frequency responses
        _fast_conv();
        
        // Apply fading between the two resuling blocks with ramps.
        _result += _block_size;
        for ( unsigned idx = _block_size; idx--; ) {
            _result[idx] = _fade_buffer[idx] * _down_ramp[idx] + _result[idx] * _up_ramp[idx];
        }
        _result -= _block_size;
        
        _has_changed.store( false );
    }
    
    // Copy result to the output buffer.
    memcpy( out_buffer, _result+_block_size, _block_size*sizeof(float) );
    
    // Save last input.
    memcpy( _input, in_buffer, _block_size*sizeof(float) );
    
    // Shift the input spectrum buffer for next run.
    memcpy( _input_spectra+_spectrum_size, _input_spectra, ( _spectra_size-_spectrum_size)*sizeof(fftwf_complex) );
}

void TimeVarConvolver::set_partitions( fftwf_complex *new_partitions )
{
    memcpy( _other_freq_resp, new_partitions, _spectra_size*sizeof(fftwf_complex) );
    _has_changed.store( true );
}
