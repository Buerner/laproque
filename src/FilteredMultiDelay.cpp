//
//  FilteredMultiDelay.cpp
//  laproque - https://github.com/Buerner/laproque
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#include "FilteredMultiDelay.hpp"
#include "misc/include/timekeeper.hpp"

FilteredMultiDelay::FilteredMultiDelay( unsigned n_bands, unsigned sample_rate, unsigned block_size, unsigned max_delay )
: MultiDelay( max_delay )
, _filterbank( freqs, sample_rate )
{
    _block_size = block_size;
    _n_bands = n_bands;
    
    _band_weights = new float[n_bands];
    _internal_buffer = new float[block_size];
    
    _band_buffer = new float*[n_bands];
    for (unsigned band = 0; band < _n_bands; band++) {
        _band_buffer[band] = new float[_block_size];
        _band_weights[band] = 1.f;
    }
}

FilteredMultiDelay::~FilteredMultiDelay()
{
    for ( unsigned band = 0; band < _n_bands; band++ ) {
        delete [] _band_buffer[band];
    }
    delete [] _band_buffer;
    delete [] _internal_buffer;
    delete [] _band_weights;
}

void FilteredMultiDelay::process( float* input, float* output, unsigned long n_frames )
{
    unsigned idx, band;

    _filterbank.process( input, _band_buffer, n_frames );
    
    // Apply frequency band weights
    for ( idx = 0; idx < n_frames; idx++)
    {
        _internal_buffer[idx] = 0.f;
        // Weighted sum of all bands.
        for ( band = 0; band < _n_bands; band++) {
            _internal_buffer[idx] += _band_buffer[band][idx] * _band_weights[band];
        }
    }
    MultiDelay::process( _internal_buffer, output, n_frames );
}

unsigned FilteredMultiDelay::get_n_bands()
{
    return _n_bands;
}

void FilteredMultiDelay::set_band_weight( float new_band_weight, unsigned band_idx )
{
    if (band_idx < _n_bands) {
        _band_weights[band_idx] = new_band_weight;
    }
}

void FilteredMultiDelay::set_band_weights( std::vector< float > weights )
{
    for ( unsigned idx = 0; idx < _n_bands; idx++ ) {
        _band_weights[idx] = weights[idx];
    }
}

void FilteredMultiDelay::set_sample_rate( unsigned int sample_rate )
{
    _filterbank.set_sample_rate( sample_rate );
}

void FilteredMultiDelay::set_co_freqs( std::vector<float> co_freqs )
{
    _filterbank.set_co_freqs( co_freqs );
}

void FilteredMultiDelay::replace_buffer( float* sample_data, unsigned long n_frames )
{
    MultiDelay::replace_buffer( sample_data, n_frames );
    _filterbank.reverse();
}
