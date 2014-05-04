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
	Image(const std::string &sFileName, uint uiID, int iNumFramesH = 1, int iNumFramesW = 1, bool bLinearInterp = false);
	virtual ~Image()  {
        glDeleteTextures( 1, &m_uiTexture );
    }
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

#endif
