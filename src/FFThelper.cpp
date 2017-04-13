//
//  FFThelper.cpp
//  laproque - https://github.com/Buerner/laproque
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#include "FFThelper.hpp"
#include <string>
#include <math.h>
#include <cstring>

laproque::FFThelper::FFThelper( unsigned size ) :
_fft_size(size + (size % 2)),
_spectrum_size(size/2 + 1)
{
    
    _norm_fact = .5f / float(_spectrum_size-1);
    
    _time_domain = fftwf_alloc_real( _fft_size );
    _freq_domain = fftwf_alloc_complex( _spectrum_size );
    
    _fft_plan = fftwf_plan_dft_r2c_1d( int(_fft_size), _time_domain, _freq_domain, 0 );
    _ifft_plan = fftwf_plan_dft_c2r_1d( int(_fft_size), _freq_domain, _time_domain, 0 );
    
}

laproque::FFThelper::~FFThelper()
{
    // Clean up.
    fftwf_free( _time_domain );
    fftwf_free( _freq_domain );
    
    fftwf_destroy_plan( _fft_plan );
    fftwf_destroy_plan( _ifft_plan );
}

void laproque::FFThelper::real2complex( float *input, fftwf_complex *output, bool normalize )
{
    memcpy( _time_domain, input, _fft_size * sizeof(float) );
    fftwf_execute( _fft_plan );
    
    if ( normalize ) {
        for ( unsigned idx = 0; idx < _spectrum_size; idx++ ) {
            _freq_domain[idx][0] *= _norm_fact;
            _freq_domain[idx][1] *= _norm_fact;
        }
    }
    
    memcpy( output, _freq_domain, _spectrum_size * sizeof(fftwf_complex) );
}

void laproque::FFThelper::complex2real( fftwf_complex *input, float *output )
{
    memcpy( _freq_domain, input, _spectrum_size * sizeof(fftwf_complex) );
    fftwf_execute( _ifft_plan );
    memcpy( output, _time_domain, _fft_size * sizeof(float) );
}

unsigned laproque::FFThelper::get_spetrum_size()
{
    return _spectrum_size;
}

float laproque::FFThelper::get_norm_fact()
{
    return _norm_fact;
}

void laproque::FFThelper::set_norm_fact( float norm_fact )
{
    _norm_fact = norm_fact;
}

void laproque::FFThelper::reset_norm_fact()
{
    _norm_fact = .5f / float(_spectrum_size-1);
}


