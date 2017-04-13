//
//  JackPlayer.cpp
//  laproque - https://github.com/Buerner/laproque
//
//  Copyright © 2017 Martin Bürner. All rights reserved.
//  Licensed under the MIT License. See LICENSE.md file in the project root for full license information.  
//

#include "JackPlayer.hpp"

#include <math.h>
#include <sndfile.h>
#include <dirent.h>
#include <algorithm>
#include <cstring>

laproque::JackPlayer::JackPlayer() :
JackPlugin("Player", 0, 1), _gen(std::random_device{}())
{
    _setup_fades();
}

unsigned laproque::JackPlayer::add_file( std::string file_path )
{
    _playlist.push_back( file_path );
    _read_audio( file_path );
    return unsigned(_playlist.size() - 1);
}

void laproque::JackPlayer::add_directory( std::string dir_path )
{
    DIR *dir = opendir( dir_path.c_str() );
    if( !dir )
    {
        return;
    }
    
    std::string suffix{".wav"};
    char name_buffer[1024];
    
    dirent *entry;
    while( (entry = readdir(dir)) )
    {
        
        if ( std::equal( suffix.rbegin(), suffix.rend(), std::string{entry->d_name}.rbegin() ) )
        {
            sprintf( name_buffer, "%s%s", dir_path.c_str(), entry->d_name );
            add_file( name_buffer );
        }
    }
    
    closedir(dir);
}

void laproque::JackPlayer::select_by_idx( unsigned int idx )
{
    if ( idx < _playlist.size() )
    {
        if (_playing.load() ) stop();
        _current_idx.store( idx );
        _player_position = &_audio_buffers[_current_idx.load()][0];
        _n_ready = _audio_buffers[_current_idx.load()].size();
        printf("Playing: %s\n", _playlist[_current_idx.load()].c_str());
    }
}

void laproque::JackPlayer::shuffle()
{
    std::uniform_int_distribution<unsigned> distr(0, _n_tracks);
    if ( _playing.load() ) stop();
    select_by_idx( distr(_gen) );
}

void laproque::JackPlayer::shuffle_unique()
{
    std::uniform_int_distribution<unsigned> distr(0, _n_tracks);
    
    unsigned play_idx;
    do {
        play_idx = distr(_gen);
    } while ( std::find(_shuffle_list.begin(), _shuffle_list.end(), play_idx ) == _shuffle_list.end() );
    
    stop();
    select_by_idx(play_idx);
    
}

void laproque::JackPlayer::play()
{
    if ( _n_ready == 0 && _playlist.size()>0 ) {
        select_by_idx(0);
    }
    
    if ( !_playing.load() ) _started.store( true );
    _playing.store( true );
}

void laproque::JackPlayer::pause()
{
    if ( _playing.load() ) _stopped.store( true );
    _playing.store( false );
}

void laproque::JackPlayer::stop()
{
    if ( _playing.load() ) _stopped.store( true );
    _playing.store( false );
    
    _player_position = &_audio_buffers[_current_idx.load()][0];
    _n_ready = _audio_buffers[_current_idx.load()].size();
}

void laproque::JackPlayer::start( std::string file_path )
{
    stop();
    select_by_idx( add_file( file_path ) );
    play();
}

void laproque::JackPlayer::next()
{
    _shuffle.load() ? shuffle() : _jump(1);
}

void laproque::JackPlayer::previous()
{
    _jump( -1 );
}

void laproque::JackPlayer::_jump( int n_files )
{
    if ( _playlist.size()>0 ) {
        int new_idx = _current_idx.load();
        if ( _playing.load() ) pause();
        
        new_idx += n_files;
        new_idx %= _playlist.size();

        select_by_idx( new_idx );
    }
}

void laproque::JackPlayer::_setup_fades()
{
    _fadein.resize( _block_size );
    _fadeout.resize( _block_size );
    
    float phase_increment = _block_size / M_PI / 2.f;
    float phase = 0;
    
    for ( unsigned idx = 0; idx < _block_size; idx++ ) {
        _fadein[idx] = sinf( phase );
        _fadein[idx] *= _fadein[idx];
        
        _fadeout[_block_size-idx -1] = _fadein[idx];
        
        phase += phase_increment;
    }
}

void laproque::JackPlayer::render_audio(  jack_nframes_t n_frames
                              , jack_sample **in_buffers
                              , jack_sample **out_buffers
                              )
{
    if ( !_playing.load() ) {
        for ( unsigned idx = 0; idx < n_frames; idx++ ) {
            out_buffers[0][idx] = 0.f;
        }
    }
    else
    {
        sf_count_t playable = n_frames > _n_ready ? _n_ready : n_frames;
        // Handle fade in.
        if ( _started ) {
            for ( unsigned idx = 0; idx < playable; idx++ ) {
                out_buffers[0][idx] = _player_position[idx] * _fadein[idx];
            }
            _started.store( false );
        }
        // Handle fade out.
        else if ( _stopped ) {
            for ( unsigned idx = 0; idx < playable; idx++ ) {
                out_buffers[0][idx] = _player_position[idx] * _fadeout[idx];
            }
            _stopped.store( false );
        }
        // Copy data to output buffer;
        else {
            std::memcpy( out_buffers[0], _player_position, playable*sizeof(float) );
        }
        
        
        // Write zeros if data to short for block.
        if ( playable < n_frames ) {
            for (unsigned idx = playable; idx < n_frames; idx++ ) {
                out_buffers[0][idx] = 0.f;
            }
        }
        
        _player_position += playable;
        _n_ready -= playable;
        
        if ( _n_ready == 0)
        {
            if ( _loop.load() ) {
                stop();
                play();
            }
            else if ( _autoplay.load() )
            {
                stop();
                next();
                play();
            }
            else _playing.store( false );
        }
    }
}

void laproque::JackPlayer::_read_audio( std::string file_path )
{
    if ( !_playing ) {
        SF_INFO audio_info;
        SNDFILE* audio_file = sf_open( file_path.c_str(), SFM_READ, &audio_info );
        if (audio_file)
        {
            _audio_buffers.resize(_n_tracks+1);
            _audio_buffers[_n_tracks].resize( audio_info.frames );
            
            sf_read_float( audio_file, &_audio_buffers[_n_tracks][0], audio_info.frames );
            
            _n_tracks++;
        }
        sf_close( audio_file );
    }
}

bool laproque::JackPlayer::is_playing()
{
    return _playing.load();
}

bool laproque::JackPlayer::get_autoplay()
{
    return _autoplay.load();
}

bool laproque::JackPlayer::get_loop()
{
    return _loop.load();
}

bool laproque::JackPlayer::get_shuffle()
{
    return _shuffle.load();
}

unsigned laproque::JackPlayer::get_n_tracks()
{
    return _audio_buffers.size();
}

void laproque::JackPlayer::set_autoplay( bool value )
{
    _autoplay.store( value );
}

void laproque::JackPlayer::set_loop( bool value )
{
    _loop.store( value );
}

void laproque::JackPlayer::set_shuffle( bool value )
{
    _shuffle.store( value );
}

laproque::JackPlayer::~JackPlayer()
{
    deactivate();
}

void laproque::JackPlayer::print_status()
{
    printf("Current Track: %i %lu %s\n", _current_idx.load(), _audio_buffers[_current_idx].size(), _playlist[_current_idx].c_str() );
    printf( "----- Playlist: %i Tracks -----\n", _n_tracks  );
    for ( unsigned trk = 0; trk < _n_tracks; trk++ ) {
        printf( "\t%s\n", _playlist[trk].c_str() );
    }
    printf("Loop: %s\n", _loop.load() ? "true" : "false" );
    printf("Shuffle: %s\n", _shuffle.load() ? "true" : "false" );
    printf("Autoplay: %s\n", _autoplay.load() ? "true" : "false" );
}

