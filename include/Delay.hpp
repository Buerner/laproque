//
//  Delay.hpp
//  laproque - https://github.com/Buerner/laproque
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#ifndef Delay_hpp
#define Delay_hpp

#include <stddef.h>

/**
 * @class Delay
 * @brief Simple single delay module
 */
class Delay
{
public:
    /** 
     * @param n_delay Number of samples the signal gets delayed.
     * @param max_delay Maximum samples of possible delay i.e. internal buffer size.
     */
    Delay( unsigned n_delay=0, unsigned max_delay=16384 );
    virtual ~Delay();
    
    /**
     * Operator for sample by sample processing
     * @param input Audio input sample.
     * @returns Audio output sample.
     */
    float operator() ( float input );
    
    /**
     * @brief Set a new delay value.
     * @param new_delay Desired number of samples delay. Only applied if < max_delay
     */
    virtual void set_delay( long new_delay );
    
    /**
     * @brief Function for block processing. 
     * Works with abitrary number of samples as long as there is enough input.
     */
    virtual void process( float* input, float* output, unsigned long n_samples );

    /** @returns One sample without writing to the internal buffer. */
    float get_one();

    /** @brief Write one sample to the internal buffer */
    void set_one( float input );

    /**
     * @brief Erase the internal buffer.
     * Also moves read an write pointer to starting position. Keeps the delay value.
     */
    virtual void reset();

    /** @returns Number of samples delay currently set. */
    long get_delay();
    
    /**
     * @brief Replace part of the internal buffer.
     * This function can be used if the MultiDelay is used with multiple settings
     * during one processing block. Only gets executed if n_frames < max_delay.
     * @param sample_data Array with audio samples which are copied into the internal buffer.
     * @param n_frames Number of frames which are to be replaced.
     */
    void replace_buffer( float* sample_data, unsigned n_frames );
    
protected:
    /** Pointer to the position in the buffer where the process reads. */
    float* _reader;
    
    /** Pointer to one sample after the buffer memory. */
    float* _buffer_end;
    
    /** Number of samples which can be sotred in _buffer */
    long int _buffer_size;
    
    /** Number of samples which remain to be written to the output. Used in process */
    ptrdiff_t _n_remaining;
    
    /** Number of samples of delay that is currently set */
    ptrdiff_t _n_delay;
    
    /** Audio sample storage */
    float* _buffer;
    
    /** Number of samples ready to be written to the output without further steps. */
    ptrdiff_t _n_ready;
    
    /** Pointer to the position in the buffer where the process writes. */
    float* _writer;

};

#endif /* Delay_hpp */
