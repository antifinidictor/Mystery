#ifndef SPELLITEM_H
#define SPELLITEM_H

#include "game/items/Item.h"
#include "game/spells/Spell.h"

class SpellItem : public Item
{
public:
    SpellItem(uint id, uint itemId, const Point &pos);
    virtual ~SpellItem();

    static GameObject* read(const boost::property_tree::ptree &pt, const std::string &keyBase);
    virtual void write(boost::property_tree::ptree &pt, const std::string &keyBase);

    virtual const std::string getClass()        { return getClassName(); }
    static const std::string getClassName()     { return "SpellItem"; }

    Spell *createSpell(int duration, float magnitude);
private:
};

#endif // SPELLITEM_H
