//
//  Convolver.hpp
//  laproque - https://github.com/Buerner/laproque
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#ifndef Convolver_hpp
#define Convolver_hpp

#include <stdio.h>
#include <fftw3.h>
#include <atomic>

#include "misc/include/complexmath.hpp"
#include "FFThelper.hpp"


// TODO: The _freq_resp_parts array must be thread safe because
// replacing could interfere with the audio porcessing. Maybe write a
// threadsafe array class and overload braces operator.
/**
 * @brief Partitioned fast convolution class.
 *
 * Class which computes the convolution of and input buffer with an impulse response passed on initialization of instance. It uses the partitioned convolution approach to allow the use in blockwise audio processing. 
 * On initialization just pass your impulse response, its size and the size of the partitiones.
 */
class Convolver
{
public:
    /**
     * @brief Construtor for convolver instance with desired impulse response.
     * @param imp_resp Pointer to the impulse response you want to use.
     * @param n_samples Length of the impulse response in samples.
     * @param block_size Size of the processing blocks i.e. partitions.
     */
    Convolver(float* imp_resp, unsigned long n_samples, unsigned block_size);
    
    /**
     * Copy constructor.
     */
    Convolver( Convolver& conv );
    
    virtual ~Convolver();
    
    /**
     * @brief Function which computes the convolution result.
     
     * Note that the size of input and outputs equals the block_size chosen on initialization.
     * @param in_buffer Input data with size of block_size.
     * @param out_buffer Output data with size of block_size.
     */
    virtual void process( float* in_buffer, float* out_buffer );
    
    /**
     * @brief Replace frequency response.
     *
     * This functions overwrites the currently frequency response which is currently set. It expects correctly patitioned blocks in the frequency domain.  
     */
    void set_freq_response( fftwf_complex* new_response );
    
    /**
     * @brief Set time domain input buffer to 0.
     *
     * As the class processes multiple blocks, the input needs to be stored to comply with the partitiond convolution pricipal. When you want to use a Convolver instance for different input signals call this function between processing.
     */
    void reset_input_buffer();
    
    /**
     * @brief Returns FFT resolution.
     */
    unsigned get_fft_size();
    
    /**
     * @brief Returns processing block size.
     */
    unsigned get_block_size();
    
    /**
     * @brief Returns resolution of one partition (fft_size/2 + 1).
     */
    unsigned get_spectrum_size();
    
    /**
     * @brief Returns size of all frequency domain partitions combined.
     */
    unsigned get_spectra_size();
    
    /**
     * @brief Returns number of partitions.
     */
    unsigned get_n_parts();
    
protected:
    /** Length of discrete fourier transform */
    unsigned _fft_size;
    /** Number of frames in on block during block processing. */
    unsigned _block_size;
    /** Number fo complex values in one partition pectrum */
    unsigned _spectrum_size;
    /** Number of complex values in partitions combined */
    unsigned _spectra_size;
    /** Number of partitions. */
    unsigned _n_parts;
    
    float* _input;
    float* _result;

    fftwf_complex* _input_spectra;
    fftwf_complex* _freq_resp_parts;
    fftwf_complex* _multiply_buffer;
    fftwf_complex* _output_spectr;
    
    FFThelper _fft;
    
    /** Takes care of all memory reservations needed for convolution process. */
    void _make_allocations();
    
    /**
     * @brief Actual fast convolution process
     .
     * Heart of the convolution.
     * Implemented as seperate function to enable derived classes to do additional processing.
     */
    void _fast_conv();
    
    /**
     * Prepares the frequency response partitions i.e. computes FFT of blocks of the impulse response.
     */
    void _compute_freq_resp( float* imp_resp );
    
};

#endif /* Convolver_hpp */

