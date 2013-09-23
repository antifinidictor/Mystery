/*
 * defs.cpp
 * Source file for basic function definitions
 */

#include "defs.h"

/* Static members */

/* Functions from structs and classes */

//InputData
void InputData::clear() {
    m_uiBoolInputs = m_uiChangedInputs = 0;
    m_iMouseX = m_iMouseY = m_iMouseRelX = m_iMouseRelY = 0;
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
        return GET_FLAG(m_uiBoolInputs, flag);
    }
}

void InputData::setInputState(int flag, int val) {
    switch(flag) {
	case MIN_MOUSE_X:
        m_iMouseX = val;
        break;
	case MIN_MOUSE_Y:
        m_iMouseY = val;
        break;
	case MIN_MOUSE_REL_X:
        m_iMouseRelX = val;
        break;
	case MIN_MOUSE_REL_Y:
        m_iMouseRelY = val;
        break;
    default:
        m_uiBoolInputs = SET_FLAG(m_uiBoolInputs, flag, val);
        break;
    }
    m_uiChangedInputs = SET_FLAG(m_uiChangedInputs, flag, TRUE);
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

//<: Ordering operator.  If pt1 < pt2, pt1 should be rendered before pt2.
bool Vec3f::operator<(const Point &pt) const {
    return  order(*this, pt) < 0;
}

//Box
tRect::operator tBox() {
    return tBox(x, y, 0, w, l, 0);
}

/* Function definitions from STD_Defs.h */
/*
 * rcIntersection()
 * Checks to to see if the two rectangles intersect.
 */
bool rcIntersects( const Rect &rc1, const Rect &rc2 ) {
	return	rc1.x < rc2.x + rc2.w &&
			rc2.x < rc1.x + rc1.w &&
			rc1.y < rc2.y + rc2.l &&
			rc2.y < rc1.y + rc1.l;
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
			((rc.y + rc.l > rcBounds.y + rcBounds.l) << SOUTH) |    //South
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
			((pt.y > rcBounds.y + rcBounds.l) << SOUTH) |   //South
			((pt.x < rcBounds.x) << WEST);                  //West
}

/*
 * bxIntersection()
 * Checks to to see if the two boxes intersect.
 */
bool bxIntersects(const Box &bx1, const Box &bx2) {
    return  bx1.x <= bx2.x + bx2.w &&
			bx2.x <= bx1.x + bx1.w &&
			bx1.y <= bx2.y + bx2.l &&
			bx2.y <= bx1.y + bx1.l &&
			bx1.z <= bx2.z + bx2.h &&
			bx2.z <= bx1.z + bx1.h;
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
            ((bx.y + bx.l > bxBounds.y + bxBounds.l) << SOUTH) |
            ((bx.x < bxBounds.x) << WEST) |
            ((bx.z + bx.h > bxBounds.z + bxBounds.h) << UP) |
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
            ((pt.y > bxBounds.y + bxBounds.l) << SOUTH) |
            ((pt.x < bxBounds.x) << WEST) |
            ((pt.z > bxBounds.z + bxBounds.h) << UP) |
            ((pt.z < bxBounds.z) << DOWN);
}


/*
 * ptInRect()
 * Returns true if the provided point is inside the provided rectangle.
 */
bool ptInRect(const Point &pt, const Rect &rc) {
	return	pt.x >= rc.x &&
			pt.x <= rc.x + rc.w &&
			pt.y >= rc.y &&
			pt.y <= rc.y + rc.l;
}


/*
 * bxCenter()
 * Returns the center point of the bounding box/rectangle.
 */
Point bxCenter(const Box &bx) {
    return Point(bx.x + bx.w / 2, bx.y + bx.l / 2, bx.z + bx.h / 2);
}

/*
 * bxScaleAboutLocus()
 * Scales the provided box about the provided locus.
 */
Box bxScaleAboutPt(const Box &bx, const Point &locus, const double scale) {
    Box sbx = bx;
    sbx.w *= scale;
    sbx.l *= scale;
    sbx.h *= scale;
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
    ret.l = bx1.y + bx1.l > bx2.y + bx2.l ? bx1.y + bx1.l - ret.y : bx2.y + bx2.l - ret.y;
    ret.h = bx1.z + bx1.h > bx2.z + bx2.h ? bx1.z + bx1.h - ret.z : bx2.z + bx2.h - ret.z;
    return ret;
}

Rect rcIntersection(const Rect &rc1, const Rect &rc2) {
    Box ret;
    ret.x = rc1.x > rc2.x ? rc1.x : rc2.x;
    ret.y = rc1.y > rc2.y ? rc1.y : rc2.y;
    ret.w = rc1.x + rc1.w > rc2.x + rc2.w ? rc1.x + rc1.w - ret.x : rc2.x + rc2.w - ret.x;
    ret.l = rc1.y + rc1.l > rc2.y + rc2.l ? rc1.y + rc1.l - ret.y : rc2.y + rc2.l - ret.y;
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


double getDist(PT &ptHere, PT &ptThere) {
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













#if 0

/*
 * STD_TextRenderer
 */

//Static members
STD_TextRenderer *STD_TextRenderer::m_pInstance = NULL;

//Constructor
STD_TextRenderer::STD_TextRenderer() {
    m_pFont = new STD_Image("res/font.png",0,26,3);


                //A  B  C  D  E  F  G  H  I  J  K  L  M  N  O  P  Q  R  S  T  U  V  W  X  Y  Z
    char t[78] = {10,10,10,11,9, 8, 11,10,3, 3, 10,8, 12,10,11,9, 11,10,9, 10,10,10,14,10,10,10,
                //a  b  c  d  e  f  g  h  i  j  k  l  m  n  o  p  q  r  s  t  u  v  w  x  y  z
                  8, 9, 8, 9, 9, 5, 9, 9, 3, 3, 9, 3, 13,9, 9, 9, 9, 6, 8, 6, 9, 9, 12,8, 9, 8,
                //0  1  2  3  4  5  6  7  8  9  .  !  ?  ,  :  ;  (  )  "  '  -  _  <  >  /  %
                  9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 4, 4, 7, 4, 4, 4, 5, 5, 6, 4, 6, 8,11,11, 6,13};
    memcpy(m_aWidths,t,78);

}

STD_TextRenderer::~STD_TextRenderer() {
    delete m_pFont;
}

int STD_TextRenderer::Char2IndH(char c) {
    if('a' <= c && c <= 'z') {
        return c - 'a';
    } else if('A' <= c && c <= 'Z') {
        return c - 'A';
    } else if('0' <= c && c <= '9') {
        return c - '0';
    } else {
        switch(c) {
        case '.':
            return 10;
        case '!':
            return 11;
        case '?':
            return 12;
        case ',':
            return 13;
        case ':':
            return 14;
        case ';':
            return 15;
        case '(':
            return 16;
        case ')':
            return 17;
        case '"':
            return 18;
        case '\'':
            return 19;
        case '-':
            return 20;
        case '_':
            return 21;
        case '<':
            return 22;
        case '>':
            return 23;
        case '/':
            return 24;
        case '%':
            return 25;
        default: //space
            return -1;
        }
    }
}

int STD_TextRenderer::Char2IndW(char c) {
    if('a' <= c && c <= 'z') {
        return 1;
    } else if ('A' <= c && c <= 'Z') {
        return 0;
    } else {
        return 2;
    }
}

float STD_TextRenderer::TWMin(int iw) {
    return iw * 1.0F / m_pFont->m_iNumFramesW;
}

float STD_TextRenderer::TWMax(int iw, int ih) {
    return TWMin(iw) + m_aWidths[iw * m_pFont->m_iNumFramesH + ih] / (float)m_pFont->w;
}

float STD_TextRenderer::THMin(int ih) {
    return ih * 1.0F / m_pFont->m_iNumFramesH;
}

float STD_TextRenderer::THMax(int ih) {
    return THMin(ih) + 1.0F / m_pFont->m_iNumFramesH;
}

void STD_TextRenderer::Render(const char *str, float x, float y) {
    float x_start = x;
    for(int i = 0; str[i] != '\0'; ++i) {
        if(str[i] == '\n') {
            y += m_pFont->h / m_pFont->m_iNumFramesH;
            x = x_start;
        }
        x += RenderChar(str[i], x, y);
    }
}

float STD_TextRenderer::RenderChar(char c, float x, float y) {
    int iw = Char2IndW(c),
        ih = Char2IndH(c);

    if(ih < 0) {
        return 3.0F;   //Space & unsupported characters
    }

    float w = m_aWidths[iw * m_pFont->m_iNumFramesH + ih],
          h = m_pFont->h / m_pFont->m_iNumFramesH;

    glBindTexture( GL_TEXTURE_2D, m_pFont->m_uiTexture );

	glBegin( GL_QUADS );
		//Top-left vertex (corner)
		glTexCoord2f( TWMin(iw), THMin(ih) );
		glVertex3f( x, y, 0.0f );

		//Top-right vertex (corner)
		glTexCoord2f( TWMax(iw, ih), THMin(ih) );
        glVertex3f( x + w, y, 0.0f );

		//Bottom-right vertex (corner)
		glTexCoord2f( TWMax(iw, ih), THMax(ih) );
		glVertex3f( x + w, y + h, 0.0f );

		//Bottom-left vertex (corner)
		glTexCoord2f( TWMin(iw), THMax(ih) );
        glVertex3f( x, y + h, 0.0f );
	glEnd();

	return w;
}

char* STD_TextRenderer::SplitText(const char *str, float maxw) {
//#if 0
    float xw = 0;
    int lastSpace = 0;
    int oi = 0;
    char *out = (char*)SDL_calloc(strlen(str) * 2, sizeof(char));
    for(int i = 0; str[i] != '\0'; ++i) {
        if(str[i] == '\n') {
            lastSpace = 0;
            out[oi] = str[i];
            oi++;
            xw = 0;
            continue;
        }
        int iw = Char2IndW(str[i]),
            ih = Char2IndH(str[i]);
        xw += m_aWidths[iw * m_pFont->m_iNumFramesH + ih];
        if(xw > maxw) {
            if(lastSpace == 0) {
                out[oi] = '\n';
                ++oi;
                --i;
                xw = 0;
                continue;
            } else {
                oi -= i - lastSpace;
                i = lastSpace;
                out[oi] = '\n';
                oi++;
                xw = 0;
                continue;
            }
        }
        if(ih < 0) {
            lastSpace = i;
        }
        out[oi] = str[i];
        oi++;
    }
    out[oi] = '\0';
    return out;
//#endif
}

#endif
