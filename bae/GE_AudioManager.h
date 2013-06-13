/*
 * GE_AudioManager.h
 * Manages audio for the GE class.
 *
 * All game objects are permitted access to this class.
 */
#ifndef GE_AUDIO_MANAGER_H
#define GE_AUDIO_MANAGER_H

#include <stdlib.h>
#include <map>
#include "SDL_mixer.h"

#define NO_SOUND 0
#define NO_MUSIC 0

class GE_AudioManager {
private:
    struct SoundQ {
        SoundQ(uint i, int r, int c) { id = i; reps = r; chnl = c; }
        uint id;
        int reps;
        int chnl;
    };

    GE_AudioManager();

    static GE_AudioManager *m_pInstance;

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

    void InitializeThread();

public:
    virtual ~GE_AudioManager();
    static void _Initialize();
    static void _Clean()           { delete m_pInstance; m_pInstance = NULL; }

    //Visible to the user
    static GE_AudioManager* Get() { return m_pInstance; }

    uint LoadMusic(const char *filename);
    uint LoadSound(const char *filename);

    void PlayMusic(uint uiID, int fadeout=0, int fadein = 0);
    void PlaySound(uint uiID, int reps=0, int channel=-1);

    //For the thread
    bool IsRunning() { return m_bRunning; }
    bool MusicQueued() { return m_bQueuedSong; }
    void LoadNextSong();
};

#endif
