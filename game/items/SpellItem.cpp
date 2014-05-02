#include "SpellItem.h"
#include "game/spells/SourceSinkSpell.h"
#include "game/spells/FlowSpell.h"
#include "game/spells/VortexSpell.h"
#include "pwe/PartitionedWorldEngine.h"
#include "game/gui/DraggableHud.h"

using namespace std;

SpellItem::SpellItem(uint id, uint itemId, const Point &pos)
    : Item(id, itemId, pos)
{
}

SpellItem::~SpellItem()
{
}

GameObject*
SpellItem::read(const boost::property_tree::ptree &pt, const std::string &keyBase) {
    uint id = PWE::get()->reserveId(pt.get(keyBase + ".id", 0));
    uint itemId = pt.get(keyBase + ".itemId", 0);
    Point ptPos;
    ptPos.x = pt.get(keyBase + ".pos.x", 0.f);
    ptPos.y = pt.get(keyBase + ".pos.y", 0.f);
    ptPos.z = pt.get(keyBase + ".pos.z", 0.f);
    string sInfo = pt.get(keyBase + ".info", "no information available");
    printf("Info = '%s'/", sInfo.c_str());
    SpellItem *item = new SpellItem(id, itemId, ptPos);
    item->setItemInfo(sInfo);
    printf("'%s'\n", item->getItemInfo().c_str());
    return item;
}

void
SpellItem::write(boost::property_tree::ptree &pt, const std::string &keyBase) {
    Item::write(pt, keyBase);
}

Spell *
SpellItem::createSpell(int duration, float magnitude) {
    switch(getItemId()) {
    case ITEM_SPELL_CYCLIC:
        return new FlowSpell(duration, magnitude);
    case ITEM_SPELL_FLOW:
        return new SourceSinkSpell(duration, magnitude);
    case ITEM_SPELL_VORTEX:
        return new VortexSpell(duration, magnitude);
    default:
        break;
    }
    return NULL;
}
