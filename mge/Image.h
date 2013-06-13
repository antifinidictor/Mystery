/*
 * Image.h
 * Header file for the Image structure
 * (Images are still loaded the same across all render models, so Image remains
 * in STD)
 */

#ifndef IMAGE_H
#define IMAGE_H

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_opengl.h"
#include "mge/defs.h"
#include <string>
#include <iostream>

struct Image {
public:
	//Constructor/Destructor
//	Image(std::string &sFileName, char cID, int iNumFramesH = 1, int iNumFramesW = 1);
	Image(const char *szFileName, uint uiID, int iNumFramesH = 1, int iNumFramesW = 1);
	virtual ~Image();
	//Members
	std::string m_sImageFileName;
	GLuint m_uiTexture;
	GLenum m_eTextureFormat;
	GLint  m_iNumColors;
	int w;
	int h;
	int m_iNumFramesH,
		 m_iNumFramesW;
	uint m_uiID;
};


//===============Constructor/Destructor for the Image structure================

/*
 * Creates an OpenGL texture from the provided bitmap
 * Code thanks to http://gpwiki.org/index.php/SDL:Tutorials:Using_SDL_with_OpenGL
 * (variable names differ)
 */
inline Image::Image(const char *szFileName, uint uiID, int iNumFramesH, int iNumFramesW) {
    //Load image onto a surface
	SDL_Surface *pSurface = /*SDL_LoadBMP*/IMG_Load(szFileName/*sFileName.c_str()*/);

	if( pSurface ) {
		// Check that the image's width is a power of 2
		if ( (pSurface->w & (pSurface->w - 1)) != 0 ) {
			std::cout << "warning: " << szFileName << "'s width is not a power of 2\n";
		}

		// Also check if the height is a power of 2
		if ( (pSurface->h & (pSurface->h - 1)) != 0 ) {
			std::cout << "warning: " << szFileName << "'s height is not a power of 2\n";
		}

		//Store the original width and height of the image
		this->w = pSurface->w;
		this->h = pSurface->h;

			// get the number of channels in the SDL surface
			m_iNumColors = pSurface->format->BytesPerPixel;
			if (m_iNumColors == 4)     // contains an alpha channel
			{
					if (pSurface->format->Rmask == 0x000000ff)
							m_eTextureFormat = GL_RGBA;
					else
							m_eTextureFormat = GL_BGRA;
			} else if (m_iNumColors == 3)     // no alpha channel
			{
					if (pSurface->format->Rmask == 0x000000ff)
							m_eTextureFormat = GL_RGB;
					else
							m_eTextureFormat = GL_BGR;
			} else {
					printf("warning: the image is not truecolor..  this will probably break\n");
					// this error should not go unhandled
			}

		// Have OpenGL generate a texture object handle for us
		glGenTextures( 1, &m_uiTexture );

		// Bind the texture object
		glBindTexture( GL_TEXTURE_2D, m_uiTexture );

		// Set the texture's stretching properties
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

		// Edit the texture object's image data using the information SDL_pSurface gives us
		glTexImage2D( GL_TEXTURE_2D, 0, m_iNumColors, pSurface->w, pSurface->h, 0,
						  m_eTextureFormat, GL_UNSIGNED_BYTE, pSurface->pixels );
	} else {
		std::cout << "SDL could not load " << szFileName << ": " << SDL_GetError();
		exit(-1);
	}

	// Free the SDL_pSurface only if it was successfully created
	if ( pSurface ) {
		SDL_FreeSurface( pSurface );
	}

	//Initialize other members
	m_iNumFramesH = iNumFramesH;
	m_iNumFramesW = iNumFramesW;
	m_sImageFileName = szFileName;
	m_uiID = uiID;
}

/*
 * Destructor:  Destroys the texture information.
 */
inline Image::~Image() {
	glDeleteTextures( 1, &m_uiTexture );
}

#endif
