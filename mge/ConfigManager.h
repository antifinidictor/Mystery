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

    std::string get(const std::string &key, const std::string &defaultValue = "");
    bool get(const std::string &key, const bool &defaultValue);
    int get(const std::string &key, int defaultValue);
    float get(const std::string &key, float defaultValue);
    Point get(const std::string &key, const Point &defaultValue);
    Box get(const std::string &key, const Box &defaultValue);

protected:
private:
    ConfigManager();
    virtual ~ConfigManager();

    static ConfigManager *m_pInstance;

    boost::property_tree::ptree m_configInfo;
};

#endif // CONFIGMANAGER_H
