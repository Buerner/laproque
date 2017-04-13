//
//  TimeVarConvolver.hpp
//  laproque - https://github.com/Buerner/laproque
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#ifndef TimeVarConvolver_hpp
#define TimeVarConvolver_hpp

#include "Convolver.hpp"

namespace laproque {

/**
 * @class TimeVarConvolver
 * @brief Partitioned Convolver with exchangable partitions.
 *
 * Provides the ability to change the partitions of the partitioned convolution
 * during block processing. Results of previous and new partitions are crossfaded 
 * in the timedomain.
 */
class TimeVarConvolver : public Convolver
{
public:
    /**
     * @param imp_resp One channel impulse response.
     * @param n_samples Number of samples in imp_resp.
     * @param block_size Number of samples in a block during block processing.
     */
    TimeVarConvolver(float* imp_resp, unsigned n_samples, unsigned block_size);
    TimeVarConvolver( TimeVarConvolver& tvconv );
    ~TimeVarConvolver();
    
    /**
     * @param in_buffer Pointer to buffer with input samples.
     * @param out_buffer Pointer to buffer with output samples.
     */
    void process( float* in_buffer, float* out_buffer );
    
    /** Function which replaces the partitiones used in the convolution process. */
    void set_partitions( fftwf_complex* new_partitions );
    
    
private:
    /** Additional buffer needed during exchange of partitions */
    fftwf_complex* _other_freq_resp;
    /** Pointer backup storage */
    fftwf_complex* _pointer_backup;
    
    /** Atomic variable used to indicate that the partitions have been exchanged.  */
    std::atomic<bool> _has_changed{false};
    /** cos^2 fade in ramp. */
    float* _up_ramp;
    /** cos^2 fade out ramp. */
    float* _down_ramp;
    /** Additional buffer for faded sample values. */
    float* _fade_buffer;
    
    /** Function which calculates the cos^2 crossfading curves. */
    void _setup_ramps();
    
};

} // namespace laproque

#endif /* TimeVarConvolver_hpp */
