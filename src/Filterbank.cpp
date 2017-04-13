//
//  Filterbank.cpp
//  laproque - https://github.com/Buerner/laproque
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#include "Filterbank.hpp"
#include <math.h>
#include <cstring>

laproque::Filterbank::Filterbank(
                       std::vector<float> co_freqs
                       , unsigned sample_rate
                       )
{
    _sample_rate = sample_rate;
    
    renew( co_freqs );
    
    _buffer1 = new float[_intern_buff_size];
    _buffer2 = new float[_intern_buff_size];
    
    _low_band = _buffer1;
}

laproque::Filterbank::~Filterbank()
{
    delete [] _buffer1;
    delete [] _buffer2;
}

void laproque::Filterbank::set_sample_rate( unsigned sample_rate )
{
    for ( unsigned idx = 0; idx < _n_filters; idx++) {
        _filters[idx].set_sample_rate( sample_rate );
    }
}

void laproque::Filterbank::operator()( float in_sample, float* bands )
{
    float low_band;

    low_band = in_sample;
    
    for ( unsigned band = _n_bands; --band ; ) {
        bands[band] = _filters[band*2-1](low_band);
        low_band = _filters[band*2-2](low_band);
    }
    bands[0] = low_band;
    
}

void laproque::Filterbank::_update_filters()
{
    for ( unsigned idx = 0; idx < _n_filters; idx+=2 ) {
        _filters[idx].set_cutoff_freq( _crossover_freqs[idx/2] );
        _filters[idx+1].set_cutoff_freq( _crossover_freqs[idx/2] );
    }
}

void laproque::Filterbank::set_co_freqs( std::vector<float> new_co_freqs )
{
    if ( new_co_freqs.size() == _crossover_freqs.size() ) {
        _crossover_freqs = new_co_freqs;
        _update_filters();
    }
}

void laproque::Filterbank::process( float *input, float **output, unsigned long n_frames )
{
    _n_remaining = n_frames;
    unsigned band;
    
    while ( _n_remaining ) {
        // Update number of processable frames;
        _n_remaining < _intern_buff_size ? _n_ready = _n_remaining : _n_ready = _intern_buff_size;
        
        memcpy( _low_band, input, _n_ready * sizeof(float) );
        
        // Process filters
        for ( band = _n_bands; --band; ) {
            // High Pass
            _filters[band*2-1].process( _low_band, output[band], _n_ready );
            
            // Low Pass
            if ( _low_band == _buffer1 ) {
                _filters[band*2-2].process( _low_band, _buffer2, _n_ready );
                _low_band = _buffer2;
            }
            else {
                _filters[band*2-2].process( _low_band, _buffer1, _n_ready );
                _low_band = _buffer1;
            }
        }
        
        memcpy( output[0], _low_band, _n_ready*sizeof(float) );
        
        // Increment buffers.
        input += _n_ready;
        for ( band = 0; band < _n_bands; band++ ) {
            output[band] += _n_ready;
        }
        
        // Reduce number of samples to be processed
        _n_remaining -= _n_ready;
    }
    // Reset band buffer pointer
    for ( band = 0; band < _n_bands; band++ ) {
        output[band] -= n_frames;
    }
}

void laproque::Filterbank::reverse()
{
    for ( unsigned idx = 0; idx < _n_filters; idx++ ) {
        _filters[idx].reverse();
    }
}

void laproque::Filterbank::renew( std::vector<float> new_co_freqs )
{
    _crossover_freqs = new_co_freqs;
    
    _n_bands = unsigned(_crossover_freqs.size()) + 1;
    _n_filters = (_n_bands-1) * 2;
    
    // Create Filter Objects
    _filters.resize( _n_filters );
    for ( unsigned idx = 0; idx < _n_filters; idx+=2 ) {
        _filters[idx] = Filter(LOW, _crossover_freqs[idx/2], _sample_rate);
        _filters[idx+1] = Filter(HIGH, _crossover_freqs[idx/2], _sample_rate);
    }
}

unsigned laproque::Filterbank::get_n_bands()
{
    return _n_bands;
}

void laproque::Filterbank::reset()
{
    for ( unsigned flt = 0; flt < _n_filters; flt++ ) {
        _filters[flt].reset();
    }
}

