//
//  FilteredDelay.cpp
//  laproque - https://github.com/Buerner/laproque
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#include "FilteredDelay.hpp"

FilteredDelay::FilteredDelay( unsigned n_delay
                              , unsigned max_delay
                              , std::vector<float> co_freqs
                              , unsigned sample_rate
                              )
: Delay( n_delay, max_delay )
{
    _filterbank.renew( co_freqs );
    
    _n_bands = unsigned(co_freqs.size()) + 1;
    _filterbank.set_co_freqs( co_freqs ) ;
    _filterbank.set_sample_rate( sample_rate );
    _band_weights.resize( _n_bands );
    
    // Reserver memory for Filterbank outputs
    _band_buffer = new float*[_n_bands];
    for ( unsigned band = 0; band < _n_bands; band++) {
        _band_buffer[band] = new float[_intern_buff_size];
    }
    
    _delay_buffer = new float[_intern_buff_size];
    
    // Set weights to one
    reset_weights();
}

FilteredDelay::~FilteredDelay()
{
    for ( unsigned band = 0; band < _n_bands; band++) {
        delete [] _band_buffer[band];
    }
    delete [] _band_buffer;
    
    delete [] _delay_buffer;
}

float FilteredDelay::operator()( float input )
{
    float output;
    Delay::process( &input, &output, 1 );
    
    _filterbank( output, _band_buffer[0] );
    
    output = 0.f;
    for ( unsigned band = 0; band < _n_bands; band++ ) {
        output += _band_buffer[0][band] * _band_weights[band];
    }
    
    return output;
}

void FilteredDelay::process( float *input, float *output, unsigned long n_frames )
{
    _n_remaining = n_frames;
    
    while (_n_remaining) {
        // Update number of processable samples in this iteration.
        _n_remaining < _intern_buff_size ? _n_ready = _n_remaining : _n_ready = _intern_buff_size;
        
        // Process input in delay.
        Delay::process( input, _delay_buffer, _n_ready );
        
        // Process input in filterbank.
        _filterbank.process( _delay_buffer, _band_buffer, _n_ready );
        
        // Apply band weights.
        for ( unsigned idx = 0; idx < _n_ready; idx++ ) {
            output[idx] = 0.f;
            for ( unsigned band = 0; band < _n_bands; band++ ) {
                output[idx] += _band_buffer[band][idx] * _band_weights[band];
            }
        }
        // Increment buffers.
        input += _n_ready;
        output += _n_ready;
        
        // Reduce number of remaining frames.
        _n_remaining -= _n_ready;
    }
}

void FilteredDelay::reset_weights()
{
    _band_weights.resize( _n_bands );
    for ( unsigned band = 0; band < _n_bands; band++ ) {
        _band_weights[band] = 1.f;
    }
}

void FilteredDelay::set_co_freqs( std::vector<float> new_co_freqs )
{
    _filterbank.set_co_freqs( new_co_freqs );
}

void FilteredDelay::set_all_weights( std::vector<float> new_band_weights )
{
    if ( _band_weights.size() == new_band_weights.size() ) {
        _band_weights = new_band_weights;
    }
}

void FilteredDelay::set_band_weight( float weight, unsigned int band_idx )
{
    if ( band_idx < _band_weights.size() ) {
        _band_weights[band_idx] = weight;
    }
}

unsigned FilteredDelay::get_n_bands()
{
    return _n_bands;
}
