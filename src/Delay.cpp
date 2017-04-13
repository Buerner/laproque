//
//  Delay.cpp
//  laproque - https://github.com/Buerner/laproque
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#include "Delay.hpp"
#include <algorithm>
#include <cstring>

Delay::Delay( unsigned n_delay, unsigned max_delay )
{
    _buffer_size = max_delay;
    _n_delay = n_delay;
    
    _buffer = new float[max_delay];
    _buffer_end = _buffer + _buffer_size;
    reset();
}

Delay::~Delay()
{
    delete [] _buffer;
}

float Delay::get_one()
{    
    if ( _writer == _buffer_end ) _writer = _buffer;
    if ( _reader == _buffer_end ) _reader = _buffer;
    
    return *_reader++;
}

void Delay::set_one( float input )
{
    if ( _writer == _buffer_end ) _writer = _buffer;
    if ( _reader == _buffer_end ) _reader = _buffer;
    
    *_writer++ = input;
}

long Delay::get_delay()
{
    if ( _writer > _reader ) return _writer - _reader;
    else
    {
        return (_writer - _buffer) + (_buffer_end - _reader);
    }
}

float Delay::operator()( float input )
{
    if ( _writer == _buffer_end ) _writer = _buffer;
    if ( _reader == _buffer_end ) _reader = _buffer;
    
    *_writer++ = input;
    return *_reader++;
}

void Delay::process( float *input, float* output, unsigned long n_samples )
{
    // There is nothing to do when the delay is 0.
    if ( _n_delay == 0 ) {
        memcpy( output, input, n_samples*sizeof(float) );
        return;
    }
    
    _n_remaining = long(n_samples);
    
    while ( _n_remaining ) {
        
        // Check if writer is in front of reader.
        if ( _writer > _reader ) {
            // Only copy until writer reaches _buffer end.
            _n_ready = std::min( _n_remaining, _buffer_end - _writer );
        }
        else {
            // Only copy until reader reaches buffer end.
            _n_ready = std::min( _n_remaining, _buffer_end - _reader );
        }
        
        // Only copy until new input is needed.
        _n_ready = std::min( _n_ready, _n_delay );
        
        std::memcpy( output, _reader, unsigned(_n_ready)*sizeof(float) );
        _reader += _n_ready;
        
        std::memcpy( _writer, input, unsigned(_n_ready)*sizeof(float) );
        _writer += _n_ready;
        
        _n_remaining -= _n_ready;

        if ( _writer  == _buffer_end ) _writer = _buffer;
        if ( _reader  == _buffer_end ) _reader = _buffer;
        
        input += _n_ready;
        output += _n_ready;
    }
}

void Delay::set_delay( long new_delay )
{
    if ( new_delay < _buffer_size) {
        _reader = _writer - new_delay;
        if ( _reader < _buffer ) _reader += _buffer_size;
        
        _n_delay = new_delay;
    }
}

void Delay::reset()
{
    for ( int idx = 0; idx < _buffer_size; idx++ )
    {
        _buffer[idx] = 0.f;
    }

    _reader = _buffer;
    _writer = _buffer + _n_delay;
}

void Delay::replace_buffer( float *sample_data, unsigned int n_frames )
{
    // Check if n_frames exceeds buffer size.
    if ( ptrdiff_t(n_frames) <= _buffer_size) {
        
        ptrdiff_t part1 = _buffer_end - _writer;
        if ( part1 > ptrdiff_t(n_frames) ) {
            part1 = n_frames;
        }
        memcpy( _writer, sample_data, (unsigned long)part1 * sizeof(float) );
        memcpy( _buffer, sample_data+part1, (unsigned long)(n_frames-part1) * sizeof(float) );
    }
}
