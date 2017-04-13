//
//  FilteredDelay.hpp
//  laproque - https://github.com/Buerner/laproque
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#ifndef FilteredDelay_hpp
#define FilteredDelay_hpp

#include "FadingDelay.hpp"
#include "Filterbank.hpp"
#include <vector>

namespace laproque {

/**
 @class FilteredDelay
 @brief Delay unit with Filterbank and weighted sum of bands.
 */
class FilteredDelay : public Delay
{
public:
    /**
     @param n_delay Number of frames the input signal gets delayed.
     @param max_delay Maximum number of possible frames delay = buffer size.
     @param co_freqs Crossover frequencies in the filterbank.
     @param sample_rate sample rate the internal filterbank workes with.
     */
    FilteredDelay(
                  unsigned n_delay=1
                  , unsigned max_delay=16384
                  , std::vector<float> co_freqs = std::vector<float>{200, 1e3}
                  , unsigned sample_rate = 44100
                  );
    
    ~FilteredDelay();
    
    /**
     @brief Operator for sample by sample processing.
     @param input Input sample value.
     @returns Output sample value.
     */
    float operator()( float input );
    
    /**
     @brief Function for block wise processing.
     @param input 1D Buffer with input samples containing n_frames samples.
     @param output 1D Buffer with output samples. Must be able to store n_frames Samples.
     @param n_frames Number of frames to be processed.
     */
    void process( float* input, float* output, unsigned long n_frames );
    
    /**
     @brief Change the crossover frequencies of the filterbank.
     @param new_co_freqs Vector containing crossover frequency values as floats. Must contain the correct amount of values.
     */
    void set_co_freqs( std::vector<float> new_co_freqs );
    
    /**
     @brief Change weighting factors of the frequency bands.
     @param new_band_weights Vector containing weighting factors as floats. Must contain the correct amount of values.
     */
    void set_all_weights( std::vector<float> new_band_weights );
    
    /**
     @brief Set only one frequency band weighting factor.
     @param weight Weighting factor value.
     @param band_idx Index of frequency band corresponding to the weighting factor to be set.
     */
    void set_band_weight( float weight, unsigned band_idx );
    
    /*@brief Sets weighting factors of all frequency bands to 1. */
    void reset_weights();
    
    /** @returns Number of frequency bands in Filterbank. */
    unsigned get_n_bands();
    
    
private:
    unsigned _n_bands;
    std::vector< float > _band_weights;
    
    Filterbank _filterbank;
    float** _band_buffer;
    float* _delay_buffer;
    
    unsigned long _n_ready;
    unsigned long _n_remaining;
    
    const unsigned _intern_buff_size = 1024;
};

} //namespace laproque

#endif /* FilteredDelay_hpp */
