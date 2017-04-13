//
//  Filter.hpp
//  laproque - https://github.com/Buerner/laproque
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#ifndef Filter_hpp
#define Filter_hpp

/**
 * This enum specifies the type of a filter in the Filter class.
 */
enum filter_type
{
    HIGH,
    LOW
};

/**
 * @class Filter
 * @brief First order Butterworth filter. High- or low-pass.
 *
 * This is a first order Butterworth filter which can be used as either low or high pass filter.
 * It offeres sample wise processing with the operator()( float in_sample )
 * or block processing with the process( float* input, float* output, unsigned n_samples ) function.
 * Processes only one channel.
 *
 */
class Filter
{
public:
    /**
     * @brief Filter constructor.
     *
     * @param type Defines if Filter is high- or low-pass filter
     * @param cutoff_freq The desired -3 dB cutoff frequency
     * @param sample_rate Audio sample frequency the filter operates with.
     */
    Filter( filter_type type = LOW,
            float cutoff_freq = 1000,
            unsigned sample_rate = 44100
           );
    
    /**
     * @brief Operator for processing one sample.
     * @param in_sample Input value of audio signal.
     * @returns Filtered output value.
     */
    float operator()( float in_sample );
    
    /**
     * @brief Function for block processing.
     * @param input Pointer to buffer with input signal.
     * @param output Pointer to buffer with output signal.
     */
    void process( float* input, float* output, unsigned long long n_frames );
    
    /**
     * @brief Use to change cutoff frequency of the Filter.
     * @param co_freq The new desired cutoff frequency-
     */
    void set_cutoff_freq( float co_freq );
    
    /**
     * @brief Set delaylines to 0.
     */
    void reset();
    
    /**
     * @brief Use to change operating sample rate
     *
     * This function changes the sample rate member variable and updates the filter coefficients accordingly.
     * @param sample_rate The new desired sample frequency.
     */
    void set_sample_rate( unsigned sample_rate );
    
    /**
     * @returns Current -3 dB cutoff frequency of the Filter instance.
     */
    float get_cutoff_freq();
    
    /**
     * @brief Reset the Filter to the state before processing the last block.
     *
     * Can be used i.e. for processing one block with different filter settings. 
     * Therefore the delaylines need to be reset to the previous state. To enable this functionality
     * theese values need to be stored internaly after processing an audio signal block.
     *
     */
    void reverse();
    
    
private:
    /** Stores if filter instance is low or high pass. */
    filter_type _f_type;
    /** -3 dB cutoff frequency. */
    float _cutoff_freq;
    /** Operating sampling frequency. */
    unsigned _sample_rate;
    /** Recursive filter coefficients. _a_coeffs[1] always equals 1. */
    float _a_coeffs[2];
    /** Non-recursive filter coefficients. */
    float _b_coeffs[2];
    /** Stores the amplitude of the last input audio sample. */
    float _in_dlyline = 0.f;
    /** Stores the amplitude of the last output audio sample. */
    float _out_dlyline = 0.f;
    /** Used to backup delay line state in order to be able to reset filter status. */
    float _in_dlyline_backup = 0.f;
    /** Used to backup delay line state in order to be able to reset filter status. */
    float _out_dlyline_backup = 0.f;
    
    /**
     * @brief Calculates the filter coefficients with the current cutoff frequency and sample frequency values.
     *
     * \f[ b_0 = \frac{1}{1+c} \f]
     * \f[ b_1 =  b_0 \f]
     * \f[ a_0 =  1 \f]
     * \f[ a_1 =  \frac{1-c}{1+c} \f]
     * with
     * \f[ c = \frac{1}{tan\left( \pi * \frac{f_c}{f_s} \right) } \f]
     */
    void _compute_coeffs();

};

#endif /* defined(__FDN__Filter__) */
