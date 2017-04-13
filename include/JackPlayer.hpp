//
//  JackPlayer.hpp
//  laproque - https://github.com/Buerner/laproque
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#ifndef JackPlayer_hpp
#define JackPlayer_hpp

#include <stdio.h>
#include <string>
#include <vector>
#include <atomic>
#include <random>
#include <sndfile.h>
#include "JackPlugin.hpp"


/**
 @class Plays single-channel wav files in Jack Audio Connection Kit.
 */
class JackPlayer : public JackPlugin
{
public:
    JackPlayer();
    ~JackPlayer();
    void render_audio(jack_nframes_t n_frames, jack_sample **in_buffers, jack_sample **out_buffers);
    
    /** @returns True if the instance ist currently playing back audio. */
    bool is_playing();
    
    /** @brief Add a file to the playlist. */
    unsigned add_file( std::string file_path );
    
    /** @brief Add all .wav files in a folder. */
    void add_directory( std::string dir_path );
    
    /** @brief Start playback of a specified file. */
    void start( std::string file_path );
    
    /** @brief Select a file from the playlist by its index.*/
    void select_by_idx( unsigned idx );
    
    /** @brief Choose random track from current playlist. */
    void shuffle();
    
    /** @brief Choose reandom track without repeating a track before all tracks have been played back. */
    void shuffle_unique();
    
    /** @bief Start playback of currently selected file. */
    void play();
    
    /** @brief Pause playback at current position. */
    void pause();
    
    /** @brief Stop playback and go to beginning of the track which is currently being played. */
    void stop();
    
    /** @brief Go to next track in playlist. */
    void next();
    
    /** @brief Go to previous track in the playlist. */
    void previous();
    
    /** @brief Sets whether next track in playlist starts automatially after end of current track. */
    void set_autoplay( bool value );
    
    /** @brief If set to true, playlists starts over after last track. */
    void set_loop( bool value );
    
    void set_shuffle( bool value );
    
    bool get_autoplay();
    bool get_loop();
    bool get_shuffle();
    unsigned get_n_tracks();
    
    void print_status();
    
private:
    std::atomic<bool> _playing{ false };
    std::atomic<bool> _shuffle{ false };
    std::atomic<bool> _loop{ false };
    std::atomic<bool> _autoplay{ false };
    
    std::atomic<bool> _started{ false };
    std::atomic<bool> _stopped{ false };
    
    std::vector<std::string> _playlist;
    
    std::atomic<unsigned> _current_idx{ 0 };
    unsigned _n_tracks = 0;
    
    std::vector<float> _fadein;
    std::vector<float> _fadeout;
    
    void _setup_fades();
    
    std::mt19937 _gen;
    
    std::vector< std::vector<float> > _audio_buffers;
    float* _player_position;
    
    std::vector< unsigned > _shuffle_list;
    
    sf_count_t _n_ready = 0;
    
    void _read_audio( std::string file_path );
    
    void _jump( int n_files );
};



#endif /* JackPlayer_hpp */

