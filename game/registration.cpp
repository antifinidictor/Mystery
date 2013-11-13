/*
 * registration.cpp
 * File defining a single function in which all classes and default resources should be registered
 */

#include "game/ObjectFactory.h"
#include "game/Player.h"
#include "game/world/SimplePhysicsObject.h"
#include "game/world/Wall.h"
#include "game/world/AreaLinkObject.h"
#include "game/world/Water.h"
#include "game/world/SimpleResettableObject.h"
#include "game/spells/WaterElementalVolume.h"
#include "game/items/Item.h"

void registerClasses() {
    ObjectFactory *fac = ObjectFactory::get();
    fac->registerClass(Player::getClassName(), Player::read)
        .registerAttribute("ID", "id", ATYPE_OBJECT_ID)
        .registerAttribute("Position", "pos", ATYPE_POINT)
    ;

    fac->registerClass(SimpleResettableObject::getClassName(), SimpleResettableObject::read)
        .registerAttribute("ID", "id", ATYPE_OBJECT_ID)
        .registerAttribute("Texture", "tex", ATYPE_RESOURCE_ID)
        .registerAttribute("Volume", "vol", ATYPE_BOX)
        .registerAttribute("Color", "cr", ATYPE_COLOR)
        .registerAttribute("Density", "density", ATYPE_FLOAT)
    ;


    fac->registerClass(SimplePhysicsObject::getClassName(), SimplePhysicsObject::read)
        .registerAttribute("ID", "id", ATYPE_OBJECT_ID)
        .registerAttribute("Texture", "tex", ATYPE_RESOURCE_ID)
        .registerAttribute("Volume", "vol", ATYPE_BOX)
        .registerAttribute("Color", "cr", ATYPE_COLOR)
        .registerAttribute("Density", "density", ATYPE_FLOAT)
    ;

    fac->registerClass(Water::getClassName(), Water::read)
        .registerAttribute("ID", "id", ATYPE_OBJECT_ID)
        .registerAttribute("Texture", "tex", ATYPE_RESOURCE_ID)
        .registerAttribute("Volume", "vol", ATYPE_BOX)
        .registerAttribute("Color", "cr", ATYPE_COLOR)
        .registerAttribute("Density", "density", ATYPE_FLOAT)
    ;

    fac->registerClass(WaterElementalVolume::getClassName(), WaterElementalVolume::read)
        .registerAttribute("ID", "id", ATYPE_OBJECT_ID)
        .registerAttribute("Texture", "tex", ATYPE_RESOURCE_ID)
        .registerAttribute("Volume", "vol", ATYPE_BOX)
        .registerAttribute("Color", "cr", ATYPE_COLOR)
        .registerAttribute("Density", "density", ATYPE_FLOAT)
    ;

    fac->registerClass(Wall::getClassName(), Wall::read)
        .registerAttribute("ID", "id", ATYPE_OBJECT_ID)
        .registerAttribute("North Texture", "tex.north", ATYPE_RESOURCE_ID)
        .registerAttribute("South Texture", "tex.south", ATYPE_RESOURCE_ID)
        .registerAttribute("East Texture",  "tex.east",  ATYPE_RESOURCE_ID)
        .registerAttribute("West Texture",  "tex.west",  ATYPE_RESOURCE_ID)
        .registerAttribute("Top Texture",   "tex.up",    ATYPE_RESOURCE_ID)
        .registerAttribute("Bottom Texture", "tex.down", ATYPE_RESOURCE_ID)
        .registerAttribute("Volume", "vol", ATYPE_BOX)
        .registerAttribute("Color", "cr", ATYPE_COLOR)
    ;

    fac->registerClass(AreaLinkObject::getClassName(), AreaLinkObject::read)
        .registerAttribute("ID", "id", ATYPE_OBJECT_ID)
        .registerAttribute("Dest Area ID", "destAreaId", ATYPE_OBJECT_ID)
        .registerAttribute("Dest Position", "dest", ATYPE_POINT)
        .registerAttribute("Trigger Volume", "vol", ATYPE_BOX)
    ;

    fac->registerClass(Item::getClassName(), Item::read)
        .registerAttribute("ID", "id", ATYPE_OBJECT_ID)
        .registerAttribute("Item ID", "itemId", ATYPE_UINT)
        .registerAttribute("Position", "pos", ATYPE_POINT)
    ;
}
