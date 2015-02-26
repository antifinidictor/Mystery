#include "SpeechBubble.h"
using namespace std;

#define BUBBLE_WIDTH 128
#define BUBBLE_HEIGHT 64

SpeechBubble::SpeechBubble(const string &msg, const Point &initPos)
    : D3HudRenderModel(
        D3RE::get()->getImageId("speechBubble"),
        Rect(256, 128, BUBBLE_WIDTH, BUBBLE_HEIGHT),
        msg,
        Point(3, 3, 0),
        0.8f
    )
{
    updatePosition(initPos);
}

SpeechBubble::~SpeechBubble()
{
    //dtor
}

void
SpeechBubble::updatePosition(const Point &pos) {
    Point screenPos = D3RE::get()->projectPoint(pos);
    Point prevPos = getPosition();
    Point shift = Point(
        screenPos.x - prevPos.x,
        screenPos.y - prevPos.y - BUBBLE_HEIGHT,
        0.f
    );
    moveBy(shift);

    printf("(%f,%f,%f) -> (%f,%f,%f)\n", pos.x, pos.y, pos.z, shift.x, shift.y, shift.y);
}
