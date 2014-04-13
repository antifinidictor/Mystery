/*
 * defs.h
 * Header file with standard definitions
 */

#ifndef DEFS_H
#define DEFS_H
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <vector>
#include <ostream>

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
enum Directions {
    NORTH,
    NORTHEAST,
    EAST,
    SOUTHEAST,
    SOUTH,
    SOUTHWEST,
    WEST,
    NORTHWEST,
    NUM_CARDINAL_DIRECTIONS,
    UP = NUM_CARDINAL_DIRECTIONS,
    DOWN,
    NUM_DIRECTIONS
};

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

#define TEXTURE_TILE_SIZE 32
#define WORLD_TILE_SIZE 1.f

//Macros
#define GET_FLAG(flags, flag)             ((flags >> flag) & 0x1)
#define SET_FLAG(flags, flag, val)   ((val) ? (flags | (1 << flag)) : (flags & ~(1 << flag)))

enum LetterKeyMasks {
    LKEY_A = 0x1,
    LKEY_B = 0x2,
    LKEY_C = 0x4,
    LKEY_D = 0x8,
    LKEY_E = 0x10,
    LKEY_F = 0x20,
    LKEY_G = 0x40,
    LKEY_H = 0x80,
    LKEY_I = 0x100,
    LKEY_J = 0x200,
    LKEY_K = 0x400,
    LKEY_L = 0x800,
    LKEY_M = 0x1000,
    LKEY_N = 0x2000,
    LKEY_O = 0x4000,
    LKEY_P = 0x8000,
    LKEY_Q = 0x10000,
    LKEY_R = 0x20000,
    LKEY_S = 0x40000,
    LKEY_T = 0x80000,
    LKEY_U = 0x100000,
    LKEY_V = 0x200000,
    LKEY_W = 0x400000,
    LKEY_X = 0x800000,
    LKEY_Y = 0x1000000,
    LKEY_Z = 0x2000000,
    LKEY_MAX = 0x3FFFFFF
};

//Identities for boolean and nonboolean inputs.  IDs are related to what the
// input does, not what triggers the input.  Maximum 8 bools (IN_NUM_BOOLS can
// be no more than 8); any values after that must be integers, and have their
// code manually added to the setInput function.
enum TypingKeyId {
    KIN_LETTER_PRESSED,
    KIN_NUMBER_PRESSED,
    KIN_NUM_EVENTS
};

enum MouseInputID {
	MIN_MOUSE_X = KIN_NUM_EVENTS,
	MIN_MOUSE_Y,
	MIN_MOUSE_REL_X,
	MIN_MOUSE_REL_Y,
    MIN_NUM_MOUSE_INPUTS
};

enum EventIdAllocs {
    MGE_EVENTS_BEGIN        = 0x00,
    AUDIO_EVENTS_BEGIN      = 0x20,
    WORLD_EVENTS_BEGIN      = 0x40,
    PHYSICS_EVENTS_BEGIN    = 0x60,
    RENDER_EVENTS_BEGIN     = 0x80,
    GAME_EVENTS_BEGIN       = 0x100
};

enum EventID {      //EventIDs: Add to the World object to listen to them.  Feel free to add to this list.
	ON_MOUSE_MOVE = MGE_EVENTS_BEGIN, //Called when mouse moves.  WARNING: Mouse position may not be accurate if object is not on screen!
	ON_BUTTON_INPUT,	        //Called on every registered key press or mouse click
    NUM_EVENT_IDS
};

/*
 * Each engine is allocated 4 flags.  The actual game may use the rest.
 */
enum ObjFlags {
    WORLD_FLAGS_BEGIN =   0x00,
    PHYSICS_FLAGS_BEGIN = 0x08,
    RENDER_FLAGS_BEGIN =  0x10,
    GAME_FLAGS_BEGIN =    0x80
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
    std::vector<bool> m_vBoolInputs;
    std::vector<bool> m_vChangedInputs;
	int m_iMouseX,
		m_iMouseY,
		m_iMouseRelX,
		m_iMouseRelY;
    bool m_bMouseHasMoved, m_bInputHasChanged;
    uint m_uiLetterKeyUp,
         m_uiLetterKeyDown; //Special info used for typing
    uint m_uiNumberKeyUp,
         m_uiNumberKeyDown;
public:
    InputData(uint size = 64);
    ~InputData();

    void clear();
    void clearChanged();
    int  getInputState(int flag);
    void setInputState(int flag, int val);
    bool hasChanged(int flag)   { return m_vChangedInputs[flag]; }
    bool mouseHasMoved()        { return m_bMouseHasMoved; }
    bool inputHasChanged()      { return m_bInputHasChanged; }
    void setLetter(uint letter, bool bDown);
    uint getLettersUp()   { return m_uiLetterKeyUp; }
    uint getLettersDown() { return m_uiLetterKeyDown; }
    void setNumber(uint letter, bool bDown);
    uint getNumbersUp()   { return m_uiNumberKeyUp; }
    uint getNumbersDown() { return m_uiNumberKeyDown; }
};

struct tRect;
struct tBox;

typedef struct Vec3f {
	float x;
	float y;
	float z;
	Vec3f() { x = 0; y = 0; z = 0; }
	Vec3f(float fx, float fy, float fz) { x = fx; y = fy; z = fz; }
	Vec3f(const Vec3f &pt) { x = pt.x; y = pt.y; z = pt.z; }
	Vec3f(const tRect &rc);
	Vec3f(const tBox &bx);

    // Constant operators
	bool operator==(const Vec3f &pt) const { return x == pt.x && y == pt.y && z == pt.z; }
	bool operator!=(const Vec3f &pt) const { return !(pt == *this); }
	bool operator<(const Vec3f &pt) const;
	Vec3f operator+(const Vec3f &pt) const{
        return Vec3f(x + pt.x, y + pt.y, z + pt.z);
	}
	Vec3f operator-(const Vec3f &pt) const{
        return Vec3f(x - pt.x, y - pt.y, z - pt.z);
	}
	Vec3f operator*(const float val) const{
	    return Vec3f(x * val, y * val, z * val);
	}
	Vec3f operator/(const float val) const{
	    return Vec3f(x / val, y / val, z / val);
	}
    float magnitude() const {
        return sqrt(this->x * this->x + this->y * this->y + this->z * this->z);
    }
    friend std::ostream& operator<< (std::ostream& stream, const Vec3f& v);

	//Nonconstant operators
	void operator*=(const float val) { x *= val; y *= val; z *= val; }
	void operator-=(const Vec3f &pt) { x -= pt.x; y -= pt.y; z -= pt.z; }
	void operator+=(const Vec3f &pt) { x += pt.x; y += pt.y; z += pt.z; }
	void operator=(const Vec3f &pt) { x = pt.x; y = pt.y; z = pt.z; }
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
	float w;
	float h;
	tRect() { x = y = 0.0F; w = h = 0; }
	tRect(float fx, float fy, float fw, float fh) { x = fx; y = fy; w = fw; h = fh; }
	operator tBox();
    friend std::ostream& operator<< (std::ostream& stream, const tRect& rc);
	void operator+=(const Vec3f &pt) { x += pt.x; y += pt.y; }
	void operator-=(const Vec3f &pt) { x -= pt.x; y -= pt.y; }
	void operator=(const tRect &rc) { x = rc.x; y = rc.y; w = rc.w; h = rc.h; }
} Rect, RC;

typedef struct tBox {
	float x;
	float y;
	float z;
	float w;
	float h;
	float l;
	tBox() { x = y = z = 0.0F; w = l = h = 0; }
	tBox(float fx, float fy, float fz, float fw, float fh, float fl) { x = fx; y = fy; z = fz; w = fw; l = fl; h = fh; }

    friend std::ostream& operator<< (std::ostream& stream, const tBox& bx);
	//Constant operators
	tBox operator+(const tBox &bx) const {  //positive translation
	    tBox res;
	    res.x = bx.x < x ? bx.x : x;
	    res.y = bx.y < y ? bx.y : y;
	    res.z = bx.z < z ? bx.z : z;
	    res.w = bx.x + bx.w > x + w ? bx.x + bx.w - res.x : x + w - res.x;
	    res.h = bx.y + bx.h > x + w ? bx.y + bx.h - res.y : y + h - res.y;
	    res.l = bx.z + bx.l > x + w ? bx.z + bx.l - res.z : z + l - res.z;
	    return res;
	}
	tBox operator+(const Vec3f &pt) const {  //positive translation
        return tBox(x + pt.x, y + pt.y, z + pt.z, w, h, l);
	}
	tBox operator-(const Vec3f &pt) const {  //negative translation
        return tBox(x - pt.x, y - pt.y, z - pt.z, w, h, l);
	}
	bool operator==(const tBox &bx) const {
	    return  x == bx.x && y == bx.y && z == bx.z &&
                w == bx.w && l == bx.l && h == bx.h;
	}
	bool operator==(const Vec3f &pt) const {
	    return  x == pt.x && y == pt.y && z == pt.z;
	}
	bool operator!=(const tBox &bx) const {
	    return !(*this == bx);
	}
	bool operator!=(const Vec3f &pt) const {
	    return !(*this == pt);
	}
	operator tRect() {
        return tRect(x,y,w,h);
    }

	//Nonconstant operators
	void operator+=(const tBox &bx) { *this = *this + bx; }   //Less efficient, but necessary
	void operator+=(const Vec3f &pt) { x += pt.x; y += pt.y; z += pt.z; }
	void operator-=(const Vec3f &pt) { x -= pt.x; y -= pt.y; z -= pt.z; }
	void operator=(const tBox &bx) { x = bx.x; y = bx.y; z = bx.z; w = bx.w; l = bx.l; h = bx.h; }
} Box, BX;


#define MAX_COLOR_VAL 0x00FFFFFF

typedef struct tColor {
    unsigned b : 8;
    unsigned g : 8;
    unsigned r : 8;

    tColor(unsigned int color) {
        *this = *((tColor*)&color);
    }
    tColor(unsigned char r, unsigned char g, unsigned char b) {
        this->r = r;
        this->g = g;
        this->b = b;
    }
    tColor() {
        b = g = r = 0;
    }
    uint toUint() {
        return *(uint*)(this) & MAX_COLOR_VAL;
    }
} Color, CR;

Color mix(int numColors, ...);

bool  rcIntersects(const RC &rc1, const RC &rc2);
char  rcOutOfBounds(const RC &rc, const RC &rcBounds);
char  ptOutOfBounds(const PT &pt, const RC &rcBounds);
bool  bxIntersects(const BX &bx1, const BX &bx2);
bool  bxIntersectsEq(const BX &bx1, const BX &bx2);
char  bxOutOfBounds(const BX &bx, const BX &bxBounds);
char  ptOutOfBounds(const PT &pt, const BX &bxBounds);
bool  ptInRect(const PT &pt, const RC &rc);
bool  ptInXZRect(const PT &pt, const BX &bx);
PT    bxCenter(const BX &bx);
Box   bxScaleAboutPt(const BX &bx, const PT &locus, const double scale);
PT    ptMid(const PT &A, const PT &B);
Box   bxIntersection(const BX &bx1, const BX &bx2);
Rect  rcIntersection(const Rect &rc1, const Rect &rc2);

double dist(const PT &ptHere, const PT &ptThere);
bool equal(PT &pt1, PT &pt2, float offset);
bool equal(float f1, float f2, float offset=0.001f);
float dot(PT &pt1, PT &pt2);
PT cross(const PT &pt1, const PT &pt2);
int order(const PT &pt1, const PT &pt2);    //FIXME: Obsolete
std::string bin2str(uint bin);
#endif
