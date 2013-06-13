/*
 * defs.h
 * Header file with standard definitions
 */

#ifndef DEFS_H
#define DEFS_H
#include <math.h>
#include <stdio.h>

typedef unsigned int uint;

const int SCREEN_HEIGHT = 480;
const int SCREEN_WIDTH = 640;

//Macros
#define BIT(bit) (1 << bit)
#define GET_BIT(val, bit)  ((val >> bit) & 1)
#define SET_BIT(val, bit)   (val |  BIT(bit))
#define UNSET_BIT(val, bit) (val & ~BIT(bit))
#define FLIP_BIT(val, bit)  (val ^  BIT(bit))

//Constants
#define NORTH   0
#define EAST    1
#define SOUTH   2
#define WEST    3
#define UP      4
#define DOWN    5

#define BIT_NORTH BIT(NORTH)
#define BIT_EAST  BIT(EAST)
#define BIT_SOUTH BIT(SOUTH)
#define BIT_WEST  BIT(WEST)
#define BIT_UP    BIT(UP)
#define BIT_DOWN  BIT(DOWN)

#define FIRST_DIR NORTH //First direction, if you want to loop through them
#define LAST_DIR DOWN   //Last direction, if you want to loop through them

#define MOUSE_MOVE_MASK     (MIN_NUM_MOUSE_INPUTS - 1)
#define BUTTON_INPUT_MASK   (~MOUSE_MOVE_MASK)

#define TRUE 1
#define FALSE 0

//Macros
#define GET_FLAG(flags, flag)             ((flags >> flag) & 0x1)
#define SET_FLAG(flags, flag, val)   ((val) ? (flags | (1 << flag)) : (flags & ~(1 << flag)))

//Identities for boolean and nonboolean inputs.  IDs are related to what the
// input does, not what triggers the input.  Maximum 8 bools (IN_NUM_BOOLS can
// be no more than 8); any values after that must be integers, and have their
// code manually added to the setInput function.
enum MouseInputID {
	MIN_MOUSE_X,
	MIN_MOUSE_Y,
	MIN_MOUSE_REL_X,
	MIN_MOUSE_REL_Y,
    MIN_NUM_MOUSE_INPUTS
};

enum EventID {      //EventIDs: Add to the World object to listen to them.  Feel free to add to this list.
	ON_MOUSE_MOVE,      //Called when mouse moves.  WARNING: Mouse position may not be accurate if object is not on screen!
	ON_BUTTON_INPUT,	//Called on every registered key press or mouse click
	ON_COLLISION,       //Called when the EventHandler registers a collision
    ON_SELECT,
    ON_DESELECT,
    ON_ACTIVATE,
    NUM_EVENT_IDS
};

/*
 * Each engine is allocated 4 flags.  The actual game may use the rest.
 */
enum ObjFlags {
    WORLD_FLAGS_BEGIN =   0x00,
    PHYSICS_FLAGS_BEGIN = 0x04,
    RENDER_FLAGS_BEGIN =  0x08,
    GAME_FLAGS_BEGIN =    0x10
};

/*
 * The first few IDs are reserved for each of the engines
 */
enum EngineIDs {
    ID_MODULAR_ENGINE,
    ID_WORLD_ENGINE,
    ID_PHYSICS_ENGINE,
    ID_RENDER_ENGINE,
    ID_AUDIO_ENGINE,
    ID_FIRST_UNUSED
};


class InputData {
private:
    uint m_uiBoolInputs,
         m_uiChangedInputs;
	int m_iMouseX,
		m_iMouseY,
		m_iMouseRelX,
		m_iMouseRelY;
public:
    void clear();
    void clearChanged()         { m_uiChangedInputs = 0; }
    int  getInputState(int flag);
    bool hasChanged(int flag)   { return GET_FLAG(m_uiChangedInputs, flag); }
    bool mouseHasMoved()        { return m_uiChangedInputs & MOUSE_MOVE_MASK; }
    bool inputHasChanged()      { return m_uiChangedInputs & BUTTON_INPUT_MASK; }
    void setInputState(int flag, int val);
};

struct tRect;
struct tBox;

typedef struct tPoint {
	float x;
	float y;
	float z;
	tPoint() { x = 0; y = 0; z = 0; }
	tPoint(float fx, float fy, float fz) { x = fx; y = fy; z = fz; }
	tPoint(const tPoint &pt) { x = pt.x; y = pt.y; z = pt.z; }
	tPoint(const tRect &rc);
	tPoint(const tBox &bx);

    // Constant operators
	bool operator==(const tPoint &pt) const { return x == pt.x && y == pt.y && z == pt.z; }
	bool operator!=(const tPoint &pt) const { return !(pt == *this); }
	bool operator<(const tPoint &pt) const;
	tPoint operator+(const tPoint &pt) const{
        return tPoint(x + pt.x, y + pt.y, z + pt.z);
	}
	tPoint operator-(const tPoint &pt) const{
        return tPoint(x - pt.x, y - pt.y, z - pt.z);
	}
	tPoint operator*(const float val) const{
	    return tPoint(x * val, y * val, z * val);
	}
	tPoint operator/(const float val) const{
	    return tPoint(x / val, y / val, z / val);
	}
    float magnitude() {
        return sqrt(this->x * this->x + this->y * this->y + this->z * this->z);
    }

	//Nonconstant operators
	void operator*=(const float val) { x *= val; y *= val; z *= val; }
	void operator-=(const tPoint &pt) { x -= pt.x; y -= pt.y; z -= pt.z; }
	void operator+=(const tPoint &pt) { x += pt.x; y += pt.y; z += pt.z; }
	void operator=(const tPoint &pt) { x = pt.x; y = pt.y; z = pt.z; }
	void normalize() {
	    float magnitude = this->magnitude();
        if(magnitude != 0) {
            *this *= 1 / magnitude;
        }
    }
} Point, PT;

typedef struct tRect {
	float x;
	float y;
	int w;
	int l;
	tRect() { x = y = 0.0F; w = l = 0; }
	tRect(float fx, float fy, int iw, int il) { x = fx; y = fy; w = iw; l = il; }
	operator tBox();
	void operator+=(const tPoint &pt) { x += pt.x; y += pt.y; }
	void operator-=(const tPoint &pt) { x -= pt.x; y -= pt.y; }
	void operator=(const tRect &rc) { x = rc.x; y = rc.y; w = rc.w; l = rc.l; }
} Rect, RC;

typedef struct tBox {
	float x;
	float y;
	float z;
	int w;
	int l;
	int h;
	tBox() { x = y = z = 0.0F; w = l = h = 0; }
	tBox(float fx, float fy, float fz, int iw, int il, int ih) { x = fx; y = fy; z = fz; w = iw; l = il; h = ih; }

	//Constant operators
	tBox operator+(const tPoint &pt) const {  //positive translation
        return tBox(x + pt.x, y + pt.y, z + pt.z, w, l, h);
	}
	tBox operator-(const tPoint &pt) const {  //negative translation
        return tBox(x - pt.x, y - pt.y, z - pt.z, w, l, h);
	}
	bool operator==(const tBox &bx) const {
	    return  x == bx.x && y == bx.y && z == bx.z &&
                w == bx.w && l == bx.l && h == bx.h;
	}
	bool operator==(const tPoint &pt) const {
	    return  x == pt.x && y == pt.y && z == pt.z;
	}
	bool operator!=(const tBox &bx) const {
	    return !(*this == bx);
	}
	bool operator!=(const tPoint &pt) const {
	    return !(*this == pt);
	}
	operator tRect() {
        return tRect(x,y,w,l);
    }

	//Nonconstant operators
	void operator+=(const tPoint &pt) { x += pt.x; y += pt.y; z += pt.z; }
	void operator-=(const tPoint &pt) { x -= pt.x; y -= pt.y; z -= pt.z; }
	void operator=(const tBox &bx) { x = bx.x; y = bx.y; z = bx.z; w = bx.w; l = bx.l; h = bx.h; }
} Box, BX;

bool  rcIntersects(const RC &rc1, const RC &rc2);
char  rcOutOfBounds(const RC &rc, const RC &rcBounds);
char  ptOutOfBounds(const PT &pt, const RC &rcBounds);
bool  bxIntersects(const BX &bx1, const BX &bx2);
char  bxOutOfBounds(const BX &bx, const BX &bxBounds);
char  ptOutOfBounds(const PT &pt, const BX &bxBounds);
bool  ptInRect(const PT &pt, const RC &rc);
PT    bxCenter(const BX &bx);
Box   bxScaleAboutPt(const BX &bx, const PT &locus, const double scale);
PT    ptMid(const PT &A, const PT &B);
Box   bxIntersection(const BX &bx1, const BX &bx2);
Rect  rcIntersection(const Rect &rc1, const Rect &rc2);

double getDist(PT &ptHere, PT &ptThere);
bool equal(PT &pt1, PT &pt2, float offset);
float dot(PT &pt1, PT &pt2);
int order(const PT &pt1, const PT &pt2);    //FIXME: Obsolete

#endif
