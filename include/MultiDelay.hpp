//
//  MultiDelay.hpp
//  laproque - https://github.com/Buerner/laproque
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#ifndef MultiDelay_hpp
#define MultiDelay_hpp

#include <vector>
#include <sndfile.h>
#include <stddef.h>

namespace laproque {

/**
 * @class MultiDelay
 * @brief One channel delay module which supports multiple delay values at once.
 * Also, every delay value has its own gain factor.
 */
class MultiDelay
{
public:
    /** @param max_delay Maximum samples of possible delay i.e. internal buffer size. */
    MultiDelay( unsigned max_delay=16384 );
    virtual ~MultiDelay();
    
    /**
     * brief Operator for sample by sample processing
     * @param input Audio input sample.
     * @returns Audio output sample.
     */
    float operator() ( float input );
    
    /**
     * @brief Add a delay value to existing vlaues in this instance.
     * Value is only added to the list if it does not exceed max_delay.
     * @param delay Desired number of samples delay.
     * @param weight Gain factor for the delay to be set.
     */
    void add_delay( long delay, float weight );
    
    /**
     * @brief Replace currently set delay values.
     * Must contain same number of values as currently set. Only values smaller
     * than max_delay are applied. 
     * @param new_delays Array with new desired delay values.
     */
    void set_delays( long* new_delays );
    
    /**
     * @brief Replace gain factors of all delays currently set.
     * @param new_weights Array with gain factors.
     */
    void set_weights( float* new_weights );
    
    /**
     * @brief Function for block processing.
     * @param input Buffer with input audio samples.
     * @param output Buffer with output audio samples.
     * @param n_frames Number of audio frames to be processed.
     */
    virtual void process( float* input, float* output, unsigned long n_frames );
    
    
    void get_one( float* output );
    
    void set_one( float input );
    
    /** 
     * @brief Erase internal buffer.
     * Also resets the writer and all reader pointers. Keeps the delay values.
     */
    void reset();
    
    /**
     * @brief Remove all delay values in this instance.
     * This clears all the delay values while retaining the buffered audio.
     */
    void clear_delays();
    
    /** 
     * @brief Replace part of the internal buffer.
     * This function can be used if the MultiDelay is used with multiple settings
     * during one processing block. Only gets executed if n_frames < max_delay.
     * @param sample_data Array with audio samples which are copied into the internal buffer.
     * @param n_frames Number of frames which are to be replaced.
     */
    void replace_buffer( float* sample_data, unsigned long n_frames );
    
    /** @brief Print values of internal buffer to console. */
    void print_buffer( unsigned n_frames );
    
    /** @returns Vector with delay values currently set. */
    std::vector< float* > get_delays();
    
protected:
    /** Array which stores the input audio samples. */
    float* _buffer;
    /** Pointer sample after last _buffer value. */
    float* _buffer_end;
    /** Pointer to place in _buffer where input is currently to. */
    float* _writer;
    /** Vector with pointers to the value in _buffer which is read next. */
    std::vector< float* > _readers;
    
    /** Vector with number of samples delay */
    std::vector< long > _n_samples_delay;
    /** Vector storing the gain factors associated to the delays. */
    std::vector< float > _weights;
    /** Number of delays currently set */
    unsigned _n_delays = 0;
    
    long _n_remaining, _n_ready_read, _n_ready_write;
    /** Number of storeable audio sampls. */
    ptrdiff_t _buffer_size;
    
};

} // namespace laproque

#endif /* MultiDelay_hpp */
