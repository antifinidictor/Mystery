/*
 * ObjectFactory.cpp
 */

#include "ObjectFactory.h"
#include "d3re/d3re.h"
#include "pwe/PartitionedWorldEngine.h"
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/info_parser.hpp>
using namespace std;

ObjectFactory *ObjectFactory::m_pInstance;

ObjectFactory::ObjectFactory() {
    m_pCurFactory = NULL;
}

ObjectFactory::~ObjectFactory() {
}

ObjectFactory::FactoryData &
ObjectFactory::registerClass(const std::string &className, ReadFuncPtr readFunc) {
    m_mRegisteredObjects[className] = ClassFactory();
    ClassFactory *cf = &m_mRegisteredObjects[className];
    cf->readFunc = readFunc;
    cf->data.m_sClassName = className;
    return cf->data;
}

ObjectFactory::FactoryData &
ObjectFactory::initObject(const std::string &className, const std::string &objName) {
    initClass(className);
    m_pCurFactory->data.m_sObjName = objName;
    return m_pCurFactory->data;
}


void
ObjectFactory::initClass(const std::string &className) {
    map<string, ClassFactory>::iterator iter = m_mRegisteredObjects.find(className);
    if(iter == m_mRegisteredObjects.end()) {
        string err = "Error: Class " + className + " not registered";
        printf(err.c_str());
        throw err;
    }
    m_pCurFactory = &m_mRegisteredObjects[className];
}

GameObject *
ObjectFactory::createFromAttributes() {
//printf(__FILE__" %d %s\n", __LINE__, m_pCurFactory->data.m_sClassName.c_str());
    GameObject *obj = m_pCurFactory->readFunc(m_pCurFactory->data.m_pt, m_pCurFactory->data.m_sObjName);
    m_pCurFactory->data.m_pt.clear();
    return obj;
}


GameObject *
ObjectFactory::createFromTree(const boost::property_tree::ptree &pt, const std::string &keyBase) {
//printf(__FILE__" %d %s\n", __LINE__, m_pCurFactory->data.m_sClassName.c_str());
    return m_pCurFactory->readFunc(pt, keyBase);
}

void
ObjectFactory::getClassList(vector<const std::string *> &vClasses) {
    map<std::string, ClassFactory>::iterator iter;
    for(iter = m_mRegisteredObjects.begin(); iter != m_mRegisteredObjects.end(); ++iter) {
        vClasses.push_back(&iter->second.data.m_sClassName);
    }
}

void
ObjectFactory::read(const std::string &fileName) {
    using boost::property_tree::ptree;
    ptree pt;
    uint fileExtIndex = fileName.find_last_of(".");
    if(fileName.substr(fileExtIndex) == ".info") {
        read_info(fileName, pt);
    } else {
        read_xml(fileName, pt);
    }

    //Read resources
    D3RE::get()->read(pt, "resources");

    //Read areas
    PWE::get()->read(pt, "areas");

    //Read files
    try {
    BOOST_FOREACH(ptree::value_type &v, pt.get_child("files")) {
        read(v.first.data());
    }
    } catch(exception e) {
        printf("Could not read files\n");
    }
}


void
ObjectFactory::write(const std::string &fileName) {
    using boost::property_tree::ptree;
    ptree pt;

    D3RE::get()->write(pt, "resources");

    //Read areas
    PWE::get()->write(pt, "areas");

    uint fileExtIndex = fileName.find_last_of(".");
    if(fileName.substr(fileExtIndex) == ".info") {
        write_info(fileName, pt);
    } else {
        write_xml(fileName, pt);
    }
}

/*
 * FactoryData
 */
ObjectFactory::FactoryData &
ObjectFactory::FactoryData::registerAttribute(const std::string &name, const std::string &key, AttributeType eType) {
    m_lsAttributeInfo.push_back(AttributeInfo(name, key, eType));
    return *this;
}


ObjectFactory::FactoryData &
ObjectFactory::FactoryData::setAttribute(const std::string &key, int value) {
    m_pt.put(m_sObjName + "." + key, value);
    return *this;
}

ObjectFactory::FactoryData &
ObjectFactory::FactoryData::setAttribute(const std::string &key, uint value) {
    m_pt.put(m_sObjName + "." + key, value);
    return *this;
}

ObjectFactory::FactoryData &
ObjectFactory::FactoryData::setAttribute(const std::string &key, float value) {
    m_pt.put(m_sObjName + "." + key, value);
    return *this;
}

ObjectFactory::FactoryData &
ObjectFactory::FactoryData::setAttribute(const std::string &key, const Point &value) {
    setAttribute(key + ".x", value.x);
    setAttribute(key + ".y", value.y);
    setAttribute(key + ".z", value.z);
    return *this;
}

ObjectFactory::FactoryData &
ObjectFactory::FactoryData::setAttribute(const std::string &key, const Rect &value) {
    setAttribute(key + ".x", value.x);
    setAttribute(key + ".y", value.y);
    setAttribute(key + ".w", value.w);
    setAttribute(key + ".h", value.h);
    return *this;
}

ObjectFactory::FactoryData &
ObjectFactory::FactoryData::setAttribute(const std::string &key, const Box &value) {
    setAttribute(key + ".x", value.x);
    setAttribute(key + ".y", value.y);
    setAttribute(key + ".z", value.z);
    setAttribute(key + ".w", value.w);
    setAttribute(key + ".h", value.h);
    setAttribute(key + ".l", value.l);
    return *this;
}

ObjectFactory::FactoryData &
ObjectFactory::FactoryData::setAttribute(const std::string &key, const Color &value) {
    setAttribute(key + ".r", value.r);
    setAttribute(key + ".g", value.g);
    setAttribute(key + ".b", value.b);
    return *this;
}
ObjectFactory::FactoryData &
ObjectFactory::FactoryData::setAttribute(const std::string &key, const std::string &value) {
    m_pt.put(m_sObjName + "." + key, value);
    return *this;
}

int
ObjectFactory::FactoryData::getAttribute(const std::string &key, int defaultValue) {
    return m_pt.get(m_sObjName + "." + key, defaultValue);
}

uint
ObjectFactory::FactoryData::getAttribute(const std::string &key, uint defaultValue) {
    return m_pt.get(m_sObjName + "." + key, defaultValue);
}

float
ObjectFactory::FactoryData::getAttribute(const std::string &key, float defaultValue) {
    return m_pt.get(m_sObjName + "." + key, defaultValue);
}

const Point
ObjectFactory::FactoryData::getAttribute(const std::string &key, const Point &defaultValue) {
    Point value = Point(
        getAttribute(key + ".x", defaultValue.x),
        getAttribute(key + ".y", defaultValue.y),
        getAttribute(key + ".z", defaultValue.z)
    );
    return value;
}

const Rect
ObjectFactory::FactoryData::getAttribute(const std::string &key, const Rect &defaultValue) {
    Rect value = Rect(
        getAttribute(key + ".x", defaultValue.x),
        getAttribute(key + ".y", defaultValue.y),
        getAttribute(key + ".w", defaultValue.w),
        getAttribute(key + ".h", defaultValue.h)
    );
    return value;
}

const Box
ObjectFactory::FactoryData::getAttribute(const std::string &key, const Box &defaultValue) {
    Box value = Box(
        getAttribute(key + ".x", defaultValue.x),
        getAttribute(key + ".y", defaultValue.y),
        getAttribute(key + ".z", defaultValue.z),
        getAttribute(key + ".w", defaultValue.w),
        getAttribute(key + ".h", defaultValue.h),
        getAttribute(key + ".l", defaultValue.l)
    );
    return value;
}

const Color
ObjectFactory::FactoryData::getAttribute(const std::string &key, const Color &defaultValue) {
    Color value = Color(
        getAttribute(key + ".r", defaultValue.r),
        getAttribute(key + ".g", defaultValue.g),
        getAttribute(key + ".b", defaultValue.b)
    );
    return value;
}

const std::string
ObjectFactory::FactoryData::getAttribute(const std::string &key, const std::string &defaultValue) {
    return m_pt.get(m_sObjName + "." + key, defaultValue);
}

