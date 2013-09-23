/*
 * TextRenderer
 */

#include "TextRenderer.h"
#include "mge/Image.h"
#include "game/game_defs.h"
#include "d3re/d3re.h"
using namespace std;

#define COLOR_SIZE 7

//Static members
TextRenderer *TextRenderer::m_pInstance = NULL;

//Constructor
TextRenderer::TextRenderer() {
/*  Old font
    #define SPACE_WIDTH 3.0f
                //A  B  C  D  E  F  G  H  I  J  K  L  M  N  O  P  Q  R  S  T  U  V  W  X  Y  Z
    char t[78] = {10,10,10,11,9, 8, 11,10,3, 3, 10,8, 12,10,11,9, 11,10,9, 10,10,10,14,10,10,10,
                //a  b  c  d  e  f  g  h  i  j  k  l  m  n  o  p  q  r  s  t  u  v  w  x  y  z
                  8, 9, 8, 9, 9, 5, 9, 9, 3, 3, 9, 3, 13,9, 9, 9, 9, 6, 8, 6, 9, 9, 12,8, 9, 8,
                //0  1  2  3  4  5  6  7  8  9  .  !  ?  ,  :  ;  (  )  "  '  -  _  <  >  /  %
                  9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 4, 4, 7, 4, 4, 4, 5, 5, 6, 4, 6, 8,11,11, 6,13};

*/
///*  New font
    #define SPACE_WIDTH 8.0f
                //A  B  C  D  E  F  G  H  I  J  K  L  M  N  O  P  Q  R  S  T  U  V  W  X  Y  Z
    char t[78] = {14,13,14,15,12,12,14,14,6, 6, 14,12,16,14,15,12,15,14,12,14,14,14,18,14,14,14,
                //a  b  c  d  e  f  g  h  i  j  k  l  m  n  o  p  q  r  s  t  u  v  w  x  y  z
                  10,12,10,11,12,8, 11,11,5, 5, 12,5, 17,11,12,12,11,9, 9, 8, 11,12,16,12,12,10,
                //0  1  2  3  4  5  6  7  8  9  .  !  ?  ,  :  ;  (  )  "  '  -  _  <  >  /  %
                  11,11,10,11,12,10,11,10,11,11,5, 6, 9, 5, 5, 5, 7, 6, 8, 5, 7, 7,14,14, 7,17};
//*/
    memcpy(m_aWidths,t,78);

    m_pFont = D3RE::get()->getImage(IMG_FONT);
}

TextRenderer::~TextRenderer() {
}

int TextRenderer::char2IndH(char c) {
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

int TextRenderer::char2IndW(char c) {
    if('a' <= c && c <= 'z') {
        return 1;
    } else if ('A' <= c && c <= 'Z') {
        return 0;
    } else {
        return 2;
    }
}

double TextRenderer::twMin(int iw) {
    return iw * 1.0F / m_pFont->m_iNumFramesW;
}

double TextRenderer::twMax(int iw, int ih) {
    return twMin(iw) + m_aWidths[iw * m_pFont->m_iNumFramesH + ih] / (double)m_pFont->w;
}

double TextRenderer::thMin(int ih) {
    return ih * 1.0F / m_pFont->m_iNumFramesH;
}

double TextRenderer::thMax(int ih) {
    return thMin(ih) + 1.0F / m_pFont->m_iNumFramesH;
}

void TextRenderer::render(const char *str, double x, double y) {
    double x_start = x;

    glBindTexture( GL_TEXTURE_2D, m_pFont->m_uiTexture );

    for(int i = 0; str[i] != '\0'; ++i) {
        if(str[i] == '\n') {
            y += m_pFont->h / m_pFont->m_iNumFramesH;
            x = x_start;
            continue;
        } else if(str[i] == '#') {
            i += setColor(str + i + 1);
            continue;
        }
        x += renderChar(str[i], x, y);
    }
    //Reset the color to default
    glColor3f(1.f, 1.f, 1.f);
}

int TextRenderer::setColor(const char *hexColor) {
    char *end;
    Color color = strtol(hexColor, &end, 16);
    glColor3f(color.r / 255.f, color.g / 255.f, color.b / 255.f);
    return end - hexColor + 1;
}

double TextRenderer::renderChar(char c, double x, double y) {
    int iw = char2IndW(c),
        ih = char2IndH(c);
    if(ih < 0) {
        return SPACE_WIDTH;   //Space & unsupported characters
    }
    double w = m_aWidths[iw * m_pFont->m_iNumFramesH + ih],
          h = m_pFont->h / m_pFont->m_iNumFramesH;

	glBegin( GL_QUADS );
		//Top-left vertex (corner)
		glTexCoord2f( twMin(iw), thMin(ih) );
		glVertex3f( x, y, 0.0f );

		//Top-right vertex (corner)
		glTexCoord2f( twMax(iw, ih), thMin(ih) );
        glVertex3f( x + w, y, 0.0f );

		//Bottom-right vertex (corner)
		glTexCoord2f( twMax(iw, ih), thMax(ih) );
		glVertex3f( x + w, y + h, 0.0f );

		//Bottom-left vertex (corner)
		glTexCoord2f( twMin(iw), thMax(ih) );
        glVertex3f( x, y + h, 0.0f );
	glEnd();
	return w;
}

char* TextRenderer::splitText(const char *str, double maxw) {
//#if 0
    double xw = 0;
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
        } else if(str[i] == '#') {
            i += COLOR_SIZE;
            continue;
        }

        int iw = char2IndW(str[i]),
            ih = char2IndH(str[i]);

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

void TextRenderer::splitText(string &str, double maxw) {
//#if 0
    double xw = 0;
    int lastSpace = 0;
    //char *out = (char*)SDL_calloc(strlen(str) * 2, sizeof(char));
    for(string::iterator curChar = str.begin(); curChar < str.end(); ++curChar) {
        if(*curChar == '\n') {
            lastSpace = 0;
            xw = 0;
            continue;
        } else if(*curChar == '#') {
            curChar += COLOR_SIZE;
            continue;
        }

        int iw = char2IndW(*curChar),
            ih = char2IndH(*curChar);
        xw += m_aWidths[iw * m_pFont->m_iNumFramesH + ih];

        if(xw > maxw) {
            if(lastSpace == 0) {    //word is too long
                curChar = str.insert(curChar, '\n');
                xw = 0;
                continue;
            } else {
                str[lastSpace] = '\n';
                curChar = str.begin() + lastSpace;
                xw = 0;
                lastSpace = 0;
                continue;
            }
        }
        if(ih < 0) {
            lastSpace = curChar - str.begin();
        }

    }
//#endif
}

Rect TextRenderer::getArea(const char *str, double x, double y) {
    const double h = m_pFont->h / m_pFont->m_iNumFramesH;

    double cur_w = 0.f,
          max_w = 0.f,
          max_h = h;

    for(int i = 0; str[i] != '\0'; ++i) {
        if(str[i] == '\n') {
            max_h += h;
            if(cur_w > max_w) max_w = cur_w;
            cur_w = 0;
            continue;
        } else if(str[i] == '#') {
            i += COLOR_SIZE;
            continue;
        }
        int iw = char2IndW(str[i]),
            ih = char2IndH(str[i]);
        cur_w += m_aWidths[iw * m_pFont->m_iNumFramesH + ih];
    }
    if(cur_w > max_w) max_w = cur_w;

    return Rect(x,y,max_w,max_h);
}

