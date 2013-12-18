#include "BasicAudioEngine.h"
#include "SDL.h"
using namespace std;

//Static members
BasicAudioEngine *BasicAudioEngine::bae;

//Constructor
BasicAudioEngine::BasicAudioEngine() {

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

//Destructor
BasicAudioEngine::~BasicAudioEngine() {
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

//Thread
int audioThread(void *lock) {
    //Get the audio manager and the mutex
    BasicAudioEngine *aud = BasicAudioEngine::get();
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
    while(aud->isRunning()) {
        SDL_mutexP(pLock);
        if(aud->musicQueued()) {
            aud->loadNextSong();
        }
        SDL_mutexV(pLock);
        SDL_Delay(1000);
    }
    return 0;
}

//Initialization
void BasicAudioEngine::init() {
    bae = new BasicAudioEngine();
    bae->initThread();
}

void BasicAudioEngine::initThread() {
    m_pLock = SDL_CreateMutex();
    m_pThread = SDL_CreateThread(audioThread, "MGE Audio", m_pLock);
}

void BasicAudioEngine::loadNextSong() {
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

void
BasicAudioEngine::loadMusic(uint uiId, const char *filename) {
    SDL_mutexP(m_pLock);
    m_mMusic[uiId] = Mix_LoadMUS(filename);
    if(m_mMusic[uiId] == NULL) {
        printf("Error: Failed to load muisc %d (%s)- %s\n", uiId, filename, Mix_GetError());
    }
    SDL_mutexV(m_pLock);
}

void
BasicAudioEngine::loadSound(uint uiId, const char *filename) {
    SDL_mutexP(m_pLock);
    m_mChunks[uiId] = Mix_LoadWAV(filename);
    if(m_mChunks[uiId] == NULL) {
        printf("Error: Failed to load sound chunk %d (%s)- %s\n", uiId, filename, Mix_GetError());
    }
    SDL_mutexV(m_pLock);
}

void
BasicAudioEngine::playMusic(uint uiId, int fadein, int fadeout) {
    SDL_mutexP(m_pLock);
    m_uiNextMusicID = uiId;
    m_iNextMusicFadein = fadein;
    m_iNextMusicFadeout = fadeout;
    m_bQueuedSong = true;
    SDL_mutexV(m_pLock);

}

void
BasicAudioEngine::playSound(uint uiId, int reps, int channel) {
    Mix_PlayChannel(channel, m_mChunks[uiId], reps);    //Apparently this does not clash with the music channel...
}

