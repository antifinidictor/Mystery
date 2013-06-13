#ifndef BASIC_AUDIO_ENGINE_H
#define BASIC_AUDIO_ENGINE_H

#include "mge/AudioEngine.h"
#include "mge/defs.h"
#include <stdlib.h>
#include <map>
#include "SDL_mixer.h"

#define NO_SOUND 0
#define NO_MUSIC 0

class BasicAudioEngine : public AudioEngine
{
public:
    static void init();
    static void clean() { delete bae; }
    static BasicAudioEngine *get() { return bae; }
    
    uint loadMusic(const char *filename);
    uint loadSound(const char *filename);

    void playMusic(uint uiID, int fadeout=0, int fadein = 0);
    void playSound(uint uiID, int reps=0, int channel=-1);

    //For the thread
    bool isRunning() { return m_bRunning; }
    bool musicQueued() { return m_bQueuedSong; }
    void loadNextSong();

protected:
private:
    BasicAudioEngine();
    virtual ~BasicAudioEngine();

    static BasicAudioEngine *bae;
    
    struct SoundQ {
        SoundQ(uint i, int r, int c) { id = i; reps = r; chnl = c; }
        uint id;
        int reps;
        int chnl;
    };

    //Sound management:  maps of sound ids to the sounds themselves
    std::map<uint, Mix_Music*> m_mMusic;
    std::map<uint, Mix_Chunk*> m_mChunks;   //Sounds

    //Information about current music/sounds
    int m_iCurMusicFadeout;
    uint m_uiNextMusicID;
    int  m_iNextMusicFadein,
         m_iNextMusicFadeout;

    //Sound id management: keeps track of sound ids
    uint m_uiMusicID;
    uint m_uiChunkID;

    //Thread data
    SDL_Thread *m_pThread;
    bool m_bRunning;
    bool m_bQueuedSong;
    SDL_mutex *m_pLock;

    void initThread();
};

#endif // BASICAUDIOENGINE_H
