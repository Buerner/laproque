//
// Filterbank.hpp
//  laproque - https://github.com/Buerner/laproque
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#ifndef Filterbank_hpp
#define Filterbank_hpp

#include "Filter.hpp"
#include <vector>

namespace laproque {

/**
 * @class Filterbank
 *
 * @brief One channel filter bank which consists of first order butterworth filters.
 *
 * The sum of the resulting bands is equal to the input signal.
 * Can be used for sample by sample processig with the operator()( float in_sample )
 * or for block processing with the process( float* input, float** output, unsigned n_samples ).
 */
class Filterbank
{
public:
    /**
     * @param sample_rate Audio sample frequency the Filterbank operates with.
     * @param co_freqs Crossover frequencies splitting the bands.
     */
    Filterbank(
               std::vector<float> co_freqs = std::vector<float>{1000.f}
               , unsigned sample_rate = 44100
               
               );
    
    ~Filterbank();
    
    /**
     * @brief Operator for processing one sample.
     * @param in_sample Input value of audio signal.
     * @param bands Pointer to buffer into which all bands are written. Must hold _n_bands values.
     */
    void operator()( float in_sample, float* bands );
    
    /**
     * @brief Function for block processing.
     * @param input Pointer to buffer with input signal.
     * @param output Pointer to buffer with band signals. 
     * Must hold: First dimension _n_bands, second dimension n_samples.
     */
    virtual void process( float* input, float** output, unsigned long n_frames );
    
    /**
     * @brief Set new crossover frequency values.
     * @param co_freqs New crossover frequency values.
     * Must have the same length as _crossover_freqs i.e. number of bands is fixed to length on initialization.
     */
    void set_co_freqs( std::vector<float> co_freqs );
    
    /**
     * @brief Change operating sampling frequency.
     * @param sample_rate New desired value for sampling frequency.
     */
    void set_sample_rate( unsigned sample_rate );
    
    /**
     * @brief reset delay lines in the filters.
     */
    void reset();
    
    
    /**
     * @brief Reset the Filterbank to the state before processing the last block.
     *
     * Can be used i.e. for processing one block with different filterbank settings.
     * Therefore the delaylines of the filters need to be reset to the previous state.
     *
     */
    void reverse();
    
    /**
     *
     */
    void renew( std::vector<float> new_co_freqs );
    
    /**
     * @returns Number of bands in Filterbank 
     */
    unsigned get_n_bands();
    
private:
    /** Vector holding all the required Filter objects */
    std::vector<Filter> _filters;
    /** Stores all crossover frequency values */
    std::vector<float> _crossover_freqs;
    /** Stores the sampling freuqency the flilterbank is curently operating with. */
    unsigned _sample_rate;
    
    /** Number of frequency bands. */
    unsigned _n_bands;
    /** Number of Filters the Filterbanks needs to work. */
    unsigned _n_filters;
    
    /** Internal sample buffer */
    float* _buffer1;
    /** Internal sample buffer */
    float* _buffer2;
    
    /** Pointer to lowest band used in block processing  */
    float* _low_band;
    
    /** Creates the Filter objects with  */
    void _update_filters();
    
    /** Size of internal buffer */
    const unsigned _intern_buff_size = 1024;
    
    unsigned long _n_remaining;
    unsigned long _n_ready;
};

} // namespace laproque

#endif /* defined(Filterbank_hpp) */
    
