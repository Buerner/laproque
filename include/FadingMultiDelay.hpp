//
//  FadingMultiDelay.hpp
//  laproque - https://github.com/Buerner/laproque
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#ifndef FadingMultiDelay_hpp
#define FadingMultiDelay_hpp

#include <vector>
#include <array>
#include <sndfile.h>
#include <stddef.h>
#include <atomic>

namespace laproque {

/**
 * @class FadingMultiDelay
 * @brief One channel delay module which supports multiple delay values at once.
 * Also, every delay value has its own gain factor.
 */
class FadingMultiDelay
{
public:
    /** 
     @param max_delay Maximum samples of possible delay i.e. internal buffer size.
     */
    FadingMultiDelay( unsigned max_delay=16384 );
    virtual ~FadingMultiDelay();
    
    
    /**
     * @brief Add a delay value to existing vlaues in this instance.
     * Value is only added to the list if it does not exceed max_delay.
     * @param delay Desired number of samples delay.
     * @param weight Gain factor for the delay to be set.
     */
    void add_delay( unsigned long delay, float weight );
    
    /**
     * @brief Replace currently set delay values.
     * Must contain same number of values as currently set. Only values smaller
     * than max_delay are applied.
     * @param delays Array with new desired delay values.
     */
    void set_delays( unsigned long* delays, float* weights, unsigned n_values );
    
    /**
     * @brief Replace gain factors of all delays currently set.
     * @param new_weights Array with gain factors.
     */
    void set_weights( float* new_weights );
    
    /**
     * @brief Function for block processing.
     * @param input Buffer with input audio samples.
     * @param output Buffer with output audio samples.
     * @param n_frames Number of audio frames to be processed.
     */
    void process( float* input, float* output, unsigned long n_frames );
    
    /**
     * @brief Erase internal buffer.
     * Also resets the writer and all reader pointers. Keeps the delay values.
     */
    void reset();
    
    /**
     * @brief Remove all delay values in this instance.
     * This clears all the delay values while retaining the buffered audio.
     */
    void clear_delays();
    
    
    /** @brief Print values of internal buffer to console. */
    void print_buffer( unsigned n_frames );
    
    /** @returns Vector with delay values currently set. */
    std::vector< float* > get_delays();
    
    /** Number of storeable audio sampls. */
    const ptrdiff_t _buffer_size;
    
    static const float fade_in[128];
    static const float fade_out[128];
    static const unsigned long N_FADE = 128;
    
    enum FadeBehavior {
        BORN,
        ALIVE,
        CHANGE,
        DYING,
        DEAD
    };
    
    unsigned get_n_delays();
    
    static const unsigned N_DELAYS_MAX = 1000;
    
    void kill_last();
    
private:
    
    /** Array which stores the input audio samples. */
    float* _buffer;
    
    /** Pointer to sample after last _buffer value. */
    float* _buffer_end;
    
    /** Array which stores the outputs of the internal processing objects */
    float* _core_buffer;
    
    /** Pointer to place in _buffer where input is currently to. */
    float* _writer;
        
    /** Number of delays currently set */
    unsigned _n_delays = 0;
    
    unsigned _new_n_delays = 0;
    
    std::array< unsigned long, N_DELAYS_MAX > _new_delays;
    std::array< float, N_DELAYS_MAX > _new_weights;
    
    void _update();
    
    /** Shortest delay value */
    unsigned long _min_delay;
    
    unsigned long _n_remaining, _n_ready;
    
    void _update_min_delay();
    
    std::atomic<bool> _has_changed{false};
    
    class _DelayCore
    {
    public:
        
        _DelayCore( float* buf, float* buf_end, unsigned long n_delay, float** writer );
        _DelayCore();
        _DelayCore( const _DelayCore &obj);
        
        _DelayCore & operator= ( const _DelayCore & );
        
        void process( float* output, unsigned long n_samples );
        
        void set_delay( unsigned long delay, float weight );
        void set_status( FadeBehavior status );
        
        unsigned long get_delay();
        FadeBehavior get_status();
        
    private:
        float* _buf;
        float* _buf_end;
        long _buf_size;
        
        float** _wtr;
        float* _rdr;
        float* _old_rdr;
        
        float _wgt;
        float _old_wgt;
        
        unsigned long _n_dly;
        unsigned long _to_fade = 0;
        FadeBehavior _status = BORN;
        
        unsigned long processed = 0;
        
    };
    
    std::array<_DelayCore, N_DELAYS_MAX> _delays;
    
};

} // namespace laproque

#endif /* FadingMultiDelay_hpp */
