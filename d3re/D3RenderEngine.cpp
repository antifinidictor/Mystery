/*
 * Source file for the D3RenderEngine
 */

#include "d3re.h"

#include "SDL.h"
#include "pgl.h"

#include "mge/GameObject.h"
#include "game/game_defs.h"

using namespace std;

#define CAM_DIST 200.f
#define CAM_ANGLE (9 * M_PI / 32)

//static members
D3RenderEngine *D3RenderEngine::re;

D3RenderEngine::D3RenderEngine() {
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );	//This line used to come directly after SDL_Init()

	glEnable( GL_TEXTURE_2D );
	glEnable( GL_BLEND );
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//These allow for only binary alpha values: all visible or not visible.
	// Until proper render sorting is implemented, this should be used.
    glAlphaFunc(GL_GREATER, 0.9f);
    glEnable(GL_ALPHA_TEST);

    //Set the initial window size
    resize(SCREEN_WIDTH, SCREEN_HEIGHT);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black and opaque
    glClearDepth(1.0f);                   // Set background depth to farthest
    glEnable(GL_DEPTH_TEST);   // Enable depth testing for z-culling
    glDepthFunc(GL_LEQUAL);    // Set the type of depth-test
    glShadeModel(GL_SMOOTH);   // Enable smooth shading
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);  // Nice perspective corrections

    prepCamera();

    //Other values

    m_fCamDist = CAM_DIST;
    m_fCamAngle = CAM_ANGLE;
    m_ptPos = Point();
    updateCamPos();
    m_crWorld = Color(0xFF, 0xFF, 0xFF);
    m_fColorWeight = 0.5f;
    m_bGuiMode = false;

    m_ptMouseInWorld = Point(-SCREEN_WIDTH / 2, 0.f, -SCREEN_HEIGHT / 2);
    m_v3MouseRay = Vec3f();

    m_pHudContainer = new ContainerRenderModel(Rect(0,0,SCREEN_WIDTH, SCREEN_HEIGHT));
    MGE::get()->addListener(this, ON_MOUSE_MOVE);
}

D3RenderEngine::~D3RenderEngine() {
    MGE::get()->removeListener(getId(), ON_MOUSE_MOVE);
    delete m_pHudContainer;
}

void
D3RenderEngine::render() {
    glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    prepCamera();
    for(list<GameObject *>::iterator it = m_lsObjsOnScreen.begin();
            it != m_lsObjsOnScreen.end(); ++it) {
        if(!(*it)->getFlag(D3RE_INVISIBLE)) {
            (*it)->getRenderModel()->render(this);
        }
    }

    drawCircle(m_ptMouseInWorld, 2.f, Color(0x0, 0x0, 0xFF));

    prepHud();
    m_pHudContainer->render(this);

    //glBindTexture(GL_TEXTURE_2D, 0);
    SDL_GL_SwapBuffers();   //Should probably be done by the render engine
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
    obj->setFlag(D3RE_ON_SCREEN, true);
    for(it = m_lsObjsOnScreen.begin(); it != m_lsObjsOnScreen.end(); ++it) {
        if(comesBefore(obj, *it)) {
            m_lsObjsOnScreen.insert(it, obj);
            return;
        }
    }

    //Does not come before any objects on screen
    m_lsObjsOnScreen.push_back(obj);
}

void
D3RenderEngine::resort(GameObject *obj) {
    //TODO: implement an actual resorting algorithm!
    remove(obj);
    addInOrder(obj);
}

void
D3RenderEngine::remove(GameObject *obj) {
    for(list<GameObject *>::iterator it = m_lsObjsOnScreen.begin();
            it != m_lsObjsOnScreen.end(); ++it) {
        if(obj->getId() == (*it)->getId()) {
            m_lsObjsOnScreen.erase(it);
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
D3RenderEngine::createImage(uint id, const char *name, int numFramesH, int numFramesW) {
    if(id < m_vImages.size() && m_vImages[id] != NULL) {
        //TODO: Find out why this doesn't work!
        //delete m_vImages[id];
        return m_vImages[id];
    }
    while(id >= m_vImages.size()) {
        m_vImages.push_back(NULL);
    }
    m_vImages[id] = new Image(name, id, numFramesH, numFramesW);
    return m_vImages[id];
}

Image *
D3RenderEngine::getImage(uint id) {
    if(id < m_vImages.size()) {
        return m_vImages[id];
    }
    //else
    return NULL;
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
    gluLookAt(m_ptCamPos.x, m_ptCamPos.y, m_ptCamPos.z, //Camera position
              m_ptPos.x,    m_ptPos.y,    m_ptPos.z,             //Look at this coord
              0,            1,            -1);                   //Up vector
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
        gluPerspective(45.0f, aspect, 1.f, 1024.0f);
    }
}

void
D3RenderEngine::drawBox(const Box &bx, const Color &cr) {
    //D3RE::get()->prepCamera();

    glBindTexture( GL_TEXTURE_2D, NULL);
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
    glBindTexture( GL_TEXTURE_2D, NULL);
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
}

void
D3RenderEngine::read(boost::property_tree::ptree &pt, const std::string &keyBase) {
    using boost::property_tree::ptree;
    try {
    BOOST_FOREACH(ptree::value_type &v, pt.get_child(keyBase.c_str())) {
        //Each element should be stored by id
        string key = keyBase + "." + v.first.data();
        string filename = v.second.data();
        uint uiId = atoi(v.first.data());
        uint framesW = pt.get(key + ".framesW", 1);
        uint framesH = pt.get(key + ".framesH", 1);
        createImage(uiId, filename.c_str(), framesH, framesW);
    }
    } catch(exception e) {
        printf("Could not read resources\n");
    }
}

void
D3RenderEngine::updateCamPos() {
    m_ptCamPos = Point(m_ptPos.x,
                       m_ptPos.y + m_fCamDist * sin(m_fCamAngle),
                       m_ptPos.z + m_fCamDist * cos(m_fCamAngle));
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

void
D3RenderEngine::callBack(uint uiEventHandlerId, void *data, uint uiEventId) {
    switch(uiEventId) {
    case ON_MOUSE_MOVE: {
        InputData *idat = (InputData*)data;
        //m_ptMouseInWorld.x += idat->getInputState(MIN_MOUSE_REL_X);
        //m_ptMouseInWorld.z += idat->getInputState(MIN_MOUSE_REL_Y);
        int x = idat->getInputState(MIN_MOUSE_X);
        int y = idat->getInputState(MIN_MOUSE_Y);
        updateMousePos(x, y);
        break;
      }
    default:
        break;
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
D3RenderEngine::updateMousePos(int x, int y) {
    #define X_SCALE 110.f / CAM_DIST
    #define Z_SCALE 80.f / CAM_DIST
    #define SKEW_FACTOR -25.f / CAM_DIST

    //Approx mouse coords
    float xScale = X_SCALE * m_fCamDist;
    float zScale = Z_SCALE * m_fCamDist;
    float skewFactor = SKEW_FACTOR * m_fCamDist;
    float xNorm = 2 * (float)(x - SCREEN_WIDTH / 2) / (float)SCREEN_WIDTH;  //Range = -1 to 1
    float zNorm = 2 * (float)(y - SCREEN_HEIGHT / 2) / (float)SCREEN_HEIGHT;  //Range = -1 to 1
    m_ptMouseInWorld.x = m_ptPos.x + xScale * xNorm + skewFactor * zNorm * xNorm;
    m_ptMouseInWorld.z = m_ptPos.z + zScale * zNorm;
    #if 0
    GLdouble matModelView[16];
    GLdouble matProjection[16];
    GLint viewport[4];  //x, y, width, height

    glGetDoublev(GL_PROJECTION_MATRIX, matModelView);
    glGetDoublev(GL_MODELVIEW_MATRIX, matProjection);
    glGetIntegerv(GL_VIEWPORT, viewport);

    GLdouble resX, resY, resZ;

    x -= viewport[2] / 2;
    //y -= viewport[3] / 2;
    y = viewport[3] - y;


    //gluUnProject((GLdouble)x, (GLdouble)y, 0.1f, modelview, projection, viewport, &resX, &resY, &resZ);
    //Point ptNearPlane = Point((float)resX, (float)resY, (float)resZ);

    gluUnProject((GLdouble)x, (GLdouble)y, 1.f, matModelView, matProjection, viewport, &resX, &resZ, &resY);
    //Point ptFarPlane =Point((float)resX, (float)resY, (float)resZ);

    //m_v3MouseRay = ptFarPlane - ptNearPlane;
    m_v3MouseRay = Point((float)resX, (float)resY, (float)resZ);

    //glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z); //Get a single pixel of the depth buffer at the mouse position
    //gluUnProject((GLdouble)x, (GLdouble)y, z, modelview, projection, viewport, &resX, &resY, &resZ);
    //m_ptMouseInWorld = Point((float)resX, (float)resY, (float)resZ) + m_ptCamPos;
    #endif

}

