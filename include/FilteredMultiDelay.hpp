//
//  FilteredMultiDelay.hpp
//  laproque - https://github.com/Buerner/laproque
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#ifndef FilteredMultiDelay_hpp
#define FilteredMultiDelay_hpp

#include "MultiDelay.hpp"
#include "Filterbank.hpp"

const std::vector< float > freqs{ 300, 2000 };

/**
 * @class FilteredMultiDelay
 * @brief Processing unit which works like MultiDelay with a Filterbank and weighted sum of bands before output.
 */
class FilteredMultiDelay : public MultiDelay
{
public:
    FilteredMultiDelay( unsigned n_bands = 3, unsigned sample_rate = 44100, unsigned block_size = 1024, unsigned max_delay = 45643 );
    virtual ~FilteredMultiDelay();
    
    /**
     * @brief Function for block wise processing.
     * @param input 1D Buffer with input samples containing n_frames samples.
     * @param output 1D Buffer with output samples. Must be able to store n_frames Samples.
     * @param n_frames Number of frames to be processed.
     */
    void process( float* input, float* output, unsigned long n_frames );
    
    /**
     * @brief Change the crossover frequencies of the filterbank.
     * @param new_co_freqs Vector containing crossover frequency values as floats. Must contain the correct amount of values.
     */
    void set_co_freqs( std::vector<float> new_co_freqs );
    
    /**
     * @brief Change single weighting factor of the frequency band specified.
     * @param new_band_weight Value to be set.
     * @param band_idx Index of frequency band the new weight should be applied to.
     */
    void set_band_weight( float new_band_weight, unsigned band_idx );
    
    /**
     * @brief Change weighting factors of the frequency bands.
     * @param new_band_weights Vector containing weighting factors as floats. Must contain the correct amount of values.
     */
    void set_band_weights( std::vector<float> new_band_weights );
    
    /**
     * @brief Change the sample rate the Unit is working with.
     * @param sample_rate The new sample rate value.
     */
    void set_sample_rate( unsigned sample_rate );
    
    /** @returns Number of frequency bands in Filterbank. */

    unsigned get_n_bands();
    
    /** 
     * @brief Change n_frames value in signal buffer.
     * 
     * This also reverses the filterbank in the module. Which is why this only makes sense when n_frames equals the number of frames in the last processing step.
     */
    void replace_buffer( float* sample_data, unsigned long n_frames );
    
    

private:
    Filterbank _filterbank;
    unsigned _block_size;
    unsigned _n_bands;
    
    float** _band_buffer;
    float* _internal_buffer;
    
    float* _band_weights;
};

#endif /* FilteredMultiDelay_hpp */
