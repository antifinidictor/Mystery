/*
 * Manages game state
 */
#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include <stack>
#include <map>

#include "mge/GameObject.h"
#include "mge/Event.h"
#include "game/game_defs.h"

#include "d3re/d3re.h"
#include "pwe/PartitionedWorldEngine.h"
#include "tpe/TimePhysicsEngine.h"
#include <boost/filesystem.hpp>

//Special filenames
#define WORLD_FILE_INFO "res/world.info"
#define WORLD_FILE_XML "res/world.xml"
#define SAVE_TEMPLATE_FILE_INFO "res/save-template.info"
#define SAVE_TEMPLATE_FILE_XML "res/save-template.xml"

class ElementalVolume;
class Player;

class GameManager : public GameObject
{
public:
    static void init() { m_pInstance = new GameManager(PWE::get()->genId()); }
    static GameManager *get() { return m_pInstance; }
    static void clean() { delete m_pInstance; }

    //File i/o
    static GameObject* read(const boost::property_tree::ptree &pt, const std::string &keyBase);
    virtual void write(boost::property_tree::ptree &pt, const std::string &keyBase);

    //General
    virtual bool update(float fDeltaTime);

    virtual uint getId()                        { return m_uiId; }
    virtual bool getFlag(uint flag)             { return GET_FLAG(m_uiFlags, flag); }
    virtual void setFlag(uint flag, bool value) { m_uiFlags = SET_FLAG(m_uiFlags, flag, value); }
    virtual uint getType()                      { return TYPE_MANAGER; }
    virtual const std::string getClass()        { return getClassName(); }
    static const std::string getClassName()     { return "GameManager"; }

    //Render model
    virtual RenderModel  *getRenderModel()  { return NULL; }
    virtual PhysicsModel *getPhysicsModel() { return NULL; }

    //Input
    virtual int callBack(uint uiID, void *data, uint eventId);

    //Other
    void addActiveVolume(ElementalVolume *ev);
    void removeActiveVolume(uint id);
    ElementalVolume *getTopVolume();

    void registerPlayer(Listener *pPlayer);

    void setDefaultInputMapping();
    void setTypingInputMapping();
    void resetInputMapping();
    bool newGame(const std::string &filename);     //Creates new save file & loads it
    bool loadGame(const std::string &filename);    //Loads save file (world file should already be loaded)
    bool saveGame(const std::string &filename);
    void getCurGameFileRoot(std::string &result);

    void readWorldFile();
    void readSaveFile();

private:
    GameManager(uint uiId);
    virtual ~GameManager();

    void initBasicHud();
    void cleanBasicHud();
    void initPlayerHud();
    void cleanPlayerHud();

    void cleanGame();   //Cleans old information in preparation for a new/loaded game

    void fadeArea();

    void pushState(GameManagerState eNewState);
    void popState();
    void swapState(GameManagerState eNewState); //Pushes & pops without initializing the state below

    void initCurState();
    void cleanCurState();

    bool validateSaveFileName(const std::string &filename, bool bMustExist);

    static GameManager *m_pInstance;

    uint m_uiId;
    flag_t m_uiFlags;
    float m_fFadeTimer;
    uint m_uiNextArea;
    Color m_crWorld;
    Color m_crBackground;
    bool m_bWorldIsClean;   //True if world is definitely clean

    boost::filesystem::path m_fsGameFile;

    Listener *m_pPlayerListener; //The player, as a listener

    std::stack<GameManagerState> m_skState;
    std::map<uint, ElementalVolume*> m_mActiveVolumes;
};

#endif // GAME_MANAGER_H
