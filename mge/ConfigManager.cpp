#include "ConfigManager.h"
#include "ModularEngine.h"
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
using namespace std;

ConfigManager *ConfigManager::m_pInstance;
static const string CONFIG_OPT_BASE = "options.";
static const string USER_KEY_BASE = "keys";
ConfigManager::ConfigManager()
{
    //ctor
}

ConfigManager::~ConfigManager()
{
    //dtor
}


void
ConfigManager::load(const std::string &sConfigFile) {
    try {
    //Read either a .info or .xml file
    uint fileExtIndex = sConfigFile.find_last_of(".");
    if(sConfigFile.substr(fileExtIndex) == ".info") {
        read_info(sConfigFile, m_configInfo);
    } else {
        read_xml(sConfigFile, m_configInfo);
    }

    } catch(exception &e) {
        printf("Could not read config options: %s\n", e.what());
    }

    //Apply user-specified key configurations
    setKeyMapping();
}

void
ConfigManager::save(const std::string &sConfigFile) {
    //Write either a .info or .xml file
    uint fileExtIndex = sConfigFile.find_last_of(".");
    if(sConfigFile.substr(fileExtIndex) == ".info") {
        write_info(sConfigFile, m_configInfo);
    } else {
        write_xml(sConfigFile, m_configInfo);
    }
}


void
ConfigManager::setKeyMapping() {
    using boost::property_tree::ptree;
    using boost::lexical_cast;
    using boost::bad_lexical_cast;

    try {
    //Special configuration options should be dealt with: In this case, user-specified keys
    BOOST_FOREACH(ptree::value_type &a, m_configInfo.get_child(USER_KEY_BASE)) {
        //Extract the numerical key mappings (probably isn't human readable)
        string key = USER_KEY_BASE + "." + a.first.data();
        int gameKeyId = m_configInfo.get(key, -1);
        try{
            int sdlKeyId = lexical_cast<int>(a.first.data());

            //Map the inputs, if numbers are valid
            if(gameKeyId != -1) {
                MGE::get()->mapInput(sdlKeyId, gameKeyId);
            }
        } catch(const bad_lexical_cast &) {
            printf("Could not read key %s: not a number\n", a.first.data());
        }
    }
    } catch(exception &e) {
        printf("Could not apply keys: %s\n", e.what());
    }
}

void
ConfigManager::configKey(int iSdlKeyIn, int iGameKeyOut) {
    using boost::lexical_cast;
    string propTreeKey = USER_KEY_BASE + "." + lexical_cast<string>(iSdlKeyIn);
    m_configInfo.put(propTreeKey, iGameKeyOut);
}

std::string
ConfigManager::get(const std::string &key, const std::string &defaultValue) {
    return m_configInfo.get(CONFIG_OPT_BASE + key, defaultValue);
}

bool
ConfigManager::get(const std::string &key, bool defaultValue) {
    string sVal = m_configInfo.get(CONFIG_OPT_BASE + key, defaultValue ? "true" : "false");
    bool bVal = defaultValue;
    if(sVal.compare("true") == 0 || sVal.compare("yes") == 0) {
        bVal = true;
    } else if(sVal.compare("false") == 0 || sVal.compare("no") == 0) {
        bVal = false;
    }
    return bVal;
}

int
ConfigManager::get(const std::string &key, int defaultValue) {
    return m_configInfo.get(CONFIG_OPT_BASE + key, defaultValue);
}

float
ConfigManager::get(const std::string &key, float defaultValue) {
    return m_configInfo.get(CONFIG_OPT_BASE + key, defaultValue);
}

Point
ConfigManager::get(const std::string &key, const Point &defaultValue) {
    Point pt;
    pt.x = m_configInfo.get(CONFIG_OPT_BASE + key + ".x", defaultValue.x);
    pt.y = m_configInfo.get(CONFIG_OPT_BASE + key + ".y", defaultValue.y);
    pt.z = m_configInfo.get(CONFIG_OPT_BASE + key + ".z", defaultValue.z);
    return pt;
}

Rect
ConfigManager::get(const std::string &key, const Rect &defaultValue) {
    Rect rc;
    rc.x = m_configInfo.get(CONFIG_OPT_BASE + key + ".x", defaultValue.x);
    rc.y = m_configInfo.get(CONFIG_OPT_BASE + key + ".y", defaultValue.y);
    rc.w = m_configInfo.get(CONFIG_OPT_BASE + key + ".w", defaultValue.w);
    rc.h = m_configInfo.get(CONFIG_OPT_BASE + key + ".h", defaultValue.h);
    return rc;
}

Box
ConfigManager::get(const std::string &key, const Box &defaultValue) {
    Box bx;
    bx.x = m_configInfo.get(CONFIG_OPT_BASE + key + ".x", defaultValue.x);
    bx.y = m_configInfo.get(CONFIG_OPT_BASE + key + ".y", defaultValue.y);
    bx.z = m_configInfo.get(CONFIG_OPT_BASE + key + ".z", defaultValue.z);
    bx.w = m_configInfo.get(CONFIG_OPT_BASE + key + ".w", defaultValue.w);
    bx.h = m_configInfo.get(CONFIG_OPT_BASE + key + ".h", defaultValue.h);
    bx.l = m_configInfo.get(CONFIG_OPT_BASE + key + ".l", defaultValue.l);
    return bx;
}


void
ConfigManager::set(const std::string &key, const std::string &value) {
    m_configInfo.put(CONFIG_OPT_BASE + key, value);
}

void
ConfigManager::set(const std::string &key, bool value) {
    m_configInfo.put(CONFIG_OPT_BASE + key, value);
}

void
ConfigManager::set(const std::string &key, int value) {
    m_configInfo.put(CONFIG_OPT_BASE + key, value);
}

void
ConfigManager::set(const std::string &key, float value) {
    m_configInfo.put(CONFIG_OPT_BASE + key, value);
}

void
ConfigManager::set(const std::string &key, const Point &value) {
    m_configInfo.put(CONFIG_OPT_BASE + key + ".x", value.x);
    m_configInfo.put(CONFIG_OPT_BASE + key + ".y", value.y);
    m_configInfo.put(CONFIG_OPT_BASE + key + ".z", value.z);
}

void
ConfigManager::set(const std::string &key, const Rect &value) {
    m_configInfo.put(CONFIG_OPT_BASE + key + ".x", value.x);
    m_configInfo.put(CONFIG_OPT_BASE + key + ".y", value.y);
    m_configInfo.put(CONFIG_OPT_BASE + key + ".w", value.w);
    m_configInfo.put(CONFIG_OPT_BASE + key + ".h", value.h);
}

void
ConfigManager::set(const std::string &key, const Box &value) {
    m_configInfo.put(CONFIG_OPT_BASE + key + ".x", value.x);
    m_configInfo.put(CONFIG_OPT_BASE + key + ".y", value.y);
    m_configInfo.put(CONFIG_OPT_BASE + key + ".z", value.z);
    m_configInfo.put(CONFIG_OPT_BASE + key + ".w", value.w);
    m_configInfo.put(CONFIG_OPT_BASE + key + ".h", value.h);
    m_configInfo.put(CONFIG_OPT_BASE + key + ".l", value.l);
}
