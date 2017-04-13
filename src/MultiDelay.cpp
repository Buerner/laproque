//
//  MultiDelay.cpp
//  laproque - https://github.com/Buerner/laproque
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#include "MultiDelay.hpp"
#include <algorithm>
#include <cstring>

laproque::MultiDelay::MultiDelay( unsigned max_delay )
{
    _buffer_size = max_delay;
    _buffer = new float[max_delay];
    _buffer_end = _buffer + _buffer_size;
    reset();
}

laproque::MultiDelay::~MultiDelay()
{
    delete [] _buffer;
}

float laproque::MultiDelay::operator() ( float input )
{
    float result = 0;
    if ( _writer >= _buffer_end ) { _writer -= _buffer_size; }
    
    for ( unsigned idx = 0; idx < _readers.size(); idx++ )
    {
        if ( _readers[idx] >= _buffer_end ) _readers[idx] -= _buffer_size;
        result += *_readers[idx]++ * _weights[idx];
    }
    
    *_writer++ = input;
    return result;
}

void laproque::MultiDelay::get_one( float* output )
{
    if ( _writer - _buffer >= _buffer_size ) { _writer -= _buffer_size; }
    
    for ( unsigned idx = 0; idx < _readers.size(); idx++ )
    {
        if ( _readers[idx] - _buffer >= _buffer_size ) { _readers[idx] -= _buffer_size; }
        output[idx] = *_readers[idx]++;
    }
}

void laproque::MultiDelay::set_one( float input )
{
    if ( _writer - _buffer >= _buffer_size ) { _writer -= _buffer_size; }
    *_writer++ = input;
}

void laproque::MultiDelay::process( float *input, float* output, unsigned long n_frames )
{
    for ( unsigned idx = 0; idx < n_frames; idx++) {
        output[idx] = operator()(input[idx]);
    }
//    _n_remaining = n_samples;
//    
//    for (int idx = 0; idx < n_samples; idx++) {
//        output[idx] = 0.f;
//    }
//    
//    while ( _n_remaining ) {
//        
//        // Find number of copyable output samples
//        _max_reader = *std::max_element( _readers.begin(), _readers.end() );
//        _n_ready_read = std::min( _min_delay, _buffer_end - _max_reader + 1  );
//        _n_ready_read = std::min( _n_ready_read, _n_remaining  );
//        
//        for ( unsigned idx = 0; idx < _n_delays; idx++ ) {
//            // Copy available samples.
//            //std::memcpy( output, _readers[idx], _n_ready_read*sizeof(float) );
//            for (int spl = 0; spl < _n_ready_read; spl++) {
//                output[spl] += _weights[idx] * _readers[idx][spl];
//                //printf("%.9f\n", _readers[idx][spl]);
//            }
//            
//            // Advance reader and output pointer
//            _readers[idx] += _n_ready_read;
//            
//            // Check if reader reached end if buffer
//            if ( _readers[idx] >= _buffer_end ) {
//                _readers[idx] -= _buffer_size;
//            }
//            
//        }
////        for (int spl = 0; spl < _n_ready_read; spl++) {
////            printf("%f\n", output[spl]);
////        }
//        
//        output += _n_ready_read;
//        
//        // Copy input samples to buffer while checking for boundries
//        if ( _n_ready_write >= _n_ready_read ) {
//            std::memcpy( _writer, input, _n_ready_read*sizeof(float) );
//            
//            input += _n_ready_read;
//            _writer += _n_ready_read;
//            _n_ready_write -= _n_ready_read;
//        }
//        else {
//            long rest = _n_ready_read - _n_ready_write;
//            
//            std::memcpy( _writer, input, _n_ready_write*sizeof(float) );
//            
//            input+= _n_ready_write;
//            _writer = _buffer;
//            
//            std::memcpy( _writer, input, rest*sizeof(float) );
//            
//            input += rest;
//            _writer += rest;
//            _n_ready_write = _buffer_size-rest;
//        }
//        
//        _n_remaining -= _n_ready_read;
//    }
}

void laproque::MultiDelay::add_delay( long n_samples_delay, float weight )
{
    // Check if delay already exists
    if(std::find(_n_samples_delay.begin(), _n_samples_delay.end(), n_samples_delay) != _n_samples_delay.end())
    {
        return;
    }
    
    // Check if delay value works with buffer size
    if ( n_samples_delay < _buffer_size && n_samples_delay > 0 )
    {
        float* new_reader = _writer - n_samples_delay;
        
        // Check if read pointer is in buffer range
        if ( new_reader <  _buffer ) { new_reader += _buffer_size; }
        
        _readers.push_back( new_reader );
        _n_samples_delay.push_back( n_samples_delay );
        _weights.push_back( weight );
        
//        if ( n_samples_delay < _min_delay ) { _min_delay = n_samples_delay; }
//        if ( n_samples_delay > _max_delay ) { _max_delay = n_samples_delay; }
        
        _n_delays++;
    }
}

void laproque::MultiDelay::set_delays( long* new_delays )
{
    for ( unsigned idx = 0; idx < _n_delays; idx++ )
    {
        float* reader = _writer - new_delays[idx];
        // Check if delay value works with buffer size
        if ( new_delays[idx] <= _buffer_size )
        {
            // Check if read pointer is in buffer range
            if ( reader < _buffer ) { reader += _buffer_size;; }
            
            _n_samples_delay[idx] = new_delays[idx];
            _readers[idx] = reader;
        }
    }
}

void laproque::MultiDelay::set_weights( float* new_weights )
{
    for ( unsigned idx = 0; idx < _n_delays; idx++ )
    {
        _weights[idx] = new_weights[idx];
    }
}

void laproque::MultiDelay::reset()
{
    // Set buffer values to 0
    for ( ptrdiff_t idx = 0; idx < _buffer_size; idx++ )
    {
        _buffer[idx] = 0.f;
    }
    
    // Reset read and write pointers
    for ( unsigned idx = 0; idx < _n_delays; idx++ ) {
        _readers[idx] = _buffer_end - _n_samples_delay[idx];
    }
    
    _writer = _buffer ;
    _n_ready_write = _buffer_size;
}

void laproque::MultiDelay::clear_delays()
{
    _n_samples_delay.clear();
    _readers.clear();
    _weights.clear();
    _n_delays = 0;
}

void laproque::MultiDelay::replace_buffer( float* sample_data, unsigned long n_frames )
{
    // Check if n_frames exceeds buffer size.
    if ( ptrdiff_t(n_frames) <= _buffer_size) {
        // Set back writer pointer
        _writer -= n_frames;
        if ( _writer < _buffer ) {
            _writer += _buffer_size;
        }
        
        // Set all back reader pointer.
        for ( unsigned dly = 0; dly < _n_delays; dly++ ) {
            _readers[dly] -= n_frames;
            if ( _readers[dly] < _buffer ) {
                _readers[dly] += _buffer_size;
            }
        }
        
        ptrdiff_t part1 = _buffer_end - _writer;
        if ( part1 > ptrdiff_t(n_frames) ) {
            part1 = n_frames;
        }
        memcpy( _writer, sample_data, (unsigned long)part1 * sizeof(float) );
        memcpy( _buffer, sample_data+part1, (unsigned long)(n_frames-part1) * sizeof(float) );
    }
}

void laproque::MultiDelay::print_buffer( unsigned n_frames )
{
    long part1 = _buffer_end - _writer;
    if ( part1 > ptrdiff_t(n_frames) ) {
        part1 = n_frames;
    }
    for ( long idx = 0; idx < part1; idx++) {
        printf("%f\n", _writer[idx]);
    }
    for ( long idx = 0; idx < ptrdiff_t(n_frames-part1); idx++) {
        printf("%f\n", _buffer[idx]);
    }
}

//void laproque::MultiDelay::write_buffer( SNDFILE *audio_file )
//{
//    sf_write_float(audio_file, _buffer, _buffer_size);
//}
