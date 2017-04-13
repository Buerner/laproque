//
//  Convolver.cpp
//  laproque - https://github.com/Buerner/laproque
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#include "Convolver.hpp"
#include <math.h>
#include <cstring>

laproque::Convolver::Convolver(float* imp_resp, unsigned long n_samples, unsigned block_size)
: _fft_size( block_size * 2 ),  _fft( block_size * 2 )
{
    _block_size = block_size;
    _fft_size = _block_size * 2;
    _spectrum_size = _block_size + 1;
    _n_parts = unsigned( ceilf( float(n_samples) / float(_block_size) ) );
    _spectra_size = _spectrum_size * _n_parts;
    
    // Double normalization factor in FFT because of zeropadded blocks.
    //_fft.set_norm_fact( 2.f * _fft.get_norm_fact() );
    
    _make_allocations();
    
    // Fill input buffer with zeros
    for ( unsigned idx = 0; idx < _fft_size; idx++ ) {
        _input[idx] = 0.f;
    }
    
    // Zeropadding of impulse response to multiple of block size.
    float* padded_imp_resp = fftwf_alloc_real( _block_size * (_n_parts) );
    for ( unsigned idx = _block_size * (_n_parts-1); idx < _block_size * (_n_parts); idx++ ) {
        padded_imp_resp[idx] = 0.f;
    }
    memcpy( padded_imp_resp, imp_resp, n_samples * sizeof(float) );
    
    // Calculate frequency response blocks.
    _compute_freq_resp( padded_imp_resp );
    
    fftwf_free( padded_imp_resp );
}

laproque::Convolver::Convolver( Convolver& conv )
: _fft( conv.get_fft_size() )
{
    _block_size = conv.get_block_size();
    _fft_size = conv.get_fft_size();
    _spectrum_size = conv.get_spectrum_size();
    _spectra_size = conv.get_spectra_size();
    _n_parts = conv.get_n_parts();
    
    _make_allocations();
    
}

laproque::Convolver::~Convolver()
{
    fftwf_free( _input_spectra );
    fftwf_free( _freq_resp_parts );
    
    fftwf_free( _input );
    fftwf_free( _output_spectr );
    fftwf_free( _multiply_buffer );
    fftwf_free( _result );
    
    fftwf_cleanup();
}

void laproque::Convolver::_make_allocations()
{
    // Allocate all the memory the Convolver needs.
    _input_spectra = fftwf_alloc_complex( _spectra_size );
    _freq_resp_parts = fftwf_alloc_complex( _spectra_size );
    _input = fftwf_alloc_real( _fft_size );
    
    _output_spectr = fftwf_alloc_complex( _spectrum_size );
    _multiply_buffer = fftwf_alloc_complex( _spectrum_size );
    _result = fftwf_alloc_real( _fft_size );
}

void laproque::Convolver::_compute_freq_resp( float* imp_resp )
{
//    // Create zero vector with twice the block size for overlap save convolution.
    float zero_padded_block[_fft_size];
    for ( unsigned idx = _fft_size; idx-- ; ) {
        zero_padded_block[idx] = 0.;
    }
    
    // Compute spectrum of the blocks.
    for ( unsigned part = 0; part < _n_parts; part++ )
    {
        memcpy( zero_padded_block, imp_resp + part * _block_size, _block_size*sizeof(float) );
        _fft.real2complex( zero_padded_block, _freq_resp_parts + part*_spectrum_size, false );
    }
}

void laproque::Convolver::_fast_conv()
{
    unsigned bin;
    
    // reset output spectrum
    for (bin = 0; bin < _spectrum_size; bin++ ) {
        _output_spectr[bin][0] = 0.;
        _output_spectr[bin][1] = 0.;
    }
    
    _fft.real2complex( _input, _input_spectra );
    
    // Multiply every partition and add to the output_spectrum
    for ( unsigned part = 0; part < _n_parts; part++ )
    {
        complex_multiply( _input_spectra + (part*_spectrum_size), _freq_resp_parts + (part*_spectrum_size), _multiply_buffer, _spectrum_size );
        
        for (bin = 0; bin < _spectrum_size; bin++ ) {
            _output_spectr[bin][0] += _multiply_buffer[bin][0];
            _output_spectr[bin][1] += _multiply_buffer[bin][1];
        }
    }
    
    // Run inverse transform.
    _fft.complex2real( _output_spectr, _result);
    
}

void laproque::Convolver::process( float *in_buffer, float *out_buffer )
{
    // Copy input into the buffer assigned to the FFT.
    memcpy( _input+_block_size, in_buffer, _block_size*sizeof(float) );
    
    _fast_conv();
    
    // Copy result to the output buffer.
    memcpy( out_buffer, _result+_block_size, _block_size*sizeof(float) );
    
    // Save last input.
    memcpy( _input, in_buffer, _block_size*sizeof(float) );
    
    // Shift the spectrum buffer for next run.
    memcpy( _input_spectra+_spectrum_size, _input_spectra, (_spectra_size-_spectrum_size)*sizeof(fftwf_complex) );
}

void laproque::Convolver::reset_input_buffer()
{
    for ( unsigned idx = 0; idx < _spectra_size; idx++ ) {
        _input_spectra[idx][0] = 0.f;
        _input_spectra[idx][1] = 0.f;
    }
    
    for ( unsigned idx = 0; idx < _fft_size; idx++ ) {
        _input[idx] = 0.f;
    }
}

unsigned laproque::Convolver::get_fft_size()
{
    return _fft_size;
}
unsigned laproque::Convolver::get_block_size()
{
    return _block_size;
}
unsigned laproque::Convolver::get_spectrum_size()
{
    return _spectrum_size;
}
unsigned laproque::Convolver::get_spectra_size()
{
    return _spectra_size;
}
unsigned laproque::Convolver::get_n_parts()
{
    return _n_parts;
}

void laproque::Convolver::set_freq_response( fftwf_complex *new_response )
{
    memcpy( _freq_resp_parts, new_response, _spectra_size*sizeof(fftwf_complex) );
}
