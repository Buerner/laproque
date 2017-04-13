//
//  CrossFader.cpp
//  laproque - https://github.com/Buerner/laproque
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#include "CrossFader.hpp"
#include <math.h>

laproque::CrossFader::CrossFader( unsigned fade_length )
{
    _sin_fade.resize( fade_length );
    _fade_length = fade_length - 1;
    _compute_fade_function();
}

void laproque::CrossFader::_compute_fade_function()
{
    float phase_increment = M_PI * 0.5f / _fade_length;
    float phase = 0.f;
    
    for ( unsigned idx = 0; idx <= _fade_length; idx++ ) {
        _sin_fade[idx] = cosf(phase);
        _sin_fade[idx] *= _sin_fade[idx];
        phase += phase_increment;
    }
}

void laproque::CrossFader::process( float *fadeout, float *fadein, float *output )
{
    for ( unsigned idx = 0; idx <= _fade_length; idx++ ) {
        output[idx] = fadeout[idx]*_sin_fade[idx] + fadein[idx]*_sin_fade[_fade_length-idx];
    }
}

unsigned laproque::CrossFader::get_fade_length()
{
    return _fade_length + 1;
}

void laproque::CrossFader::set_fade_length( unsigned int fade_length )
{
    _sin_fade.resize( fade_length );
    _fade_length = fade_length - 1;
    _compute_fade_function();
}
