//
//  FadingDelay.hpp
//  laproque - https://github.com/Buerner/laproque
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//


#ifndef FadingDelay_hpp
#define FadingDelay_hpp

#include <stdio.h>
#include <atomic>

#include "Delay.hpp"

class FadingDelay : public Delay
{
public:
    FadingDelay( unsigned long fade_length, unsigned delay=0, unsigned max_delay = 65536 );
    ~FadingDelay();
    
    void process( float *input, float *output, unsigned long n_samples );
    
    void set_delay( long new_delay );
    void set_fade_out( bool do_fade );
    
    void reset();
    
private:
    unsigned long _fade_length;
    std::atomic<bool> _has_changed{false};
    std::atomic<bool> _fade_out{false};
    
    void _setup_fades();
    
    unsigned long _to_fade;
    
    float* _old_reader;
    
    float* _fadein_buf;
    float* _fadeout_buf;
};

#endif /* FadingDelay_hpp */
