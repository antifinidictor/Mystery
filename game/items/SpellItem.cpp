#include "SpellItem.h"
#include "game/spells/SourceSinkSpell.h"
#include "game/spells/FlowSpell.h"
#include "pwe/PartitionedWorldEngine.h"
#include "game/gui/DraggableHud.h"

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
    return new SpellItem(id, itemId, ptPos);
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
        break;
    case ITEM_SPELL_FLOW:
        return new SourceSinkSpell(duration, magnitude);
        break;
    }
    return NULL;
}
