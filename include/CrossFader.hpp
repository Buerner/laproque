//
//  CrossFader.hpp
//  laproque - https://github.com/Buerner/laproque
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#ifndef CrossFader_hpp
#define CrossFader_hpp

#include <stdio.h>
#include <vector>

/**
 @class Sine square fading of two input signals.
 */
class CrossFader
{
public:
    /** @param fade_length Numbe of values to be faded.  */
    CrossFader( unsigned fade_length );
    
    /** 
     @brief Applies the fading function and writes results.
     @param fadeout Pointer to the array conatining the values to be faded out.
     @param fadein Pointer to the array conatining the values to be faded in.
     @param output Pointer to the array the results are written to.
     */
    void process( float* fadeout, float* fadein, float* output );
    
    /** @returns Length of currently set fading function. */
    unsigned get_fade_length();
    
    /** @brief Change the length of the fading function. */
    void set_fade_length( unsigned fade_length );
    
private:
    unsigned _fade_length;
    
    void _compute_fade_function();
    std::vector<float> _sin_fade;
};

#endif /* CrossFader_hpp */
