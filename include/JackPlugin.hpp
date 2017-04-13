//
//  JackPlugin.hpp
//  laproque - https://github.com/Buerner/laproque
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#ifndef JackClient_hpp
#define JackClient_hpp

#include <jack/jack.h>

// short version for jack sample type
typedef jack_default_audio_sample_t jack_sample; 

/**
 * @class JackPlugin
 *
 * @brief More or less an abstract/wrapper class hiding all the fuzz to create plugins for JACK Audio.
 *
 * Just create your class, inherit from this class, and initialize it with a name and number of input/output ports. Implement the render_audio(jack_nframes_t n_frames, jack_sample** in_buffers, jack_sample** out_buffers ); function and when ready use acitvate() to run your plugin.
 *
*/
class JackPlugin
{
public:
    /**
     @brief Constructor. Must be called by derived class.
     @param name This is the name the client is registered with.
     @param n_inputs Number of input ports of your JACK client.
     @param n_outputs Number of output ports of your JACK client.
     */
    JackPlugin( const char* name
               ,unsigned n_inputs
               ,unsigned n_outputs
               );
    ~JackPlugin();

    unsigned get_n_inputs();
    unsigned get_n_outputs();
    
    /**
     * @brief This function generates the audio samples. It needs to be implemented by every derived class.
     *
     * @param n_frames Number of audio frames which need to be processed.
     * @param in_buffers Array containing pointers to the audio input buffers for every JACK port.
     * @param out_buffers Array containing pointers to the audio output buffers for every JACK port.
     */
    virtual void render_audio( jack_nframes_t n_frames
                              ,jack_sample** in_buffers
                              ,jack_sample** out_buffers
                              ) = 0;
    
    /** @returns The pointer to the client instance */
    jack_client_t* client();
    
    /** @returns A port pointer in case the index exists */
    jack_port_t* get_out_port( unsigned index );
    
    /**
     * @brief Registers audio callback function and activates the plugin.
     */
    void activate();
    
    /**
     * @brief Deactivates the Plugin in the JACK server.
     */
    void deactivate();
    
    /**
     * @brief Returns activation status
     */
    bool is_active();
    
    /**
     * @brief Function which is used as callback function for JACK Audio.
     *
     * Collects the audio data buffers and calls the render_audio() function of the casted plugin pointer.
     */
    static int audio_callback( jack_nframes_t n_frames, void* plugin_instance )
    {
        // Cast custom JackClient.
        JackPlugin* plugin = (JackPlugin*)plugin_instance;
        
        // Get input buffers.
        for ( unsigned prt_nr = 0; prt_nr < plugin->_n_in_ports; prt_nr++ ) {
            plugin->_in_buffers[prt_nr] = (jack_sample*) jack_port_get_buffer( plugin->_in_ports[prt_nr], n_frames );
        }
        
        // Get output buffers.
        for ( unsigned prt_nr = 0; prt_nr<plugin->_n_out_ports; prt_nr++ ) {
            plugin->_out_buffers[prt_nr] = (jack_sample*) jack_port_get_buffer( plugin->_out_ports[prt_nr], n_frames );
        }
        
        // Call the function that generates all the audio data.
        plugin->render_audio( n_frames, plugin->_in_buffers, plugin->_out_buffers );
        
        return 0;
        
    };
    
    /**
     * @brief Computes impulse response of length n_frames and writes it to outputs.
     * @param outputs Pointers with output buffer mempory. First dimension: number of output channels, Second dimension number of frames;
     * @param n_frames Desired length of impulse responses.
     */
    void impulse_response( float** outputs, unsigned n_frames );
    
    /**
     * @brief Creates impulse responses and writes them to wav files.
     * @param n_frames Desired length of impulse responses.
     */
    void write_imp_resp( unsigned n_frames );
    
    /** @brief Returns the audio sample rate in frames per second. */
    jack_nframes_t get_sample_rate();
    
    /** @brief Returns the audio block processing size in frames. */
    jack_nframes_t get_block_size();
    
protected:
    jack_client_t* _jack_client;
    jack_nframes_t _block_size;
    jack_nframes_t _sample_rate;
    
    unsigned _n_in_ports;
    unsigned _n_out_ports;
    
private:
    jack_port_t** _in_ports;
    jack_port_t** _out_ports;
    
    jack_sample** _in_buffers;
    jack_sample** _out_buffers;
    
    bool _is_active;
};

#endif /* __FDN__JackClient__ */
