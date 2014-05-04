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
    //Read either a .info or .xml file
    uint fileExtIndex = sConfigFile.find_last_of(".");
    if(sConfigFile.substr(fileExtIndex) == ".info") {
        read_info(sConfigFile, m_configInfo);
    } else {
        read_xml(sConfigFile, m_configInfo);
    }

    using boost::property_tree::ptree;
    using boost::lexical_cast;
    using boost::bad_lexical_cast;

    //Special configuration options should be dealt with: In this case, user-specified keys
    try {
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
        printf("Could not read config options: %s\n", e.what());
    }
}

std::string
ConfigManager::get(const std::string &key, const std::string &defaultValue) {
    return m_configInfo.get(CONFIG_OPT_BASE + key, defaultValue);
}

bool
ConfigManager::get(const std::string &key, const bool &defaultValue) {
    return m_configInfo.get(CONFIG_OPT_BASE + key, defaultValue);
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

