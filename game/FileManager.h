/*
 * FileManager
 * Manages reading and writing to a game file.  Knows about all objects.
 */

#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <fstream>
#include <map>
#include "mge/defs.h"

class Image;
class GameObject;

#define TILE_SIZE 32

enum ObjType {
  OT_PLAYER,
  OT_WALL,
  OT_DECORATIVE,
  OT_PHYSICS_SURFACE,
  OT_PHYSICS_OBJECT,
  OT_NUM_OBJ_TYPES
};

class FileManager {
public:
  FileManager() {}
  virtual ~FileManager() {}

  void read(const char *strFileName);
  Image *getImageRes(uint uiResID);
  GameObject *getObj(uint uiResID);
  std::fstream *getFileHandle() { return &m_fio; }

private:
  std::fstream m_fio;
  std::map<uint, Image*> m_mImageRes;
  std::map<uint, GameObject*> m_mObjs;
  GameObject *createObject(ObjType eType);
};

#endif

