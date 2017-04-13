//
//  complexmath.cpp
//  laproque - https://github.com/Buerner/laproque
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#include "complexmath.hpp"
#include <math.h>

void complex_multiply(fftwf_complex* factor1, fftwf_complex* factor2, fftwf_complex* result, unsigned N)
{
    for ( unsigned idx = 0; idx < N; idx++ ) {
        result[idx][0] = factor1[idx][0]*factor2[idx][0] - factor1[idx][1]*factor2[idx][1];
        result[idx][1] = factor1[idx][0]*factor2[idx][1] + factor1[idx][1]*factor2[idx][0];
    }
}

void complex_interp(  fftwf_complex* base
                    , fftwf_complex* neighbour
                    , fftwf_complex* result
                    , float base_fract
                    , unsigned N
                    )
{
    float inv_fract = 1.f - base_fract;
    for ( unsigned idx = 0; idx < N; idx++ ) {
        result[idx][0] = base[idx][0]*base_fract + neighbour[idx][0]*inv_fract;
        result[idx][1] = base[idx][1]*base_fract + neighbour[idx][1]*inv_fract;
    }
}

void complex_bilin_interp(fftwf_complex* base
                          , fftwf_complex* x_neighbour
                          , fftwf_complex* y_neigbour
                          , fftwf_complex* diag_neighbour
                          , fftwf_complex* result
                          , float x_fract
                          , float y_fract
                          , unsigned N
                          )
{
    float next_x_fract = 1.f - x_fract;
    float next_y_fract = 1.f - y_fract;
    
    for ( unsigned idx = N; idx--; ) {
        result[idx][0] =
            ( base[idx][0] * next_x_fract + x_neighbour[idx][0] * x_fract ) * next_y_fract
          + ( y_neigbour[idx][0] * next_x_fract + diag_neighbour[idx][0] * x_fract) * y_fract;

        result[idx][1] =
            ( base[idx][1] * next_x_fract + x_neighbour[idx][1] * x_fract ) * next_y_fract
          + ( y_neigbour[idx][1] * next_x_fract + diag_neighbour[idx][1] * x_fract) * y_fract;
    }
}

float complex_abs( fftwf_complex value )
{
    return sqrtf( value[0]*value[0] + value[1]*value[1] );
}

float complex_angle( fftwf_complex value )
{
    return atan2f( value[1], value[0] );
}

void freq_domain_interp(  fftwf_complex* base
                        , fftwf_complex* neighbour
                        , fftwf_complex* result
                        , float base_fract
                        , unsigned N
                        )
{
    float inv_fract = 1.f - base_fract;
    float abs;
    float angle;
    for ( unsigned idx = 0; idx < N; idx++ ) {
        abs = complex_abs(base[idx]) * base_fract + complex_abs(neighbour[idx]) * inv_fract;
        angle = complex_angle(base[idx]) * base_fract + complex_angle(neighbour[idx]) * inv_fract;
        result[idx][0] = abs * cosf(angle);
        result[idx][1] = abs * sinf(angle);
        //printf("%f + i%f\t%f + i%f\t%f + i%f\n",base[idx][0], base[idx][1], neighbour[idx][0], neighbour[idx][1], result[idx][0], result[idx][1]);
    }
}

void unwrap_phase( fftwf_complex* spectrum, unsigned n_bins )
{
    float angle_diff, current_angle, last_angle;
    
    for ( unsigned idx = 1; idx < n_bins; idx++ ) {
        current_angle = complex_angle( spectrum[idx] );
        last_angle = complex_angle( spectrum[idx] );
        
        angle_diff = fabsf( last_angle - current_angle );
        if ( angle_diff > M_PI )
        {
            current_angle += floorf(angle_diff / (2.f*M_PI)) + 1;
        }
        
    }
    
}






