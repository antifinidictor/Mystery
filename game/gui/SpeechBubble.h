#ifndef SPEECHBUBBLE_H
#define SPEECHBUBBLE_H

#include "mge/defs.h"
#include "d3re/d3re.h"
#include <string>

class SpeechBubble : public D3HudRenderModel
{
public:
    SpeechBubble(const std::string &msg, const Point &initPos);
    virtual ~SpeechBubble();

    void updatePosition(const Point &pos);
protected:
private:
};

#endif // SPEECHBUBBLE_H
