/*
 * Source file for the sprite render model
 */

#include "d3re.h"
#include "mge/GameObject.h"

D3SpriteRenderModel::D3SpriteRenderModel(GameObject *parent, uint uiImageId, Rect rcArea) {

    m_uiImageId = uiImageId;
    m_rcDrawArea = rcArea;

    m_iFrameW = 0;
    m_iFrameH = 0;
    m_iRepsW = 1;
    m_iRepsH = 1;

    m_crColor = Color(0xFF, 0xFF, 0xFF);

    m_pParent = parent;
}

D3SpriteRenderModel::~D3SpriteRenderModel() {
}

void
D3SpriteRenderModel::render(RenderEngine *re) {
    glPushMatrix();

    //Render collision box
    if(D3RE::get()->getDrawCollisions()) {
        D3RE::get()->drawBox(m_pParent->getPhysicsModel()->getCollisionVolume(), m_crColor);
    }

    Color worldColor = D3RE::get()->getWorldColor();
    float fWeight = D3RE::get()->getColorWeight();
    Color ourColor = Color(m_crColor.r * (1 - fWeight) + worldColor.r * fWeight,
                           m_crColor.g * (1 - fWeight) + worldColor.g * fWeight,
                           m_crColor.b * (1 - fWeight) + worldColor.b * fWeight);

    Image *pImage = D3RE::get()->getImage(m_uiImageId);
    if(pImage == NULL) return;

    //Render engine is responsible for resetting the camera
    float fTexLeft   = m_iFrameW * 1.0F / pImage->m_iNumFramesW,
          fTexTop    = m_iFrameH * 1.0F / pImage->m_iNumFramesH,
          fTexRight  = m_iFrameW * 1.0F / pImage->m_iNumFramesW + m_iRepsW * 1.0F / pImage->m_iNumFramesW,
          fTexBottom = m_iFrameH * 1.0F / pImage->m_iNumFramesH + m_iRepsH * 1.0F / pImage->m_iNumFramesH;

    Point ptPos = getPosition();
    glTranslatef((ptPos.x + m_rcDrawArea.x), (ptPos.y + m_rcDrawArea.y), (ptPos.z));

    //Bind the texture to which subsequent calls refer to
    glBindTexture( GL_TEXTURE_2D, pImage->m_uiTexture );
    //glDepthMask(GL_FALSE);
    //Point ptCamPos = D3RE::get()->getCameraPosition();
    //billboardSphericalBegin(ptCamPos.x, ptCamPos.y, ptCamPos.z, ptPos.x, ptPos.y, ptPos.z);
    glBegin(GL_QUADS);
        glColor3f(ourColor.r / 255.f, ourColor.g / 255.f, ourColor.b / 255.f);
        //Top-left vertex (corner)
        glTexCoord2f(fTexLeft, fTexTop);
        glVertex3f(0.f, m_rcDrawArea.h, 0.f);

        //Top-right vertex (corner)
        glTexCoord2f(fTexRight, fTexTop);
        glVertex3f(m_rcDrawArea.w, m_rcDrawArea.h, 0.f);

        //Bottom-right vertex (corner)
        glTexCoord2f(fTexRight, fTexBottom);
        glVertex3f(m_rcDrawArea.w, 0.f, 0.f);

        //Bottom-left vertex (corner)
        glTexCoord2f(fTexLeft, fTexBottom);
        glVertex3f(0.f, 0.f, 0.f);
    glEnd();
    //billboardEnd();
    //glDepthMask(GL_TRUE);
    glPopMatrix();
}

Rect
D3SpriteRenderModel::getDrawArea() {
    Point ptPos = getPosition();
    return Rect(ptPos.x + m_rcDrawArea.x, ptPos.y + m_rcDrawArea.y, m_rcDrawArea.w, m_rcDrawArea.h);
}

Point
D3SpriteRenderModel::getPosition() {
    Point ptPos = Point();
    if(m_pParent != NULL) {
        ptPos = m_pParent->getPhysicsModel()->getPosition();
    }
    return ptPos;
}

//Code borrowed from http://www.lighthouse3d.com/opengl/billboarding
void
D3SpriteRenderModel::billboardSphericalBegin(
			float camX, float camY, float camZ,
			float objPosX, float objPosY, float objPosZ) {

	float angleCosine;
	Point lookAt, objToCamProj, objToCam, upAux;

	glPushMatrix();

// objToCamProj is the vector in world coordinates from the
// local origin to the camera projected in the XZ plane
	objToCamProj.x = camX - objPosX ;
	objToCamProj.y = 0;
	objToCamProj.z = camZ - objPosZ ;

// This is the original lookAt vector for the object
// in world coordinates
	lookAt = Point(0, 0, 1);


// normalize both vectors to get the cosine directly afterwards
	objToCamProj.normalize();

// easy fix to determine wether the angle is negative or positive
// for positive angles upAux will be a vector pointing in the
// positive y direction, otherwise upAux will point downwards
// effectively reversing the rotation.

	upAux = cross(lookAt,objToCamProj);

// compute the angle
	angleCosine = dot(lookAt,objToCamProj);

// perform the rotation. The if statement is used for stability reasons
// if the lookAt and objToCamProj vectors are too close together then
// |angleCosine| could be bigger than 1 due to lack of precision
   if ((angleCosine < 0.99990) && (angleCosine > -0.9999))
      glRotatef(acos(angleCosine)*180/3.14,upAux.x, upAux.y, upAux.z);

// so far it is just like the cylindrical billboard. The code for the
// second rotation comes now
// The second part tilts the object so that it faces the camera

// objToCam is the vector in world coordinates from
// the local origin to the camera
	objToCam.x = camX - objPosX;
	objToCam.y = camY - objPosY;
	objToCam.z = camZ - objPosZ;

// Normalize to get the cosine afterwards
	objToCam.normalize();

// Compute the angle between objToCamProj and objToCam,
//i.e. compute the required angle for the lookup vector

	angleCosine = dot(objToCamProj,objToCam);


// Tilt the object. The test is done to prevent instability
// when objToCam and objToCamProj have a very small
// angle between them

	if ((angleCosine < 0.99990) && (angleCosine > -0.9999)) {
		if (objToCam.y < 0) {
			glRotatef(acos(angleCosine)*180/3.14,1,0,0);
		} else {
			glRotatef(acos(angleCosine)*180/3.14,-1,0,0);
		}
	}

}

void
D3SpriteRenderModel::billboardEnd() {
	glPopMatrix();
}
