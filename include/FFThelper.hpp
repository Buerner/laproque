//
//  FFThelper.hpp
//  laproque - https://github.com/Buerner/laproque
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#ifndef FFThelper_hpp
#define FFThelper_hpp

#include <stdio.h>
#include <fftw3.h>

/**
 * @brief Takes care of getting a FFTW3f 1D real to complex or complex to real FFT ready.
 *
 * This class hides everything that needs to be done to compute a one-dimensional real to complex or complex to real fft with the fftw3 library. It takes care of all allocations and sets up a fft plan. Just setup an instance with the desired fft length and call process with your input and output buffer. Note that the (symetrical and thus redundant) second half of the result is not written to the output buffer. Only size/2 + 1 complex frequency bins are returned.
 */
class FFThelper
{
public:
    /**
     * @param size FFT size / resolution.
     */
    FFThelper( unsigned size );
    ~FFThelper();
    
    /**
     * @brief Processing function for real 2 complex FFT
     * @param input Input buffer, must be same size as in initialization of the instance.
     * @param output Where the complex frequency domain is written to.
     * @param normalize If set, the iFFT will result in the same as the FFT.
     */
    void real2complex( float* input, fftwf_complex* output, bool normalize = true );
    
    /**
     * @brief Processing function for complex 2 real iFFT
     * @param input Input buffer, must be same size as in initialization of the instance.
     * @param output Where real time domain data is written to.
     */
    void complex2real( fftwf_complex* input, float* output );
    
    /** @returns Number of bins in resulting spectrum */
    unsigned get_spetrum_size();
    
    /** @returns Normalization factor applied in transformation. */
    float get_norm_fact();
    
    /** @brief Set the normalization factor applied in transformation. */
    void set_norm_fact( float norm_fact );
    
    /** @brief Set the normalization factor to default state. */
    void reset_norm_fact();
    
    
private:
    const unsigned _fft_size;
    const unsigned _spectrum_size;
    
    float _norm_fact;
    
    float* _time_domain;
    
    fftwf_complex* _freq_domain;
    
    fftwf_plan _fft_plan;
    fftwf_plan _ifft_plan;
};


#endif /* FFThelper_hpp */
