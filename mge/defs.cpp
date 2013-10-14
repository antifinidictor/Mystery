/*
 * defs.cpp
 * Source file for basic function definitions
 */

#include "defs.h"

using namespace std;

/* Static members */

/* Functions from structs and classes */

//InputData
InputData::InputData(uint size) {
    for(uint i = 0; i < size; ++i) {
        m_vBoolInputs.push_back(false);
        m_vChangedInputs.push_back(false);
    }
}

InputData::~InputData() {
    m_vBoolInputs.clear();
    m_vChangedInputs.clear();
}

void InputData::clear() {
    for(uint i = 0; i < m_vBoolInputs.size(); ++i) {
        m_vBoolInputs[i] = false;
        m_vChangedInputs[i] = false;
    }
    m_iMouseX = m_iMouseY = m_iMouseRelX = m_iMouseRelY = 0;
    m_bMouseHasMoved = m_bInputHasChanged = false;
    m_uiLetterKeyUp = m_uiLetterKeyDown = 0;
}

void InputData::clearChanged() {
    vector<bool>::iterator it;
    for(it = m_vChangedInputs.begin(); it < m_vChangedInputs.end(); ++it) {
        *it = false;
    }
    m_bMouseHasMoved = m_bInputHasChanged = false;
    m_uiLetterKeyUp = m_uiLetterKeyDown = 0;
    m_uiNumberKeyUp = m_uiNumberKeyDown = 0;
}

int InputData::getInputState(int flag) {
    switch(flag) {
	case MIN_MOUSE_X:
        return m_iMouseX;
	case MIN_MOUSE_Y:
        return m_iMouseY;
	case MIN_MOUSE_REL_X:
        return m_iMouseRelX;
	case MIN_MOUSE_REL_Y:
        return m_iMouseRelY;
    default:
        return m_vBoolInputs[flag];
    }
}

void InputData::setInputState(int flag, int val) {
    switch(flag) {
	case MIN_MOUSE_X:
        m_bMouseHasMoved = true;
        m_iMouseX = val;
        break;
	case MIN_MOUSE_Y:
        m_bMouseHasMoved = true;
        m_iMouseY = val;
        break;
	case MIN_MOUSE_REL_X:
        m_bMouseHasMoved = true;
        m_iMouseRelX = val;
        break;
	case MIN_MOUSE_REL_Y:
        m_bMouseHasMoved = true;
        m_iMouseRelY = val;
        break;
    default:
        m_bInputHasChanged = true;
        m_vBoolInputs[flag] = (bool)val;
        break;
    }
    m_vChangedInputs[flag] = true;
}

void InputData::setLetter(uint letter, bool bDown) {
    if(bDown) {
        m_uiLetterKeyDown = SET_FLAG(m_uiLetterKeyDown, letter, true);
    } else {
        m_uiLetterKeyUp = SET_FLAG(m_uiLetterKeyUp, letter, true);
    }
}

void InputData::setNumber(uint number, bool bDown) {
    if(bDown) {
        m_uiNumberKeyDown = SET_FLAG(m_uiNumberKeyDown, number, true);
    } else {
        m_uiNumberKeyUp = SET_FLAG(m_uiNumberKeyUp, number, true);
    }
}

//Point
Vec3f::Vec3f(const tRect &rc) {
    x = rc.x;
    y = rc.y;
    z = 0;
}

Vec3f::Vec3f(const tBox &bx) {
    x = bx.x;
    y = bx.y;
    z = bx.z;
}

std::ostream&
operator <<(std::ostream& stream, const Vec3f& v) {
    stream << "(" << v.x << "," << v.y << "," << v.z << ")";
    return stream;
}

//<: Ordering operator.  If pt1 < pt2, pt1 should be rendered before pt2.
bool Vec3f::operator<(const Point &pt) const {
    return  order(*this, pt) < 0;
}

//Box
tRect::operator tBox() {
    return tBox(x, y, 0, w, h, 0);
}


std::ostream&
operator <<(std::ostream& stream, const Rect& rc) {
    stream << "(" << rc.x << "," << rc.y << ":" << rc.w << "," << rc.h << ")";
    return stream;
}


std::ostream&
operator <<(std::ostream& stream, const Box& bx) {
    stream << "(" << bx.x << "," << bx.y << "," << bx.z << ":" << bx.w << "," << bx.h << "," << bx.l << ")";
    return stream;
}

/* Function definitions from STD_Defs.h */
/*
 * rcIntersection()
 * Checks to to see if the two rectangles intersect.
 */
bool rcIntersects( const Rect &rc1, const Rect &rc2 ) {
	return	rc1.x < rc2.x + rc2.w &&
			rc2.x < rc1.x + rc1.w &&
			rc1.y < rc2.y + rc2.h &&
			rc2.y < rc1.y + rc1.h;
}

/*
 * rcOutOfBounds()
 * Returns true if any part of the first rectangle is outside of the second
 * (boundary) rectangle.  You can use the output to tell which boundaries the
 * rectangle has crossed.
 */
char rcOutOfBounds(const Rect &rc, const Rect &rcBounds) {
	return	((rc.y < rcBounds.y) << NORTH) |                        //North
			((rc.x + rc.w > rcBounds.x + rcBounds.w) << EAST) |     //East
			((rc.y + rc.h > rcBounds.y + rcBounds.h) << SOUTH) |    //South
			((rc.x < rcBounds.x) << WEST);                          //West
}

/*
 * ptOutOfBounds()
 * Returns true if any part of the point is outside of the boundary rectangle.
 * You can use the output to tell which boundaries the point has crossed.
 */
char ptOutOfBounds(const PT &pt, const RC &rcBounds) {
	return	((pt.y < rcBounds.y) << NORTH) |                //North
			((pt.x > rcBounds.x + rcBounds.w) << EAST) |    //East
			((pt.y > rcBounds.y + rcBounds.h) << SOUTH) |   //South
			((pt.x < rcBounds.x) << WEST);                  //West
}

/*
 * bxIntersection()
 * Checks to to see if the two boxes intersect.
 */
bool bxIntersects(const Box &bx1, const Box &bx2) {
    return  bx1.x <= bx2.x + bx2.w &&
			bx2.x <= bx1.x + bx1.w &&
			bx1.y <= bx2.y + bx2.h &&
			bx2.y <= bx1.y + bx1.h &&
			bx1.z <= bx2.z + bx2.l &&
			bx2.z <= bx1.z + bx1.l;
}

/*
 * bxOutOfBounds()
 * Returns true if any part of the first box is outside of the second
 * (boundary) box.  You can use the output to tell which boundaries the
 * box has crossed.
 */
char bxOutOfBounds(const Box &bx, const Box &bxBounds) {
    return  ((bx.y < bxBounds.y) << NORTH) |
            ((bx.x + bx.w > bxBounds.x + bxBounds.w) << EAST) |
            ((bx.y + bx.h > bxBounds.y + bxBounds.h) << SOUTH) |
            ((bx.x < bxBounds.x) << WEST) |
            ((bx.z + bx.l > bxBounds.z + bxBounds.l) << UP) |
            ((bx.z < bxBounds.z) << DOWN);
}

/*
 * ptOutOfBounds()
 * Returns true if any part of the point is outside of the boundary box.
 * You can use the output to tell which boundaries the point has crossed.
 */
char ptOutOfBounds(const Point &pt, const Box &bxBounds) {
    return  ((pt.y < bxBounds.y) << NORTH) |
            ((pt.x > bxBounds.x + bxBounds.w) << EAST) |
            ((pt.y > bxBounds.y + bxBounds.h) << SOUTH) |
            ((pt.x < bxBounds.x) << WEST) |
            ((pt.z > bxBounds.z + bxBounds.l) << UP) |
            ((pt.z < bxBounds.z) << DOWN);
}


bool ptInXZRect(const PT &pt, const BX &bx) {
	return	pt.x >= bx.x &&
			pt.x <= bx.x + bx.w &&
			pt.z >= bx.z &&
			pt.z <= bx.z + bx.l;
}

/*
 * ptInRect()
 * Returns true if the provided point is inside the provided rectangle.
 */
bool ptInRect(const Point &pt, const Rect &rc) {
	return	pt.x >= rc.x &&
			pt.x <= rc.x + rc.w &&
			pt.y >= rc.y &&
			pt.y <= rc.y + rc.h;
}


/*
 * bxCenter()
 * Returns the center point of the bounding box/rectangle.
 */
Point bxCenter(const Box &bx) {
    return Point(bx.x + bx.w / 2, bx.y + bx.h / 2, bx.z + bx.l / 2);
}

/*
 * bxScaleAboutLocus()
 * Scales the provided box about the provided locus.
 */
Box bxScaleAboutPt(const Box &bx, const Point &locus, const double scale) {
    Box sbx = bx;
    sbx.w *= scale;
    sbx.h *= scale;
    sbx.l *= scale;
    //Translate the box to the new coordinates
    return sbx + (bxCenter(bx) - locus) * scale + locus - bxCenter(sbx);
}


/*
 * ptMid()
 * Returns the midpoint between two points.
 */
Point ptMid(const Point &A, const Point &B) {
    return A + (B - A) / 2;
}

Box bxIntersection(const Box &bx1, const Box &bx2) {
    Box ret;
    ret.x = bx1.x > bx2.x ? bx1.x : bx2.x;
    ret.y = bx1.y > bx2.y ? bx1.y : bx2.y;
    ret.z = bx1.z > bx2.z ? bx1.z : bx2.z;
    ret.w = bx1.x + bx1.w > bx2.x + bx2.w ? bx1.x + bx1.w - ret.x : bx2.x + bx2.w - ret.x;
    ret.h = bx1.y + bx1.h > bx2.y + bx2.h ? bx1.y + bx1.h - ret.y : bx2.y + bx2.h - ret.y;
    ret.l = bx1.z + bx1.l > bx2.z + bx2.l ? bx1.z + bx1.l - ret.z : bx2.z + bx2.l - ret.z;
    return ret;
}

Rect rcIntersection(const Rect &rc1, const Rect &rc2) {
    Box ret;
    ret.x = rc1.x > rc2.x ? rc1.x : rc2.x;
    ret.y = rc1.y > rc2.y ? rc1.y : rc2.y;
    ret.w = rc1.x + rc1.w > rc2.x + rc2.w ? rc1.x + rc1.w - ret.x : rc2.x + rc2.w - ret.x;
    ret.h = rc1.y + rc1.h > rc2.y + rc2.h ? rc1.y + rc1.h - ret.y : rc2.y + rc2.h - ret.y;
    return ret;
}


/*
 * order()
 * Returns -1 if pt1 < pt2, 0 if pt1 = pt2, 1 if pt1 > pt2
 * Screen is ordered such that pt1 < pt2 < pt3 < ... < ptN
 */
int order(const PT &pt1, const PT &pt2) {
	if( (pt1.z < pt2.z) ||
		(pt1.z == pt2.z && pt1.y < pt2.y) ||
		(pt1.z == pt2.z && pt1.y == pt2.y && pt1.x < pt2.x) ) {
		return -1;
	} else if( (pt1.z > pt2.z) ||
		(pt1.z == pt2.z && pt1.y > pt2.y) ||
		(pt1.z == pt2.z && pt1.y == pt2.y && pt1.x > pt2.x) ) {
		return 1;
	} else {// if( pt1.z == pt2.z && pt1.y == pt2.y && pt1.z == pt2.z ) {
		return 0;
	}
}


double dist(const PT &ptHere, const PT &ptThere) {
	return sqrt((ptThere.x - ptHere.x) * (ptThere.x - ptHere.x) +
				(ptThere.y - ptHere.y) * (ptThere.y - ptHere.y) +
				(ptThere.z - ptHere.z) * (ptThere.z - ptHere.z));
}

bool equal(PT &pt1, PT &pt2, float offset) {
	return	pt1.x - offset <= pt2.x && pt1.x + offset >= pt2.x &&
			pt1.y - offset <= pt2.y && pt1.y + offset >= pt2.y &&
			pt1.z - offset <= pt2.z && pt1.z + offset >= pt2.z;
}

float dot(PT &pt1, PT &pt2) {
	return pt1.x * pt2.x + pt1.y * pt2.y + pt1.z * pt2.z;
}

PT cross(const PT &pt1, const PT &pt2) {
    Point pt;
    pt.x = pt1.y * pt2.z - pt1.z * pt2.y;
    pt.y = pt1.z * pt2.x - pt1.x * pt2.z;
    pt.z = pt1.x * pt2.y - pt1.y * pt2.x;
    return pt;
}

Color mix(int numColors, ...) {
    va_list vl;
    va_start(vl, numColors);
    long r = 0, g = 0, b = 0;
    for(int i = 0; i < numColors; ++i) {
        Color *cr = va_arg(vl, Color*);
        r += cr->r;
        g += cr->g;
        b += cr->b;
    }
    return Color(r / numColors, g / numColors, b / numColors);
}


