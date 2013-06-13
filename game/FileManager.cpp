/*
 * FileManager.cpp
 */

#include "FileManager.h"
#include "mge/Image.h"
#include "mge/GameObject.h"

//Engines
#include "ore/OrderedRenderEngine.h"

//Known objects
#include "game/Wall.h"
#include "game/Player.h"

using namespace std;

GameObject *FileManager::createObject(ObjType eType) {
    switch(eType) {
    case OT_PLAYER:
        return Player::read(this);
    case OT_WALL:
        return Wall::read(this);
    case OT_DECORATIVE:
        //return Decorative::read(this);
    case OT_PHYSICS_SURFACE:
        //return PhysicsSurface::read(this);
    case OT_PHYSICS_OBJECT:
        //return PhysicsObject::read(this);
    default:
        return NULL;
    }
}

void FileManager::read(const char *sFileName) {
    //Open the file
    m_fio.open(sFileName, ios::binary | ios::in);
    if(!m_fio.good()) {
      return;
    }

    //First, read in the number of images
    int numImages;
    m_fio.read((char*)(&numImages), sizeof(int));

    //Then map the image ids to generated ids
    //Order:
    // {image ID} {frames wide} {frames high} {filename len} {filename}
    uint uiID, uiStrLen;
    int iFramesW, iFramesH;
    char strBuffer[256];
    for(int i = 0; i < numImages; ++i) {
      //Read in data
      m_fio.read((char*)(&uiID), sizeof(uint));
      m_fio.read((char*)(&iFramesW), sizeof(uint));
      m_fio.read((char*)(&iFramesH), sizeof(uint));
      m_fio.read((char*)(&uiStrLen), sizeof(uint));
      m_fio.read((char*)(&strBuffer), sizeof(char) * uiStrLen);

      //Create the image
      m_mImageRes[uiID] =
        ORE::get()->createImage(strBuffer, iFramesH, iFramesW);
    }

    //Read in audio file data

    //Read in actual game objects
    ObjType eObjType;
    while(m_fio.good()) {
        m_fio.read((char*)(&eObjType), sizeof(ObjType));
        m_fio.read((char*)(&uiID), sizeof(uint));
        m_mObjs[uiID] = createObject(eObjType);
    }

    //Clean up
    m_fio.close();
}

Image *FileManager::getImageRes(uint uiResID) {
    map<uint, Image*>::iterator img = m_mImageRes.find(uiResID);
    if(img != m_mImageRes.end()) {
        return img->second;
    }
    return NULL;
}

GameObject *FileManager::getObj(uint uiResID) {
    map<uint, GameObject*>::iterator obj = m_mObjs.find(uiResID);
    if(obj != m_mObjs.end()) {
        return obj->second;
    }
    return NULL;
}

