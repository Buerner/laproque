//
//  SincLP.hpp
//  laproque - https://github.com/Buerner/laproque
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#ifndef SincLP_hpp
#define SincLP_hpp

#include <math.h>
#include "Convolver.hpp"

/**
 * @class SincLP
 * @brief Steep lowpass using convolution with a sinc impulse.
 *
 * This class implements a steep lowpass filter. The delay caused by the symetrical
 * sinc impulse is compensated by shifting the output back by half the filter length.
 */
class SincLP
{
public:
    /**
     @param cutoff_freq -3 dB Frequency
     @param sample_rate The sample frequency to be used during processing.
     @param block_size The block processing to be used.
     @param length Filter length.
     */
    SincLP(float cutoff_freq, float sample_rate, unsigned block_size, unsigned length);
    ~SincLP();
    /** 
     @brief Processes the samples in the input buffer and writes the result to the output. The number of processed samples always equals the initually set block size.
     @param input Pointer to array with input samples.
     @param output Pointer to array where the output samples should be written to.
     
     */
    void process(float* input, float* output);
    
    /** Resets the input buffer in the the fast convolution object.  */
    void reset();
    
    /*
     Writes a blackman window of desired length into the buffer.
     */
    static void blackman( float* buffer, unsigned length )
    {
        for (unsigned idx = 0; idx < length ; idx++) {
            buffer[idx] = 0.42f - 0.5f * cosf( 2*M_PI*idx / (length-1) ) + 0.08f * cosf(4*M_PI*idx / (length-1) );
        }
    }
    
private:
    float _co_freq;
    float _sample_rate;
    unsigned _block_size;
    float _length;
    unsigned _dly_comp;
    
    float* _in_buffer;
    float* _out_buffer;
    Convolver* _convolver;
};

#endif /* SincLP_hpp */
