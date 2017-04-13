//
//  JackPlugin.cpp
//  laproque - https://github.com/Buerner/laproque
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#include "JackPlugin.hpp"
#include <stdio.h>
#include <sndfile.h>

laproque::JackPlugin::JackPlugin(const char* name,
                       unsigned n_inputs,
                       unsigned n_outputs
                       )
{
    // Buffer for port names in JACK.
    char input_name[6];
    char output_name[6];
    
    // Store infos
    _n_in_ports = n_inputs;
    _n_out_ports = n_outputs;
    
    // Allocate port arrays.
    _in_ports = new jack_port_t*[_n_in_ports];
    _out_ports = new jack_port_t*[_n_out_ports];
    
    // Allocate buffer pointer arrays.
    _in_buffers = new jack_sample*[_n_in_ports];
    _out_buffers = new jack_sample*[_n_out_ports];
    
    _jack_client = jack_client_open( name, (jack_options_t)0, NULL );
    if(_jack_client == NULL) printf ( "Unable to open JACK client: %s \n", name );
    
    _block_size = jack_get_buffer_size( _jack_client );
    _sample_rate = jack_get_sample_rate( _jack_client );
    
    // Create JACK input for every jack channel.
    for ( unsigned port_number = 0; port_number<n_inputs; port_number++ ) {
        
        sprintf( input_name, "in_%u", port_number+1 );
        
        _in_ports[port_number] = jack_port_register (_jack_client,
                                                    input_name,
                                                    JACK_DEFAULT_AUDIO_TYPE,
                                                    JackPortIsInput,
                                                    0);
        if ( _in_ports[port_number] == NULL ) {
            printf( "Error while registering INPUT port number %i. \n", port_number );
        }
    }
    
    // Create JACK output for every channel.
    for ( unsigned port_number = 0; port_number<n_outputs; port_number++ ) {
        
        sprintf( output_name, "out_%u", port_number+1 );
        
        _out_ports[port_number] = jack_port_register (_jack_client,
                                                     output_name,
                                                     JACK_DEFAULT_AUDIO_TYPE,
                                                     JackPortIsOutput,
                                                     0);
        if ( _out_ports[port_number] == NULL ) {
            printf( "Error while registering OUTPUT port number %i. \n", port_number );
        }
    }
}

unsigned laproque::JackPlugin::get_n_inputs()
{
    return _n_in_ports;
}

unsigned laproque::JackPlugin::get_n_outputs()
{
    return _n_out_ports;
}

void laproque::JackPlugin::activate()
{
    if( jack_set_process_callback(_jack_client, audio_callback, this) )
        printf("Unable to set JACK audio callback funtion.");
    
    if( jack_activate(_jack_client) )
        printf("Unable to activate JACK client. \n");
    else _is_active = true;
}

bool laproque::JackPlugin::is_active()
{
    return _is_active;
}

void laproque::JackPlugin::deactivate()
{
    if ( _is_active )
    {
        jack_deactivate( _jack_client );
        _is_active = false;
    }
}


void laproque::JackPlugin::impulse_response( float** outputs, unsigned int n_frames )
{
    unsigned prt, idx;
    
    // Reserve memory for impulses
    float* impulses[_n_in_ports];
    for ( prt = 0; prt < _n_in_ports; prt++ ) {
        impulses[prt] = new float[n_frames];
        
        // Creat impulse for current port.
        impulses[prt][0] = 1.f;
        for ( idx = 1; idx < n_frames; idx++ ) {
            impulses[prt][idx] = 0.f;
        }
    }

    unsigned n_blocks = n_frames / _block_size;
    
    // Process audio callback funtion with full blocks fitting into n_frames.
    for ( unsigned blk = 0; blk < n_blocks; blk++ ) {
        render_audio( _block_size, impulses, outputs );
        for ( prt = 0; prt < _n_in_ports; prt++ ) {
            impulses[prt] += _block_size;
        }
        for ( prt = 0; prt < _n_out_ports; prt++ ) {
            outputs[prt] += _block_size;
        }
    }
    
    // Process remaining part.
    unsigned rest = n_frames - n_blocks*_block_size;
    render_audio( rest, impulses, outputs );
    
    // Reset pointers
    for ( prt = 0; prt < _n_in_ports; prt++ ) {
        impulses[prt] -= n_blocks * _block_size;
    }
    for ( prt = 0; prt < _n_out_ports; prt++ ) {
        outputs[prt] -= n_blocks * _block_size;
    }
    
    // Clean up
    for ( prt = 0; prt < _n_in_ports; prt++ ) {
        delete [] impulses[prt];
    }

}

void laproque::JackPlugin::write_imp_resp( unsigned int n_frames )
{
    unsigned prt;
    
    // Reserve memory for resulting impulse responses
    float* responses[ _n_out_ports ];
    for ( prt = 0; prt < _n_out_ports; prt++ ) {
        responses[prt] = new float[n_frames];
    }
    
    impulse_response( responses, n_frames );
    
    // Write responses to wav file.
    // Set audio format for sndfile.
    SF_INFO audio_format;
    audio_format.channels = 1;
    audio_format.samplerate = int(_sample_rate);
    audio_format.frames = n_frames;
    audio_format.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
    
    SNDFILE* audio_file;
    char file_name[20];
    
    // Write wav file for every output channel.
    for ( prt = 0; prt < _n_out_ports; prt++ ) {
        sprintf( file_name, "imp_resp_ch%02i.wav", prt );
        audio_file = sf_open( file_name, SFM_WRITE, &audio_format );
        sf_write_float( audio_file, responses[prt], n_frames );
        sf_close( audio_file );
    }
    
    // Clean up.
    for ( prt = 0; prt < _n_out_ports; prt++ ) {
        delete [] responses[prt];
    }
}

laproque::JackPlugin::~JackPlugin()
{
    if (_is_active) {
        deactivate();
    }
    jack_client_close( _jack_client );
    
    delete [] _in_ports;
    delete [] _out_ports;
    
    delete [] _out_buffers;
    delete [] _in_buffers;
}

jack_nframes_t laproque::JackPlugin::get_sample_rate()
{
    return _sample_rate;
}

jack_nframes_t laproque::JackPlugin::get_block_size()
{
    return _block_size;
}

jack_client_t* laproque::JackPlugin::client()
{
    return _jack_client;
}

jack_port_t* laproque::JackPlugin::get_out_port( unsigned index )
{
    if ( index < _n_out_ports ) return _out_ports[index];
    else return nullptr;
}
