/*
 * PixelMap.cpp
 */

#include "PixelMap.h"
#include <stdlib.h>

//copyPixel courtesy of http://sdl.beuc.net/sdl.wiki/Pixel_Access, wih some modifications
void copyPixel(SDL_Surface *surface, int x, int y, Color *cr)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
    Uint8 r = 0, g = 0, b = 0;
    switch(bpp) {
    case 1:
        SDL_GetRGB(*p, surface->format, &r, &g, &b);
        break;

    case 2:
        SDL_GetRGB(*(Uint16 *)p, surface->format, &r, &g, &b);
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            SDL_GetRGB(p[0] << 16 | p[1] << 8 | p[2], surface->format, &r, &g, &b);
        else
            SDL_GetRGB(p[0] | p[1] << 8 | p[2] << 16, surface->format, &r, &g, &b);
        break;

    case 4:
        SDL_GetRGB(*(Uint32 *)p, surface->format, &r, &g, &b);
        break;

    default:
        break;
    }
    cr->r = r;
    cr->g = g;
    cr->b = b;
}

//===============Constructor/Destructor for the PixelMap structure================

/*
 * Creates an OpenGL texture from the provided bitmap
 * Code thanks to http://gpwiki.org/index.php/SDL:Tutorials:Using_SDL_with_OpenGL
 * (variable names differ)
 */
PixelMap::PixelMap(const std::string &sFileName, uint uiId) {
    //Load image onto a surface
	SDL_Surface *pSurface = /*SDL_LoadBMP*/IMG_Load(sFileName.c_str()/*sFileName.c_str()*/);

	if( pSurface ) {
		// Check that the image's width is a power of 2
		if ( (pSurface->w & (pSurface->w - 1)) != 0 ) {
			std::cout << "warning: " << sFileName << "'s width is not a power of 2\n";
		}

		// Also check if the height is a power of 2
		if ( (pSurface->h & (pSurface->h - 1)) != 0 ) {
			std::cout << "warning: " << sFileName << "'s height is not a power of 2\n";
		}

		//Store the original width and height of the image
		this->m_uiW = pSurface->w;
		this->m_uiH = pSurface->h;

        // get the number of channels in the SDL surface
        /*
        int iNumColors = pSurface->format->BytesPerPixel;
        if (iNumColors == 4)     // contains an alpha channel
        {
            if (pSurface->format->Rmask == 0x000000ff)
                m_eTextureFormat = GL_RGBA;
            else
                m_eTextureFormat = GL_BGRA;
        } else if (iNumColors == 3)     // no alpha channel
        {
            if (pSurface->format->Rmask == 0x000000ff)
                m_eTextureFormat = GL_RGB;
            else
                m_eTextureFormat = GL_BGR;
        } else {
            printf("warning: the image is not truecolor..  this will probably break\n");
            // this error should not go unhandled
        }
        */

        //Allocate and fill out the pixel data
        m_pData = (float**)malloc(sizeof(float*) * m_uiW);
        for(uint x = 0; x < m_uiW; ++x) {
            m_pData[x] = (float*)malloc(sizeof(float) * m_uiH);
            for(uint y = 0; y < m_uiH; ++y) {
                //Fill each pixel
                Color cr;
                copyPixel(pSurface, x, y, &cr);
                m_pData[x][y] = (float)cr.toUint() / (float)MAX_COLOR_VAL;
            }
        }

	} else {
		std::cout << "SDL could not load " << sFileName << ": " << SDL_GetError();
		exit(-1);
	}

	// Free the SDL_pSurface only if it was successfully created
	if ( pSurface ) {
		SDL_FreeSurface( pSurface );
	}

	//Initialize other members
	m_sImageFileName = sFileName;
	m_uiId = uiId;
}

PixelMap::PixelMap(uint w, uint h, uint uiId) {
    m_uiW = w;
    m_uiH = h;

	m_sImageFileName = "";
	m_uiId = uiId;

    m_pData = (float**)malloc(sizeof(float*) * m_uiW);
    for(uint x = 0; x < m_uiW; ++x) {
        m_pData[x] = (float*)malloc(sizeof(float) * m_uiH);
        for(uint y = 0; y < m_uiH; ++y) {
            //Fill each pixel
            m_pData[x][y] = 0.f;
        }
    }
}

/*
 * Destructor:  Destroys the texture information.
 */
PixelMap::~PixelMap() {
    for(uint x = 0; x < m_uiW; ++x) {
        free(m_pData[x]);
    }
    free(m_pData);
}
