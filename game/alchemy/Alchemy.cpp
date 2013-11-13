/*
 * Alchemy.cpp
 */
#include "Alchemy.h"

using namespace std;

void
Alchemy::mix(Mixture *mix, Substance *sub0, Substance *sub1, Substance *sub2) {
    map<uint,uint>::iterator iter;
    if(mix == NULL || sub0 == NULL || sub1 == NULL || sub2 == NULL) {
        return;
    }
    iter = mix->m_mSubstances.find(sub0->m_uiType);
    if(iter != mix->m_mSubstances.end()) {
        iter->second += sub0->m_uiAmount;
    } else {
        mix->m_mSubstances[sub0->m_uiType] = sub0->m_uiAmount;
    }

    iter = mix->m_mSubstances.find(sub1->m_uiType);
    if(iter != mix->m_mSubstances.end()) {
        iter->second += sub1->m_uiAmount;
    } else {
        mix->m_mSubstances[sub1->m_uiType] = sub1->m_uiAmount;
    }

    iter = mix->m_mSubstances.find(sub2->m_uiType);
    if(iter != mix->m_mSubstances.end()) {
        iter->second += sub2->m_uiAmount;
    } else {
        mix->m_mSubstances[sub2->m_uiType] = sub2->m_uiAmount;
    }
}

void
Alchemy::purify(Mixture *mix, Substance *sub0, Substance *sub1, Substance *sub2) {
    map<uint,uint>::iterator iter;
    if(mix == NULL || sub0 == NULL || sub1 == NULL || sub2 == NULL) {
        return;
    }
    iter = mix->m_mSubstances.find(sub0->m_uiType);
    if(iter != mix->m_mSubstances.end()) {
        sub0->m_uiAmount += iter->second;
    }

    iter = mix->m_mSubstances.find(sub1->m_uiType);
    if(iter != mix->m_mSubstances.end()) {
        sub1->m_uiAmount += iter->second;
    }

    iter = mix->m_mSubstances.find(sub2->m_uiType);
    if(iter != mix->m_mSubstances.end()) {
        sub2->m_uiAmount += iter->second;
    }
}

//The following methods are completely dependent on the substances involve
bool
Alchemy::fuse(const Mixture *mix, Substance *sub) {
    map<uint,RegisteredSubstance>::iterator iter;
    for(iter = m_mRegisteredSubstances.begin(); iter != m_mRegisteredSubstances.end(); ++iter) {
        //Look for a substance that successfully fuses the mixture
        if(iter->second.fusor(mix, sub)) {
            return true;
        }
    }
    return false;
}

void
Alchemy::pulverise(const Substance *sub, Mixture *mix) {
    map<uint,RegisteredSubstance>::iterator iter;
    iter = m_mRegisteredSubstances.find(sub->m_uiType);
    if(iter != m_mRegisteredSubstances.end()) {
        iter->second.pulverisor(sub, mix);
    }
}


void
Alchemy::registerSubstance(uint uiSubType, FuseFunc fusor, PulveriseFunc pulverisor) {
    m_mRegisteredSubstances[uiSubType] = RegisteredSubstance(fusor, pulverisor);
}
