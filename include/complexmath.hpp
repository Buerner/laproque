//
//  complexmath.hpp
//  laproque - https://github.com/Buerner/laproque
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#ifndef complexmath_hpp
#define complexmath_hpp

#include <fftw3.h>

extern void complex_multiply(fftwf_complex* factor1
                             , fftwf_complex* factor2
                             , fftwf_complex* result
                             , unsigned N = 1
                             );

extern void complex_bilin_interp(fftwf_complex* base
                                 , fftwf_complex* x_neighbour
                                 , fftwf_complex* y_neigbour
                                 , fftwf_complex* diag_neighbour
                                 , fftwf_complex* result
                                 , float x_fract
                                 , float y_fract
                                 , unsigned N = 1
                                 );

extern void complex_interp(  fftwf_complex* base
                           , fftwf_complex* neighbour
                           , fftwf_complex* result
                           , float base_fract
                           , unsigned N = 1
                           );

extern void freq_domain_interp(  fftwf_complex* base
                               , fftwf_complex* neighbour
                               , fftwf_complex* result
                               , float base_fract
                               , unsigned N = 1
                               );

extern float complex_abs( fftwf_complex value );
extern float complex_angle( fftwf_complex value );

extern void unwrap_phase( fftwf_complex* spectrum, unsigned n_bins );

#endif /* complexmath_hpp */
