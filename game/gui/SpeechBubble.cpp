#include "SpeechBubble.h"
using namespace std;

#define BUBBLE_WIDTH 128
#define BUBBLE_HEIGHT 64
#define BUBBLE_Y_OFFSET (BUBBLE_HEIGHT)
#define ANIM_COUNTER_MAX 1
#define WAIT_BEFORE_CLOSING_MAX 20
#define MAX_ANIM_FRAME 3

static float s_fMaxCharSize = 0.75f;
static float s_fCharGrowthDelay = 5.f;
//static float s_fLineGrowthDelay = 2.f;
static float s_fTextSpeed = 0.3f;
static int s_iMaxLines = 3.f;

/*
 * SpeechBubble
 */
SpeechBubble::SpeechBubble(uint id, const string &msg, const Point &initPos)
    : D3HudRenderModel(
        D3RE::get()->getImageId("speechBubble"),
        Rect(256, 128, BUBBLE_WIDTH, BUBBLE_HEIGHT),
        msg,
        Point(3, 3, 0),
        new TextRenderer::BasicCharacterFilter(s_fMaxCharSize)
    ),
      m_uiId(id),
      m_uiStateTimer(0),
      m_uiCurFrame(0),
      m_eState(SB_OPENING_ANIM)
{
    update(initPos);
    s_iMaxLines = BUBBLE_HEIGHT / (TextRenderer::get()->getLineHeight(s_fMaxCharSize) + 1);
    updateFilter(new ScrollingCharacterFilter(s_fMaxCharSize, msg.size()));
}

SpeechBubble::~SpeechBubble()
{
    //dtor
}

bool
SpeechBubble::update(const Point &pos) {
    Point screenPos = D3RE::get()->projectPoint(pos);
    screenPos.y = SCREEN_HEIGHT - screenPos.y - BUBBLE_Y_OFFSET;
    Point prevPos = getPosition();
    Point shift = Point(
        screenPos.x - prevPos.x,
        screenPos.y - prevPos.y,
        0.f
    );
    moveBy(shift);

    switch(m_eState) {
    case SB_OPENING_ANIM: {
        if(++m_uiStateTimer > ANIM_COUNTER_MAX) {
            m_uiStateTimer = 0;
            ++m_uiCurFrame;
            setFrameH(m_uiCurFrame);
            if(m_uiCurFrame == MAX_ANIM_FRAME) {
                m_eState = SB_SCROLLING_TEXT;
            }
        }
        break;
      }
    case SB_SCROLLING_TEXT: {
        //Test scrolling
        ScrollingCharacterFilter *filter = (ScrollingCharacterFilter*)getFilter();
        filter->m_fCurPos += s_fTextSpeed;

        if(filter->m_bIsDone) {
            m_eState = SB_WAITING_TEXT;
        }
        break;
      }
    case SB_WAITING_TEXT: {
        if(++m_uiStateTimer > WAIT_BEFORE_CLOSING_MAX) {
            ScrollingCharacterFilter *filter = (ScrollingCharacterFilter*)getFilter();
            filter->m_fCurPos = 0.f;
            m_eState = SB_CLOSING_ANIM;
        }
        break;
      }
    case SB_CLOSING_ANIM: {
        if(++m_uiStateTimer > ANIM_COUNTER_MAX) {
            m_uiStateTimer = 0;
            --m_uiCurFrame;
            setFrameH(m_uiCurFrame);
            if(m_uiCurFrame == 0) {
                m_eState = SB_DONE;
            }
        }
        break;
      }
    default:    //SB_DONE
        return false;    //Delete this speech bubble
    }

    return true;    //Updated successfully, don't delete
}


/*
 * Scrolling character filter
 */
SpeechBubble::ScrollingCharacterFilter::ScrollingCharacterFilter(float fMaxSize, uint length)
    :   m_fCurPos(0.f),
        m_fMaxSize(fMaxSize),
        m_iCurLine(0),
        m_uiLastChar(length - 1),
        m_bIsDone(false)
{
}

SpeechBubble::ScrollingCharacterFilter::~ScrollingCharacterFilter() {
}

float
SpeechBubble::ScrollingCharacterFilter::getSize(int index, int adjustedIndex, int line) {
    float diffIx = (m_fCurPos - adjustedIndex) / s_fCharGrowthDelay;
    int diffLine = (m_iCurLine - line);
    float charSize;
    if(diffIx < 0.f) {
        charSize = 0.f;

    } else if(diffIx > m_fMaxSize) {
        charSize = m_fMaxSize;

    } else {
        charSize = m_fMaxSize * diffIx;
        if(line > m_iCurLine) {
            m_iCurLine = line;
        }
    }

    if(diffLine >= s_iMaxLines) {
        charSize = 0.f;
    }

    m_bIsDone = m_bIsDone || ((index >= m_uiLastChar) && (charSize >= m_fMaxSize));

    //TODO: Control character size by line, so that older lines seem to shrink.
    return charSize;
}

float
SpeechBubble::ScrollingCharacterFilter::getOffsetSize(int index, int adjustedIndex, int line) {
    int diffLine = (m_iCurLine - line);
    if(diffLine >= s_iMaxLines) {
        return 0.f;
//    } else if(diffLine == s_iMaxLines - 1) {
//        return 0.5f * m_fMaxSize;
    } else {
        return m_fMaxSize;
    }
}
