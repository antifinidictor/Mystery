/*
 * Source file for the D3RenderEngine
 */

#include "d3re.h"

#include "SDL.h"
#include "pgl.h"

#include "mge/GameObject.h"
#include "game/game_defs.h"
#include <boost/lexical_cast.hpp>

using namespace std;

#define CAM_DIST 6.25f  //200.f
#define CAM_ANGLE (9 * M_PI / 32)
#define LOOK_ANGLE (M_PI / 2)
#define CAM_ROTATE_SPEED (M_PI / 32)
#define MAX_MOUSE_TIMER 20
//static members
D3RenderEngine *D3RenderEngine::re;

D3RenderEngine::D3RenderEngine()
    :   m_ptPos(),
        m_ptCamPos(),
        m_fCamDist(CAM_DIST),
        m_fCamAngle(CAM_ANGLE),
        m_fLookAngle(LOOK_ANGLE),
        m_fDesiredLookAngle(LOOK_ANGLE),
        m_crWorld(0xFF, 0xFF, 0xFF),
        m_fColorWeight(0.5f),
        m_pHudContainer(new ContainerRenderModel(Rect(0,0,SCREEN_WIDTH, SCREEN_HEIGHT))),
        m_bGuiMode(false),
        m_bDrawCollisions(false),
        m_bDrawRealMouse(true),
        m_pMouseOverObject(NULL),
        m_iMouseX(0),
        m_iMouseY(0),
//        m_uiMouseFrame(0),
//        m_uiMouseTimer(0),
        m_ptMouseInWorld(-SCREEN_WIDTH / 2, 0.f, -SCREEN_HEIGHT / 2),
        m_v3MouseRay()
{
    assert(D3RE_NUM_FLAGS <= RENDER_FLAGS_END);

    printf("Render engine has ID %d\n", getId());
    m_sdlWindow = SDL_CreateWindow("The Child and the Alchemist",
                          SDL_WINDOWPOS_UNDEFINED,
                          SDL_WINDOWPOS_UNDEFINED,
                          SCREEN_WIDTH, SCREEN_HEIGHT,
                          /*SDL_WINDOW_FULLSCREEN |*/ SDL_WINDOW_OPENGL);
    m_glContext = SDL_GL_CreateContext(m_sdlWindow);
    SDL_Surface *icon = SDL_LoadBMP("res/icon.bmp");
    SDL_SetWindowIcon(m_sdlWindow, icon);
    SDL_FreeSurface(icon);

    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );	//This line used to come directly after SDL_Init()

	glEnable( GL_TEXTURE_2D );
	glEnable( GL_BLEND );
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//These allow for only binary alpha values: all visible or not visible.
	// Until proper render sorting is implemented, this should be used.
    glAlphaFunc(GL_GREATER, 0.9f);
    //glEnable(GL_ALPHA_TEST);

    //Set the initial window size
    resize(SCREEN_WIDTH, SCREEN_HEIGHT);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black and opaque
    glClearDepth(1.0f);                   // Set background depth to farthest
    glEnable(GL_DEPTH_TEST);   // Enable depth testing for z-culling
    glDepthFunc(GL_LEQUAL);    // Set the type of depth-test
    glShadeModel(GL_SMOOTH);   // Enable smooth shading
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);  // Nice perspective corrections

/*
    glEnable(GL_FOG);
    {
        GLfloat fogColor[4] = {0.5, 0.5, 0.5, 1.0};

        GLint fogMode = GL_EXP;
        glFogi (GL_FOG_MODE, fogMode);
        glFogfv (GL_FOG_COLOR, fogColor);
        glFogf (GL_FOG_DENSITY, 0.35);
        glHint (GL_FOG_HINT, GL_DONT_CARE);
        glFogf (GL_FOG_START, 1.0);
        glFogf (GL_FOG_END, 5.0);
    }
*/
    prepCamera();

    //Other values
    updateCamPos();

    MGE::get()->addListener(this, ON_MOUSE_MOVE);
}

D3RenderEngine::~D3RenderEngine() {
    printf("Render engine cleaning\n");
    MGE::get()->removeListener(getId(), ON_MOUSE_MOVE);

    //Clean images
    for(vector<Image*>::iterator it = m_vImages.begin(); it != m_vImages.end(); ++it) {
        delete *it;
    }
    m_vImages.clear();
    m_mImageNameToId.clear();
    m_lsTransparentObjs.clear();
    m_lsObjsOnScreen.clear();

    SDL_GL_DeleteContext(m_glContext);
    SDL_DestroyWindow(m_sdlWindow);

    delete m_pHudContainer;
}


//TODO: Move!
#include <limits>
float
rayIntersects(const Point &ptRay, const Point &ptRayStart, const Box &bxBounds) {
    float t1 = 0.f, t2 = 0.f, tmin = std::numeric_limits<float>::max();
    if(ptRay.z != 0.f) {
        t1 = (bxBounds.z - ptRayStart.z) / ptRay.z;
        t2 = (bxBounds.z + bxBounds.l - ptRayStart.z) / ptRay.z;
        bool b1 = (ptOutOfBounds(ptRay * t1 + ptRayStart, bxBounds) == 0);
        bool b2 = (ptOutOfBounds(ptRay * t2 + ptRayStart, bxBounds) == 0);
        if(b1) {
            tmin = fabs(t1) < fabs(tmin) ? t1 : tmin;
        } else if(b2) {
            tmin = fabs(t2) < fabs(tmin) ? t2 : tmin;
        }
    }
    if(ptRay.y != 0.f) {
        t1 = (bxBounds.y - ptRayStart.y) / ptRay.y;
        t2 = (bxBounds.y + bxBounds.h - ptRayStart.y) / ptRay.y;
        bool b1 = (ptOutOfBounds(ptRay * t1 + ptRayStart, bxBounds) == 0);
        bool b2 = (ptOutOfBounds(ptRay * t2 + ptRayStart, bxBounds) == 0);
        if(b1) {
            tmin = fabs(t1) < fabs(tmin) ? t1 : tmin;
        } else if(b2) {
            tmin = fabs(t2) < fabs(tmin) ? t2 : tmin;
        }
    }
    if(ptRay.x != 0.f) {
        t1 = (bxBounds.x - ptRayStart.x) / ptRay.x;
        t2 = (bxBounds.x + bxBounds.w - ptRayStart.x) / ptRay.x;
        bool b1 = (ptOutOfBounds(ptRay * t1 + ptRayStart, bxBounds) == 0);
        bool b2 = (ptOutOfBounds(ptRay * t2 + ptRayStart, bxBounds) == 0);
        if(b1) {
            tmin = fabs(t1) < fabs(tmin) ? t1 : tmin;
        } else if(b2) {
            tmin = fabs(t2) < fabs(tmin) ? t2 : tmin;
        }
    }
    return tmin;
}

void
D3RenderEngine::render() {
    //glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //Update look direction
    if(m_fDesiredLookAngle < m_fLookAngle - CAM_ROTATE_SPEED) {
        m_fLookAngle -= CAM_ROTATE_SPEED;
    } else if(m_fDesiredLookAngle > m_fLookAngle + CAM_ROTATE_SPEED) {
        m_fLookAngle += CAM_ROTATE_SPEED;
    } else {
        m_fLookAngle = m_fDesiredLookAngle;
    }

    //prepCamera();

    //Nontransparent objects
    glEnable(GL_ALPHA_TEST);
    bool mouseInObject = false;
    float mouseOverT = std::numeric_limits<float>::max();
    m_pMouseOverObject = NULL;
    for(list<GameObject *>::iterator it = m_lsObjsOnScreen.begin();
            it != m_lsObjsOnScreen.end(); ++it) {
        GameObject *obj = *it;
        if(!obj->getFlag(D3RE_INVISIBLE)) {
            obj->getRenderModel()->render(this);
            //if(ptOutOfBounds(m_ptMouseInWorld, (*it)->getPhysicsModel()->getCollisionVolume()) == 0) {
            float t = rayIntersects(m_v3MouseRay, m_ptCamPos, obj->getPhysicsModel()->getCollisionVolume());
            if(fabs(t) < fabs(mouseOverT)) {
                mouseInObject = true;
                mouseOverT = t;
                m_pMouseOverObject = *it;
            }
        }
    }
    glDisable(GL_ALPHA_TEST);

    //Transparent objects
    for(list<GameObject *>::iterator it = m_lsTransparentObjs.begin();
            it != m_lsTransparentObjs.end(); ++it) {
        if(!(*it)->getFlag(D3RE_INVISIBLE)) {
            (*it)->getRenderModel()->render(this);
            float t = rayIntersects(m_v3MouseRay, m_ptCamPos, (*it)->getPhysicsModel()->getCollisionVolume());
            if(fabs(t) < fabs(mouseOverT)) {
                mouseInObject = true;
                mouseOverT = t;
                m_pMouseOverObject = *it;
            }
        }
    }

    updateMousePos(m_iMouseX, m_iMouseY);

    if(m_bDrawCollisions) {
        glBegin(GL_LINES);
            glColor3f(1.f, 0.f, 0.f);
            glVertex3f(m_ptPos.x, m_ptPos.y, m_ptPos.z);
            glVertex3f(m_ptMouseInWorld.x, m_ptMouseInWorld.y, m_ptMouseInWorld.z);
        glEnd();
    }

    prepHud();
    glDisable(GL_DEPTH_TEST);   //Disable depth test
    m_pHudContainer->render(this);
    glEnable(GL_DEPTH_TEST);    // Re-enable depth testing for z-culling

    //glBindTexture(GL_TEXTURE_2D, 0);
    SDL_GL_SwapWindow(m_sdlWindow);

    glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    prepCamera();
}

void
D3RenderEngine::manageObjOnScreen(GameObject *obj) {
    //TODO: Optimize!  For now this just adds everything
    bool bIntersects = true; //rcIntersects(obj->getRenderModel()->getDrawArea(), m_rcScreenArea);
    if(bIntersects && !obj->getFlag(D3RE_ON_SCREEN)) {
        addInOrder(obj);
    } else if(bIntersects) {
        resort(obj);
    } else if(obj->getFlag(D3RE_ON_SCREEN)) {
        remove(obj);
    }
}

void
D3RenderEngine::addInOrder(GameObject *obj) {
    list<GameObject*>::iterator it;
    list<GameObject*> *thisList = (obj->getFlag(D3RE_TRANSPARENT)) ? &m_lsTransparentObjs : &m_lsObjsOnScreen;
    obj->setFlag(D3RE_ON_SCREEN, true);
    for(it = thisList->begin(); it != thisList->end(); ++it) {
        if(comesBefore(obj, *it)) {
            thisList->insert(it, obj);
            return;
        }
    }

    //Does not come before any objects on screen
    thisList->push_back(obj);
}

void
D3RenderEngine::resort(GameObject *obj) {
    //TODO: implement an actual resorting algorithm!
    remove(obj);
    addInOrder(obj);
}

void
D3RenderEngine::remove(GameObject *obj) {
    list<GameObject*> *thisList = (obj->getFlag(D3RE_TRANSPARENT)) ? &m_lsTransparentObjs : &m_lsObjsOnScreen;
    for(list<GameObject *>::iterator it = thisList->begin();
            it != thisList->end(); ++it) {
        if(obj->getId() == (*it)->getId()) {
            thisList->erase(it);
            obj->setFlag(D3RE_ON_SCREEN, false);
            return;
        }
    }
}

void
D3RenderEngine::clearScreen() {
    for(list<GameObject *>::iterator it = m_lsObjsOnScreen.begin();
            it != m_lsObjsOnScreen.end(); ++it) {
        (*it)->setFlag(D3RE_ON_SCREEN, false);
    }
    m_lsObjsOnScreen.clear();
    for(list<GameObject *>::iterator it = m_lsTransparentObjs.begin();
            it != m_lsTransparentObjs.end(); ++it) {
        (*it)->setFlag(D3RE_ON_SCREEN, false);
    }
    m_lsTransparentObjs.clear();
}

void
D3RenderEngine::moveScreenTo(Point pt) {
    m_ptMouseInWorld += (pt - m_ptPos);
    m_ptPos = pt;
    updateCamPos();
}

void
D3RenderEngine::moveScreenBy(Point pt) {
    m_ptPos += pt;
    m_ptMouseInWorld += pt;
    updateCamPos();
}

Image *
D3RenderEngine::createImage(uint id, const std::string &fileName, int numFramesH, int numFramesW, bool bLinearInterp) {
    if(id < m_vImages.size() && m_vImages[id] != NULL) {
        //TODO: Find out why this doesn't work!
        //delete m_vImages[id];
        return m_vImages[id];
    }
    while(id >= m_vImages.size()) {
        m_vImages.push_back(NULL);
    }
    m_vImages[id] = new Image(fileName, id, numFramesH, numFramesW, bLinearInterp);
    return m_vImages[id];
}


Image *
D3RenderEngine::createImage(uint id, const std::string &imageName, const std::string &fileName, int numFramesH, int numFramesW, bool bLinearInterp) {
    m_mImageNameToId[imageName] = id;
    return createImage(id, fileName, numFramesH, numFramesW, bLinearInterp);
}

Image *
D3RenderEngine::getImage(uint id) {
    if(id < m_vImages.size()) {
        return m_vImages[id];
    }
    //else
    return NULL;
}

Image *
D3RenderEngine::getImage(const std::string &imageName) {
    map<std::string, uint>::iterator iter = m_mImageNameToId.find(imageName);
    if(iter == m_mImageNameToId.end()) {
        return NULL;
    }
    //else
    return getImage(iter->second);
}


uint
D3RenderEngine::getImageId(const std::string &imageName) {
    map<std::string, uint>::iterator iter = m_mImageNameToId.find(imageName);
    if(iter == m_mImageNameToId.end()) {
        return 0;
    }
    return iter->second;
}

void
D3RenderEngine::freeImage(uint id) {
    if(id < m_vImages.size() && m_vImages[id] != NULL) {
        delete m_vImages[id];
        m_vImages[id] = NULL;
    }
}

void
D3RenderEngine::adjustCamDist(float delta) {
    m_fCamDist += delta;
    updateCamPos();
}

void
D3RenderEngine::adjustCamAngle(float delta) {
    m_fCamAngle += delta;
    updateCamPos();
}

void
D3RenderEngine::prepCamera() {
    enableCameraMode();
    glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	//Calculate the up vector
	Point ptLookDir = m_ptPos - m_ptCamPos;
	Point ptRight = cross(ptLookDir, Point(0.f, 1.f, 0.f));
	Point ptUp = cross(ptRight, ptLookDir);

	//Direct the camera
    gluLookAt(m_ptCamPos.x, m_ptCamPos.y, m_ptCamPos.z, //Camera position
              m_ptPos.x,    m_ptPos.y,    m_ptPos.z,             //Look at this coord
              ptUp.x,       ptUp.y,       ptUp.z);               //Up vector
}

void
D3RenderEngine::prepHud() {
    enableGuiMode();
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
}

void D3RenderEngine::resize(uint width, uint height) {
    // Compute aspect ratio of the new window
    if (height == 0) height = 1;                // To prevent divide by 0
    m_uiWidth = width;
    m_uiHeight = height;

    // Set the viewport to cover the new window
    glViewport(0, 0, width, height);

    // Set the aspect ratio of the clipping volume to match the viewport
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if(m_bGuiMode) {
        glOrtho(0.0f, m_uiWidth, m_uiHeight, 0.0f, -1.0f, 1.0f);
    } else {
        GLfloat aspect = (GLfloat)width / (GLfloat)height;
        // Enable perspective projection with fovy, aspect, zNear and zFar
        gluPerspective(45.0f, aspect, 0.1f, 1.f);
    }
}

void
D3RenderEngine::drawBox(const Box &bx, const Color &cr) {
    //D3RE::get()->prepCamera();

    glBindTexture( GL_TEXTURE_2D, 0);
    glBegin(GL_LINES);
        //Bottom edges
        glColor3f(cr.r / 255.f, cr.g / 255.f, cr.b / 255.f);
        glVertex3f(bx.x,        bx.y,        bx.z);
        glVertex3f(bx.x + bx.w, bx.y,        bx.z);

        glVertex3f(bx.x + bx.w, bx.y,        bx.z);
        glVertex3f(bx.x + bx.w, bx.y,        bx.z + bx.l);

        glVertex3f(bx.x + bx.w, bx.y,        bx.z + bx.l);
        glVertex3f(bx.x,        bx.y,        bx.z + bx.l);

        glVertex3f(bx.x,        bx.y,        bx.z + bx.l);
        glVertex3f(bx.x,        bx.y,        bx.z);

        //Middle edges
        glVertex3f(bx.x,        bx.y,        bx.z);
        glVertex3f(bx.x,        bx.y + bx.h, bx.z);

        glVertex3f(bx.x + bx.w, bx.y,        bx.z);
        glVertex3f(bx.x + bx.w, bx.y + bx.h, bx.z);

        glVertex3f(bx.x + bx.w, bx.y,        bx.z + bx.l);
        glVertex3f(bx.x + bx.w, bx.y + bx.h, bx.z + bx.l);

        glVertex3f(bx.x,        bx.y,        bx.z + bx.l);
        glVertex3f(bx.x,        bx.y + bx.h, bx.z + bx.l);

        //Top edges
        glVertex3f(bx.x,        bx.y + bx.h, bx.z);
        glVertex3f(bx.x + bx.w, bx.y + bx.h, bx.z);

        glVertex3f(bx.x + bx.w, bx.y + bx.h, bx.z);
        glVertex3f(bx.x + bx.w, bx.y + bx.h, bx.z + bx.l);

        glVertex3f(bx.x + bx.w, bx.y + bx.h, bx.z + bx.l);
        glVertex3f(bx.x,        bx.y + bx.h, bx.z + bx.l);

        glVertex3f(bx.x,        bx.y + bx.h, bx.z + bx.l);
        glVertex3f(bx.x,        bx.y + bx.h, bx.z);
    glEnd();
}

void
D3RenderEngine::drawCircle(const Point &ptCenter, float radius, const Color &cr) {
    const float STEP = M_PI / 10;
    glBindTexture( GL_TEXTURE_2D, 0);
    glBegin(GL_LINE_LOOP);
        glColor3f(cr.r / 255.f, cr.g / 255.f, cr.b / 255.f);
        for(float theta = 0.f; theta < M_PI * 2.f; theta += STEP) {
            glVertex3f(cos(theta) * radius + ptCenter.x, ptCenter.y, sin(theta) * radius + ptCenter.z);
        }
    glEnd();
}

void
D3RenderEngine::setBackgroundColor(const Color &cr) {
    glClearColor(cr.r / 255.f, cr.g / 255.f, cr.b / 255.f, 1.0f);
}

void
D3RenderEngine::write(boost::property_tree::ptree &pt, const std::string &keyBase) {
    std::vector<Image*>::iterator iter;
    for(iter = m_vImages.begin(); iter != m_vImages.end(); ++iter) {
        if(*iter == NULL) continue;
        std::ostringstream key;
        key << keyBase << "." << (*iter)->m_uiID;
        pt.put(key.str(), (*iter)->m_sImageFileName);
        pt.put(key.str() + ".framesW", (*iter)->m_iNumFramesW);
        pt.put(key.str() + ".framesH", (*iter)->m_iNumFramesH);
    }
    map<std::string, uint>::iterator itNamedImages;
    for(itNamedImages = m_mImageNameToId.begin(); itNamedImages != m_mImageNameToId.end(); ++itNamedImages) {
        std::ostringstream key;
        key << keyBase << "." << itNamedImages->second;
        pt.put(key.str() + ".name", itNamedImages->first);
    }
}

void
D3RenderEngine::read(boost::property_tree::ptree &pt, const std::string &keyBase) {
    using boost::property_tree::ptree;
    using boost::lexical_cast;
    using boost::bad_lexical_cast;
    try {
    BOOST_FOREACH(ptree::value_type &v, pt.get_child(keyBase.c_str())) {
        //Each element should be stored by id
        string key = keyBase + "." + v.first.data();
        string filename = v.second.data();

        try{
            uint uiId = lexical_cast<uint>(v.first.data());
            uint framesW = pt.get(key + ".framesW", 1);
            uint framesH = pt.get(key + ".framesH", 1);
            string name = pt.get(key + ".name", "?");

            if(name.compare("?") == 0) {
                createImage(uiId, filename, framesH, framesW);
            } else if(name.compare("font") == 0) {
                createImage(uiId, name, filename, framesH, framesW, true);
            } else {
                createImage(uiId, name, filename, framesH, framesW);
            }
        } catch(const bad_lexical_cast &) {
            printf("Could not read resource: bad id\n");
        }
    }
    } catch(exception e) {
        printf("Could not read resources\n");
    }
}

void
D3RenderEngine::updateCamPos() {
    m_ptCamPos = Point(m_ptPos.x + m_fCamDist * cos(m_fCamAngle) * cos(m_fLookAngle),
                       m_ptPos.y + m_fCamDist * sin(m_fCamAngle),
                       m_ptPos.z + m_fCamDist * cos(m_fCamAngle) * sin(m_fLookAngle));
}


void
D3RenderEngine::enableCameraMode() {
    if(m_bGuiMode) {
        m_bGuiMode = false;
        GLfloat aspect = (GLfloat)m_uiWidth / (GLfloat)m_uiHeight;

        // Set the aspect ratio of the clipping volume to match the viewport
        glMatrixMode(GL_PROJECTION);  // To operate on the Projection matrix
        glLoadIdentity();             // Reset
        // Enable perspective projection with fovy, aspect, zNear and zFar
        gluPerspective(45.0f, aspect, 1.f, 1024.0f);
    }
}

void
D3RenderEngine::enableGuiMode() {
    if(!m_bGuiMode) {
        m_bGuiMode = true;

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0.0f, m_uiWidth, m_uiHeight, 0.0f, -1.0f, 1.0f);
    }
}

int
D3RenderEngine::callBack(uint uiEventHandlerId, void *data, uint uiEventId) {
    switch(uiEventId) {
    case ON_MOUSE_MOVE: {
        InputData *idat = (InputData*)data;
        //m_ptMouseInWorld.x += idat->getInputState(MIN_MOUSE_REL_X);
        //m_ptMouseInWorld.z += idat->getInputState(MIN_MOUSE_REL_Y);
        m_iMouseX = idat->getInputState(MIN_MOUSE_X);
        m_iMouseY = idat->getInputState(MIN_MOUSE_Y);
        //updateMousePos(x, y);
        return EVENT_CAUGHT;
      }
    default:
        return EVENT_DROPPED;
    }
}

bool
D3RenderEngine::comesBefore(GameObject *obj1, GameObject *obj2) {
    Box bx1 = obj1->getPhysicsModel()->getCollisionVolume();
    Box bx2 = obj2->getPhysicsModel()->getCollisionVolume();
    float top1 = bx1.y + bx1.h,
          top2 = bx2.y + bx2.h,
          front1 = bx1.z + bx1.l,
          front2 = bx2.z + bx2.l,
          right1 = bx1.x + bx1.w;
    return (top1 < bx2.y) ||
           (top1 >= bx2.y && bx1.y <= top2 &&
            front1 < bx2.z) ||
           (top1 >= bx2.y && bx1.y <= top2 &&
            front1 >= bx2.z && bx1.z <= front2 &&
            right1 < bx2.x);
}


void
D3RenderEngine::showRealMouse() {
    m_bDrawRealMouse = true;
    SDL_ShowCursor(TRUE);
}

void
D3RenderEngine::hideRealMouse() {
    m_bDrawRealMouse = false;
    SDL_ShowCursor(FALSE);
}

void
D3RenderEngine::updateMousePos(int x, int y) {
    //Code courtesy of http://nehe.gamedev.net/article/using_gluunproject/16013/
    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    GLfloat winX, winY, winZ;
    GLdouble posX, posY, posZ;

    glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
    glGetDoublev( GL_PROJECTION_MATRIX, projection );
    glGetIntegerv( GL_VIEWPORT, viewport );

    winX = (float)x;
    winY = (float)viewport[3] - (float)y;
    glReadPixels( x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );

    gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);
    m_ptMouseInWorld = Point(posX, posY, posZ);
    m_v3MouseRay = m_ptMouseInWorld - m_ptCamPos;
}
