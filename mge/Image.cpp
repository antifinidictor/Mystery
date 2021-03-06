/*
 * New home for the Image class
 */
#include <stdlib.h>
#include "Image.h"
/*
 * Creates an OpenGL texture from the provided bitmap
 * Code thanks to http://gpwiki.org/index.php/SDL:Tutorials:Using_SDL_with_OpenGL
 * (variable names differ)
 */
Image::Image(const std::string &sFileName, uint uiID, int iNumFramesH, int iNumFramesW, bool bLinearInterp) {
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
		if(bLinearInterp) {
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        } else {
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        }

		// Edit the texture object's image data using the information SDL_pSurface gives us
		glTexImage2D( GL_TEXTURE_2D, 0, m_iNumColors, pSurface->w, pSurface->h, 0,
						  m_eTextureFormat, GL_UNSIGNED_BYTE, pSurface->pixels );
	} else {
		std::cout << "SDL could not load " << sFileName << ": " << SDL_GetError();
		exit(-1);
	}

	// Free the SDL_pSurface only if it was successfully created
	if ( pSurface ) {
		SDL_FreeSurface( pSurface );
	}

	//Initialize other members
	m_iNumFramesH = iNumFramesH;
	m_iNumFramesW = iNumFramesW;
	m_sImageFileName = sFileName;
	m_uiID = uiID;
}

/*
 * Destructor:  Destroys the texture information.
 * /
Image::~Image() {
	glDeleteTextures( 1, &m_uiTexture );
}
*/
