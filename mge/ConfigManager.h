#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <string>
#include <boost/property_tree/ptree.hpp>
#include "defs.h"

class ConfigManager
{
public:

    static void init() { m_pInstance = new ConfigManager(); }
    static void clean() { delete m_pInstance; m_pInstance = NULL; }
    static ConfigManager *get() { return m_pInstance; }

    //Loads config options, overriding duplicate values
    void load(const std::string &sConfigFile);

    //Saves config options to the specified config file
    void save(const std::string &sConfigFile);

    //Applies internal key mapping to the game
    void setKeyMapping();

    //Maps a particular key to a particular input
    void configKey(int iSdlKeyIn, int iGameKeyOut);

    std::string get(const std::string &key, const std::string &defaultValue = "");
    bool get(const std::string &key, bool defaultValue);
    int get(const std::string &key, int defaultValue);
    float get(const std::string &key, float defaultValue);
    Point get(const std::string &key, const Point &defaultValue);
    Rect get(const std::string &key, const Rect &defaultValue);
    Box get(const std::string &key, const Box &defaultValue);


    void set(const std::string &key, const std::string &value = "");
    void set(const std::string &key, bool value);
    void set(const std::string &key, int value);
    void set(const std::string &key, float value);
    void set(const std::string &key, const Point &value);
    void set(const std::string &key, const Rect &value);
    void set(const std::string &key, const Box &value);

    //Some things have lists of properties that they want to access
    boost::property_tree::ptree &getRawPropTree() { return m_configInfo; }

protected:
private:
    ConfigManager();
    virtual ~ConfigManager();

    static ConfigManager *m_pInstance;

    boost::property_tree::ptree m_configInfo;
};

#endif // CONFIGMANAGER_H
