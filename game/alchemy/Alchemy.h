/*
 * AlchemicalReactor
 * Performs alchemical processes on element lists
 */
#ifndef ALCHEMY_H
#define ALCHEMY_H
#include "mge/defs.h"
#include <map>

//No need to make a class for this; alchemy has no state
//namespace Alchemy {

enum BasicSubstanceType {
    SUB_EARTH,
    SUB_AIR,
    SUB_FIRE,
    SUB_WATER,
    NUM_SUBSTANCES
};

struct Substance {
    uint m_uiType;
    uint m_uiAmount;
};

struct Mixture {
    std::map<uint,uint> m_mSubstances;  //mapping of substance type to the amount present in mixture
    Color m_crColor;    //Mixture color: A weighted average of the total
};

typedef bool (*FuseFunc)(const Mixture *mix, Substance *sub);   //Returns true if function does something
typedef void (*PulveriseFunc)(const Substance *sub, Mixture *mix);

class Alchemy {
public:
    void mix(Mixture *mix, Substance *sub0, Substance *sub1, Substance *sub2);          //Mix three substances at a time
    void purify(Mixture *mix, Substance *sub0, Substance *sub1, Substance *sub2); //Purify three substances at a time
    bool fuse(const Mixture *mix, Substance *sub);      //Fuses a mixture into a new substance
    void pulverise(const Substance *sub, Mixture *mix); //Pulverizes a substance into a mixture of other substances

    void registerSubstance(uint uiSubType, FuseFunc fusor, PulveriseFunc pulverisor);
private:
    struct RegisteredSubstance {
        RegisteredSubstance() {
            fusor = NULL;
            pulverisor = NULL;
        }
        void operator=(const RegisteredSubstance& rsub) {
            this->fusor = rsub.fusor;
            this->pulverisor = rsub.pulverisor;
        }
        RegisteredSubstance(FuseFunc fusor, PulveriseFunc pulverisor) {
            this->fusor = fusor;
            this->pulverisor = pulverisor;
        }
        FuseFunc fusor;
        PulveriseFunc pulverisor;
    };
    std::map<uint,RegisteredSubstance> m_mRegisteredSubstances;
};

#endif //ALCHEMY_H
