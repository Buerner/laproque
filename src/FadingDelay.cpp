//
//  FadingDelay.cpp
//  laproque - https://github.com/Buerner/laproque
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#include <math.h>
#include <algorithm>

#include "FadingDelay.hpp"

laproque::FadingDelay::FadingDelay( unsigned long fade_length, unsigned delay, unsigned max_delay ) : Delay( delay, max_delay )
{
    _fade_length = fade_length;
    
    _fadein_buf = new float[_fade_length];
    _fadeout_buf = new float[_fade_length];
    
    _setup_fades();
    
    }


void laproque::FadingDelay::process( float *input, float *output, unsigned long n_samples )
{
    _n_remaining = long(n_samples);
    
    if ( _has_changed.load() )
    {
        _old_reader = _reader;
        
        _reader =  _writer  - _n_delay;
        if ( _reader < _buffer ) _reader += _buffer_size;
        
        _to_fade = _fade_length;
        _has_changed.store( false );
    }
    
    if ( _to_fade )
    {
        unsigned long fade_now = std::min( n_samples,  _to_fade );
        
        if ( _fade_out.load() ) {
            for ( unsigned long idx = 0; idx < fade_now; idx++ )
            {
                output[idx] = (*_old_reader++) * (*_fadeout_buf++);
                output[idx] += operator()( input[idx] ) *  (*_fadein_buf++);
                
                if ( _old_reader >= _buffer_end ) _old_reader -= _buffer_size;
            }
        }
        
        else {
            for ( unsigned long idx = 0; idx < fade_now; idx++ )
            {
                output[idx] = operator()( input[idx] ) *  (*_fadein_buf++);
            }
        }
        
        _n_remaining -= fade_now;
        _to_fade -= fade_now;
        
        // Check if fading is completed in this process call and adjust buffers in case.
        if ( _to_fade == 0 )
        {
            _fadein_buf -= _fade_length;
            
            if ( _fade_out.load() ) {
                _fadeout_buf -= _fade_length;
            }
            
            
            output += fade_now;
            input += fade_now;
        }
    }
    
    Delay::process( input, output, (unsigned long)(_n_remaining) );
}

void laproque::FadingDelay::set_delay( long new_delay )
{
    if ( new_delay < _buffer_size) {
        _has_changed.store( true );
        _n_delay = new_delay;
    }
}

void laproque::FadingDelay::_setup_fades()
{
    float phase = 0;
    float phase_increment = M_PI_2 / (_fade_length-1);
    
    for ( unsigned long idx = 0; idx < _fade_length; idx++) {
        _fadein_buf[idx] = sinf( phase );
        _fadein_buf[idx] *= _fadein_buf[idx];
        
        _fadeout_buf[idx] = cosf( phase );
        _fadeout_buf[idx] *= _fadeout_buf[idx];
        phase += phase_increment;
        
        //printf("%.11f,\t%.11f,\n", _fadein_buf[idx], _fadeout_buf[idx]);
    }
}

void laproque::FadingDelay::reset()
{
    Delay::reset();
    _to_fade = 0;
}

void laproque::FadingDelay::set_fade_out( bool do_fade )
{
    _fade_out.store( do_fade );
}

laproque::FadingDelay::~FadingDelay()
{
    delete [] _fadein_buf;
    delete [] _fadeout_buf;
}
