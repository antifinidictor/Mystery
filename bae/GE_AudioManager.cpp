/*
 * GE_AudioManager.cpp
 * Source file for the GE_AudioManager class.
 *
 * Based on code provided in the tutorial at http://www.kekkai.org/roger/sdl/mixer/.
 */

#include "GE_AudioManager.h"
#include "SDL.h"
using namespace std;

GE_AudioManager *GE_AudioManager::m_pInstance = NULL;
/*
    printf("Played music %i\n", uiID);
    if(fadeout == 0) {
        Mix_HaltMusic();
    } else {
        Mix_FadeOutMusic(fadeout);
    }

    if(uiID != NO_MUSIC) {
        if(fadein == 0) {
            Mix_PlayMusic(m_mMusic.find(uiID)->second, -1);
        } else {
            if(Mix_FadeInMusic(m_mMusic.find(uiID)->second, -1, fadein) == -1) {
                printf("Mix_FadeInMusic: %s\n", Mix_GetError());
            }
        }
    }
    */

int AudioThread(void *lock) {
    //Get the audio manager and the mutex
    GE_AudioManager *aud = GE_AudioManager::Get();
    SDL_mutex *pLock = (SDL_mutex*)lock;


    //Initialize audio
    int audio_rate = 22050;
    Uint16 audio_format = AUDIO_S16; /* 16-bit stereo */
    int audio_channels = 2;
    int audio_buffers = 4096;

    //Open audio device
    if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers)) {
        printf("Unable to open audio!\n");
        exit(1);
    }

    //Pretty much only needed for playing music
    while(aud->IsRunning()) {
        SDL_mutexP(pLock);
        if(aud->MusicQueued()) {
            aud->LoadNextSong();
        }
        SDL_mutexV(pLock);
        SDL_Delay(1000);
    }
    return 0;
}

void GE_AudioManager::LoadNextSong() {
    m_bQueuedSong = false;  //We will dequeue it shortly

    //End last song
    if(m_iCurMusicFadeout == 0) {
        Mix_HaltMusic();
    } else {
        Mix_FadeOutMusic(m_iCurMusicFadeout);
    }

    //Begin new song
    if(m_uiNextMusicID != NO_MUSIC) {
        if(m_iNextMusicFadein == 0) {
            Mix_PlayMusic(m_mMusic.find(m_uiNextMusicID)->second, -1);
        } else {
            if(Mix_FadeInMusic(m_mMusic.find(m_uiNextMusicID)->second, -1, m_iNextMusicFadein) == -1) {
                printf("Mix_FadeInMusic: %s\n", Mix_GetError());
            }
        }
    }

    m_iCurMusicFadeout = m_iNextMusicFadeout;

}

void GE_AudioManager::_Initialize() {
    if(m_pInstance)
        delete m_pInstance;

    m_pInstance = new GE_AudioManager();

    //Create thread once the manager is finished: we must be sure that all values are set correctly
    m_pInstance->InitializeThread();
}

GE_AudioManager::GE_AudioManager() {

    //Initialize static variables
    m_uiMusicID = 1; //0 is reserved for NO_MUSIC
    m_uiChunkID = 1; //0 is reserved for NO_SOUND

    /* If we actually care about what we got, we can ask here.  In this
     program we don't, but I'm showing the function call here anyway
     in case we'd want to know later. */
  //Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);

    //Initialize current song info

    m_iCurMusicFadeout = 0;
    m_uiNextMusicID = NO_MUSIC;
    m_iNextMusicFadein = 0;
    m_iNextMusicFadeout = 0;

    //Initialize threading information
    m_bRunning = true;
    m_bQueuedSong = false;
    m_pLock = NULL;
    m_pThread = NULL;
}

void GE_AudioManager::InitializeThread() {
    m_pLock = SDL_CreateMutex();
    m_pThread = SDL_CreateThread(AudioThread, m_pLock);
}

GE_AudioManager::~GE_AudioManager() {
    //Close music thread.
    m_bRunning = false;
    int status = 0;
    SDL_WaitThread(m_pThread, &status);
    SDL_DestroyMutex(m_pLock);

    //Halt sound.
    Mix_HaltMusic();
    Mix_HaltChannel(-1);

    //Free sound clips.
    for(map<uint,Mix_Chunk*>::iterator it = m_mChunks.begin();
            it != m_mChunks.end(); ++it) {
        Mix_FreeChunk(it->second);
    }
    m_mChunks.clear();

    //Free music.
    for(map<uint,Mix_Music*>::iterator it = m_mMusic.begin();
            it != m_mMusic.end(); ++it) {
        Mix_FreeMusic(it->second);
    }
    m_mMusic.clear();

    //Close the audio device.
	Mix_CloseAudio();
}



uint GE_AudioManager::LoadMusic(const char *filename) {
    SDL_mutexP(m_pLock);
    m_mMusic[m_uiMusicID] = Mix_LoadMUS(filename);
    SDL_mutexV(m_pLock);
    return m_uiMusicID++;
}

uint GE_AudioManager::LoadSound(const char *filename) {
    SDL_mutexP(m_pLock);
    m_mChunks[m_uiChunkID] = Mix_LoadWAV(filename);
    SDL_mutexV(m_pLock);
    return m_uiChunkID++;
}

void GE_AudioManager::PlayMusic(uint uiID, int fadein, int fadeout) {
    SDL_mutexP(m_pLock);
    m_uiNextMusicID = uiID;
    m_iNextMusicFadein = fadein;
    m_iNextMusicFadeout = fadeout;
    m_bQueuedSong = true;
    SDL_mutexV(m_pLock);

}

void GE_AudioManager::PlaySound(uint uiID, int reps, int channel) {
    Mix_PlayChannel(channel, m_mChunks[uiID], reps);    //Apparently this does not clash with the music channel...
}

