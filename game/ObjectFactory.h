/*
 * ObjectFactory
 * This class is responsible for managing parse trees and reading in objects.
 */

#ifndef OBJECT_FACTORY_H
#define OBJECT_FACTORY_H

#include "game_defs.h"
#include <map>
#include <list>
#include <boost/property_tree/ptree.hpp>

class GameObject;

typedef GameObject* (*ReadFuncPtr)(const boost::property_tree::ptree &, const std::string &);


void registerClasses(); //Defined in registration.cpp

enum AttributeType {
    ATYPE_INT,
    ATYPE_UINT,
    ATYPE_FLOAT,
    ATYPE_POINT,
    ATYPE_RECT,
    ATYPE_BOX,
    ATYPE_COLOR,
    ATYPE_STRING,
    //Named types: Special classes of UINT
    ATYPE_RESOURCE_ID,      //Textures, etc
    ATYPE_OBJECT_ID,        //Requires a reference to an object

    ATYPE_NUM_TYPES
};

class ObjectFactory {
public:
    //These are visible and can be read by other objects
    struct AttributeInfo {
        AttributeInfo() {
            m_sAttributeName = "?";
            m_sAttributeKey = "?";
            m_eType = ATYPE_NUM_TYPES;
        }
        AttributeInfo(const std::string &name, const std::string &key, AttributeType eType) {
            m_sAttributeName = name;
            m_sAttributeKey = key;
            m_eType = eType;
        }
        std::string m_sAttributeName;
        std::string m_sAttributeKey;
        AttributeType m_eType;
    };

    class FactoryData {
    public:
        FactoryData &registerAttribute(const std::string &name, const std::string &key, AttributeType eType);
        FactoryData &setAttribute(const std::string &key, int value);
        FactoryData &setAttribute(const std::string &key, uint value);
        FactoryData &setAttribute(const std::string &key, float value);
        FactoryData &setAttribute(const std::string &key, const Point &value);
        FactoryData &setAttribute(const std::string &key, const Rect &value);
        FactoryData &setAttribute(const std::string &key, const Box &value);
        FactoryData &setAttribute(const std::string &key, const Color &value);
        FactoryData &setAttribute(const std::string &key, const std::string &value);

        std::string m_sClassName;
        std::list<AttributeInfo> m_lsAttributeInfo;

        //Used for object creation
        std::string m_sObjName;
        boost::property_tree::ptree m_pt;
    };

    static void init() { m_pInstance = new ObjectFactory(); }
    static ObjectFactory *get() { return m_pInstance; }
    static void clean() { delete m_pInstance; }

    //Registering classes: Probably do this in the game defs file
    FactoryData &registerClass(const std::string &className, ReadFuncPtr readFunc);   //returns true if the class was registered/had not been registered before
    void getClassList(std::list<const std::string *> &lsClasses);    //Populates the list with class names

    //Creating objects from scratch
    FactoryData &initObject(const std::string &className, const std::string &objName);
    GameObject *createFromAttributes();

    void initClass(const std::string &className);
    GameObject *createFromTree(const boost::property_tree::ptree &pt, const std::string &keyBase);

    //Reading files: Writes to world engine
    void read(const std::string &fileName);
    void write(const std::string &fileName);

private:
    //Data required for constructing an object
    struct ClassFactory {
        ReadFuncPtr readFunc;
        FactoryData data;
    };

    ObjectFactory();
    virtual ~ObjectFactory();

    static ObjectFactory *m_pInstance;

    std::map<std::string, ClassFactory> m_mRegisteredObjects;
    ClassFactory *m_pCurFactory;
};

#endif //OBJECT_FACTORY_H
