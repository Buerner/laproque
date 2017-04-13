//
//  FadingMultiDelay.cpp
//  laproque - https://github.com/Buerner/laproque
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#include <algorithm>
#include <cstring>
#include <math.h>

#include "FadingMultiDelay.hpp"

const unsigned laproque::FadingMultiDelay::N_DELAYS_MAX;

laproque::FadingMultiDelay::FadingMultiDelay( unsigned max_delay ) :
_buffer_size( max_delay )
{
    _buffer = new float[max_delay];
    _core_buffer = new float[max_delay];
    _buffer_end = _buffer + _buffer_size + 1;
    _min_delay = max_delay;
    _writer = _buffer;
    
    for ( unsigned dly = 0; dly < N_DELAYS_MAX; ++dly ) {
        _delays[dly] = _DelayCore( _buffer, _buffer_end, 0, &_writer );
        _delays[dly].set_status( DEAD );
    }
    
    reset();
}

laproque::FadingMultiDelay::~FadingMultiDelay()
{
    delete [] _buffer;
    delete [] _core_buffer;
}

void laproque::FadingMultiDelay::process( float *input, float *output, unsigned long n_frames )
{
    if ( _has_changed.load() ) {
        _update();
    }
    
    unsigned idx;
    
    // Set output to 0
    for ( idx = 0; idx < n_frames; idx++ ) {
        output[idx] = 0.f;
    }
    
    _n_remaining = n_frames;
    FadeBehavior status;
    
    //printf("Processing %u Delays\n", _n_delays);
    while ( _n_remaining )
    {
        _n_ready = std::min( _min_delay, _n_remaining );
        _n_ready = std::min( _n_ready, (unsigned long)(_buffer_end - _writer) );
        
       //printf("- Main Process needs: %lu, min %lu -\n", _n_ready, _min_delay);

        for ( unsigned dly = 0; dly < _n_delays; dly++) {
            //printf("-> Processing Delay %u\n", dly);
            status = _delays[dly].get_status();
            if ( status == DEAD ) {
                continue;
            }
            _delays[dly].process( _core_buffer, _n_ready );
            
            for ( idx = 0; idx < _n_ready; idx++ ) {
                output[idx] += _core_buffer[idx];
            }
        }

        memcpy( _writer, input, _n_ready*sizeof(float) );
        
        input += _n_ready;
        output += _n_ready;
        
        _writer += _n_ready;
        if ( _writer == _buffer_end) _writer = _buffer;
        
        _n_remaining -= _n_ready;
        
    }
    
    output -= n_frames;
    _n_delays = _new_n_delays;
    _update_min_delay();
    
    _has_changed.store( false );
}

unsigned laproque::FadingMultiDelay::get_n_delays()
{
    return _n_delays;
}

void laproque::FadingMultiDelay::kill_last()
{
    if (_n_delays) {
        _delays[_n_delays-1].set_status( DYING );
    }
}

void laproque::FadingMultiDelay::add_delay( unsigned long n_samples_delay, float weight )
{
    // Check if delay value works with buffer size
    if ( n_samples_delay < _buffer_size && n_samples_delay > 0 )
    {
        _delays[_n_delays].set_delay( n_samples_delay, weight );

        if ( n_samples_delay < _min_delay ) _min_delay = n_samples_delay;
        
        _n_delays++;
    }
}

void laproque::FadingMultiDelay::_update_min_delay()
{
    _min_delay = (unsigned long)_buffer_size;
    unsigned long delay;
    for ( unsigned dly = 0; dly < _n_delays; dly++ ) {
        if ( _delays[dly].get_status() == DEAD) continue;
        delay = _delays[dly].get_delay();
        if ( delay < _min_delay ) _min_delay = delay;
    }
}

void laproque::FadingMultiDelay::set_delays( unsigned long* delays, float* weights, unsigned n_values )
{
    if ( !_has_changed.load() )
    {
        for ( unsigned dly = 0; dly < std::min(n_values, N_DELAYS_MAX); dly++) {
            // Currently 0 delays are not possible
            _new_delays[dly] = std::max( delays[dly], 1lu );
            _new_weights[dly] = weights[dly];
        }
        _new_n_delays = n_values;
        _has_changed.store(true);
    }
}

void laproque::FadingMultiDelay::_update( )
{
    unsigned dly;
    
    // Update
    for ( dly = 0; dly < std::max(_new_n_delays, _n_delays); dly++ )
    {
        // Check if delay value works with buffer size
        if ( _new_delays[dly] <= (unsigned long)_buffer_size )
        {
            _delays[dly].set_delay( _new_delays[dly], _new_weights[dly] );
            if ( _new_delays[dly] < _min_delay ) _min_delay = _new_delays[dly];
        }
    }
    
    // Kill missing in arguments.
    if ( _new_n_delays < _n_delays ) {
        for ( dly = _new_n_delays; dly < _n_delays; dly++ ) {
            if ( _delays[dly].get_status() != DEAD ) {
                //printf("Killing %u\n", dly);
                _delays[dly].set_status( DYING );
            }
        }
    }
    
    // Take maximum here, because dying delays have yet to be faded out.
    _n_delays = std::max( _new_n_delays, _n_delays );
    
    // Note: Dont update minumum here because old delays could be shorter.
}

void laproque::FadingMultiDelay::set_weights( float* new_weights )
{
    for ( unsigned idx = 0; idx < _n_delays; idx++ )
    {
        _new_weights[idx] = new_weights[idx];
        _has_changed.store( true );
    }
}

void laproque::FadingMultiDelay::reset()
{
    // Set buffer values to 0
    for ( ptrdiff_t idx = 0; idx < _buffer_size; idx++ )
    {
        _buffer[idx] = 0.f;
        _core_buffer[idx] = 0.f;
    }
    _writer = _buffer;
}

void laproque::FadingMultiDelay::clear_delays()
{
    for ( unsigned dly = 0; dly < N_DELAYS_MAX; ++dly ) {
        if ( _delays[dly].get_status() != DEAD ) _delays[dly].set_status( DYING );
    }
}

void laproque::FadingMultiDelay::print_buffer( unsigned n_frames )
{
    long part1 = _buffer_end - _writer;
    if ( part1 > ptrdiff_t(n_frames) ) {
        part1 = n_frames;
    }
    for ( long idx = 0; idx < part1; idx++) {
        printf("%f\n", _writer[idx]);
    }
    for ( long idx = 0; idx < ptrdiff_t(n_frames-part1); idx++) {
        printf("%f\n", _buffer[idx]);
    }
}


laproque::FadingMultiDelay::_DelayCore::_DelayCore( float* buf, float* buf_end, unsigned long n_delay, float** writer ) :
_buf( buf ), _buf_end( buf_end ), _wtr( writer ), _buf_size( buf_end - buf )
{
    set_delay( n_delay, 1.f );
    _status = BORN;
}

laproque::FadingMultiDelay::_DelayCore::_DelayCore() :
_buf( nullptr ), _buf_end( nullptr ), _buf_size( 0 )
{
    _status = DEAD;
}

laproque::FadingMultiDelay::_DelayCore::_DelayCore( const _DelayCore &obj ) :
_buf( obj._buf ), _buf_end( obj._buf_end ), _wtr( obj._wtr ), _buf_size( obj._buf_size )
{
    _rdr = obj._rdr;
    _n_dly = obj._n_dly;
    _status = obj._status;
    _to_fade = obj._to_fade;
    _old_rdr = obj._old_rdr;
    _wgt = obj._wgt;
    _old_wgt = obj._old_wgt;
}

laproque::FadingMultiDelay::_DelayCore &laproque::FadingMultiDelay::_DelayCore::operator= ( const laproque::FadingMultiDelay::_DelayCore &obj )
{
    this->_buf = obj._buf;
    this->_buf_end = obj._buf_end;
    this->_buf_size = obj._buf_size;
    this->_wtr = obj._wtr;
    this->_rdr = obj._rdr;
    this->_old_rdr = obj._old_rdr;
    this->_wgt = obj._wgt;
    this->_old_wgt = obj._old_wgt;
    
    this->_n_dly = obj._n_dly;
    this->_to_fade = obj._to_fade;
    this->_status = obj._status;
    
    return *this;
}

void laproque::FadingMultiDelay::_DelayCore::set_delay( unsigned long delay, float weight )
{
    _old_wgt = _wgt;
    _wgt = weight;
    
    _old_rdr = _rdr;
    _rdr = *_wtr - delay;
    if ( _rdr < _buf ) _rdr += _buf_size;
    
    _n_dly = delay;
    _to_fade = N_FADE;
    
    if ( _status == DEAD ) _status = BORN;
    else _status = CHANGE;
}


void laproque::FadingMultiDelay::_DelayCore::set_status( laproque::FadingMultiDelay::FadeBehavior status )
{
    _status = status;
    _to_fade = N_FADE;
}

unsigned long laproque::FadingMultiDelay::_DelayCore::get_delay()
{
    return _n_dly;
}

laproque::FadingMultiDelay::FadeBehavior laproque::FadingMultiDelay::_DelayCore::get_status()
{
    return _status;
}

void laproque::FadingMultiDelay::_DelayCore::process( float* output, unsigned  long n_frames )
{
    unsigned long idx;
    unsigned long n_rem = n_frames;
    
    //printf("\tRequested: %lu\t", n_frames);
    //printf("\tAvailable: %lu\t", _buf_end -  *_wtr);
    
    if ( _to_fade ) {
        
        float sample_old, sample_new;
        unsigned long fade_now = std::min( _to_fade, n_frames );
        unsigned long fade_start = N_FADE - _to_fade;
        
        switch (_status) {
            case BORN:
                //printf( "\tBORN. Fading: %lu at: %lu \n", fade_now, fade_start );
                for ( idx = fade_start; idx < fade_start+fade_now; idx++ ) {
                    sample_new = *_rdr * fade_in[idx] * _wgt;
                    *output = sample_new;
                    
                    ++output;
                    if ( ++_rdr == _buf_end) _rdr = _buf ;
                }
                break;
                
            case CHANGE:
                //printf( "\tCHANGED. Fading: %lu at: %lu \n", fade_now, fade_start );
                
                for ( idx = fade_start; idx < fade_start+fade_now; idx++ ) {
                    
                    sample_old = *_old_rdr * fade_out[idx];
                    sample_new = *_rdr * fade_in[idx];
                    
                    *output = sample_old*_old_wgt + sample_new*_wgt;
                    
                    ++output;
                    if ( ++_rdr == _buf_end ) _rdr = _buf;
                    if ( ++_old_rdr == _buf_end ) _old_rdr = _buf;
                }
                break;
                
            case DYING:
                //printf( "\tDYING. Fading: %lu at: %lu \n", fade_now, fade_start );
                for ( idx = fade_start; idx < fade_start+fade_now; idx++ ) {
                    sample_old = *_rdr * fade_out[idx];
                    *output = sample_old*_old_wgt;
                    
                    ++output;
                    if ( ++_rdr == _buf_end) _rdr = _buf;
                }
                break;
                
            default:
                break;
        }
        
        _to_fade -= fade_now;
        n_rem -= fade_now;
        
        if ( _to_fade == 0 ) {
            if ( _status == BORN ) {
                //printf("Hello...\n");
                _status = ALIVE;
            }
            if ( _status == DYING ) {
                _status = DEAD;
                //printf("Goodbye...\n");
            }
            else _status = ALIVE;
        }
    }
    
    // The part from here on is only executed when fading is completed.
    
    // Death is mute.
    if ( _status == DEAD) {
        for ( idx = 0; idx < n_rem; idx++) {
            output[idx] = 0.f;
            ++_rdr;
        }
        n_rem = 0;
    }
    
    unsigned long n_rdy;
    
    while ( n_rem ) {
        
        n_rdy = std::min( n_rem, (unsigned long)(_buf_end - _rdr) );
        
        for ( idx = 0; idx < n_rdy; idx++ ) {
            output[idx] = _rdr[idx] * _wgt;
        }
        
        _rdr += n_rdy;
        if ( _rdr  == _buf_end ) _rdr = _buf;
        
        n_rem -= n_rdy;
        
        output += n_rdy;
    }
}


const float laproque::FadingMultiDelay::fade_in[128] = {
    0.00000000000,
    0.00015297136,
    0.00061179191,
    0.00137618103,
    0.00244567054,
    0.00381960650,
    0.00549714779,
    0.00747726951,
    0.00975875836,
    0.01234021783,
    0.01522007026,
    0.01839655079,
    0.02186772041,
    0.02563144639,
    0.02968543582,
    0.03402720019,
    0.03865408897,
    0.04356326535,
    0.04875173047,
    0.05421630666,
    0.05995365232,
    0.06596025825,
    0.07223244756,
    0.07876637578,
    0.08555805683,
    0.09260331839,
    0.09989786148,
    0.10743723065,
    0.11521679163,
    0.12323180586,
    0.13147735596,
    0.13994839787,
    0.14863973856,
    0.15754610300,
    0.16666197777,
    0.17598184943,
    0.18549995124,
    0.19521048665,
    0.20510755479,
    0.21518504620,
    0.22543680668,
    0.23585659266,
    0.24643799663,
    0.25717458129,
    0.26805973053,
    0.27908676863,
    0.29024904966,
    0.30153959990,
    0.31295165420,
    0.32447808981,
    0.33611199260,
    0.34784618020,
    0.35967344046,
    0.37158653140,
    0.38357821107,
    0.39564114809,
    0.40776792169,
    0.41995120049,
    0.43218338490,
    0.44445711374,
    0.45676481724,
    0.46909892559,
    0.48145195842,
    0.49381637573,
    0.50618457794,
    0.51854896545,
    0.53090202808,
    0.54323619604,
    0.55554389954,
    0.56781756878,
    0.58004975319,
    0.59223294258,
    0.60435974598,
    0.61642271280,
    0.62841439247,
    0.64032745361,
    0.65215474367,
    0.66388887167,
    0.67552274466,
    0.68704921007,
    0.69846117496,
    0.70975178480,
    0.72091400623,
    0.73194098473,
    0.74282604456,
    0.75356262922,
    0.76414394379,
    0.77456367016,
    0.78481537104,
    0.79489284754,
    0.80478984118,
    0.81450039148,
    0.82401853800,
    0.83333837986,
    0.84245413542,
    0.85136049986,
    0.86005187035,
    0.86852288246,
    0.87676835060,
    0.88478338718,
    0.89256292582,
    0.90010225773,
    0.90739679337,
    0.91444200277,
    0.92123365402,
    0.92776763439,
    0.93403971195,
    0.94004631042,
    0.94578367472,
    0.95124822855,
    0.95643663406,
    0.96134585142,
    0.96597272158,
    0.97031450272,
    0.97436851263,
    0.97813224792,
    0.98160344362,
    0.98477989435,
    0.98765975237,
    0.99024111032,
    0.99252265692,
    0.99450272322,
    0.99618035555,
    0.99755424261,
    0.99862372875,
    0.99938821793,
    0.99984705448,
    1.00000000000
};

const float laproque::FadingMultiDelay::fade_out[128]
{
    1.00000000000,
    0.99984705448,
    0.99938821793,
    0.99862384796,
    0.99755436182,
    0.99618035555,
    0.99450284243,
    0.99252265692,
    0.99024122953,
    0.98765975237,
    0.98477989435,
    0.98160344362,
    0.97813224792,
    0.97436851263,
    0.97031462193,
    0.96597284079,
    0.96134597063,
    0.95643675327,
    0.95124822855,
    0.94578367472,
    0.94004631042,
    0.93403971195,
    0.92776751518,
    0.92123365402,
    0.91444188356,
    0.90739667416,
    0.90010219812,
    0.89256274700,
    0.88478314877,
    0.87676817179,
    0.86852264404,
    0.86005163193,
    0.85136020184,
    0.84245389700,
    0.83333802223,
    0.82401818037,
    0.81450009346,
    0.80478954315,
    0.79489243031,
    0.78481495380,
    0.77456325293,
    0.76414340734,
    0.75356197357,
    0.74282544851,
    0.73194026947,
    0.72091323137,
    0.70975095034,
    0.69846040010,
    0.68704837561,
    0.67552185059,
    0.66388797760,
    0.65215384960,
    0.64032661915,
    0.62841343880,
    0.61642175913,
    0.60435885191,
    0.59223198891,
    0.58004885912,
    0.56781655550,
    0.55554288626,
    0.54323524237,
    0.53090107441,
    0.51854801178,
    0.50618362427,
    0.49381545186,
    0.48145103455,
    0.46909794211,
    0.45676383376,
    0.44445616007,
    0.43218246102,
    0.41995027661,
    0.40776702762,
    0.39564025402,
    0.38357731700,
    0.37158563733,
    0.35967251658,
    0.34784525633,
    0.33611109853,
    0.32447722554,
    0.31295078993,
    0.30153873563,
    0.29024815559,
    0.27908602357,
    0.26805898547,
    0.25717392564,
    0.24643740058,
    0.23585604131,
    0.22543630004,
    0.21518456936,
    0.20510712266,
    0.19521012902,
    0.18549959362,
    0.17598152161,
    0.16666169465,
    0.15754583478,
    0.14863951504,
    0.13994817436,
    0.13147716224,
    0.12323164195,
    0.11521665007,
    0.10743711889,
    0.09989777207,
    0.09260325134,
    0.08555798978,
    0.07876633853,
    0.07223241776,
    0.06596024334,
    0.05995365977,
    0.05421632528,
    0.04875176400,
    0.04356331006,
    0.03865414113,
    0.03402725980,
    0.02968550101,
    0.02563151717,
    0.02186778933,
    0.01839662716,
    0.01522014756,
    0.01234029513,
    0.00975883473,
    0.00747734308,
    0.00549721671,
    0.00381966820,
    0.00244572316,
    0.00137622305,
    0.00061182171,
    0.00015298712,
    0.00000000000
};
