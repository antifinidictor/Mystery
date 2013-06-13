#include "OrderedRenderEngine.h"

#include "SDL.h"
#include <gl/gl.h>
#include <gl/glu.h>

#include "mge/defs.h"
#include "mge/Image.h"
#include "mge/GameObject.h"
#include "mge/PhysicsModel.h"

using namespace std;

OrderedRenderEngine *OrderedRenderEngine::bre;

OrderedRenderEngine::OrderedRenderEngine() {
    m_rcScreenArea = Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );	//This line used to come directly after SDL_Init()

	glEnable( GL_TEXTURE_2D );
	glEnable( GL_BLEND );
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glAlphaFunc(GL_GREATER, 0.9f);

	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
	glViewport( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT );
	glClear( GL_COLOR_BUFFER_BIT );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, -1.0f, 1.0f);
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

}

OrderedRenderEngine::~OrderedRenderEngine() {
    //dtor
}


void OrderedRenderEngine::render() {
    glClear ( GL_COLOR_BUFFER_BIT );

    for(list<GameObject*>::iterator it = m_lsObjsOnScreen.begin();
            it != m_lsObjsOnScreen.end(); ++it) {
        if(!(*it)->getFlag(ORE_INVISIBLE)) {
            (*it)->getRenderModel()->render(this);
        }
    }

    //m_lsObjsOnScreen.clear();

    //Blit the buffer onto the screen
    SDL_GL_SwapBuffers();   //Should probably be done by the render engine
}

void OrderedRenderEngine::manageObjOnScreen(GameObject *obj) {
    bool bIntersects = rcIntersects(obj->getRenderModel()->getDrawArea(), m_rcScreenArea);
    if(bIntersects && !obj->getFlag(ORE_ON_SCREEN)) {
        addInOrder(obj);
    } else if(bIntersects) {
        resort(obj);
    } else if(obj->getFlag(ORE_ON_SCREEN)) {
        remove(obj);
    }
}

void OrderedRenderEngine::moveScreenTo(Point pt) {
    m_rcScreenArea.x = pt.x;
    m_rcScreenArea.y = pt.y;
}

Image *OrderedRenderEngine::createImage(const char *name, int numFramesH, int numFramesW) {
    uint id;
    for(id = 0; id < m_vImages.size(); ++id) {
        if(m_vImages[id] == NULL) {
            m_vImages[id] = new Image(name, id, numFramesH, numFramesW);
            return m_vImages[id];
        }
    }
    //id will equal the final image value
    m_vImages.push_back(new Image(name, id, numFramesH, numFramesW));
    return m_vImages[id];
}

Image *OrderedRenderEngine::getImage(uint id) {
    if(id < m_vImages.size()) {
        return m_vImages[id];
    }
    return NULL;
}

Image *OrderedRenderEngine::getMappedImage(uint mapValue) {
    map<uint,uint>::iterator iter = m_mStdImgs.find(mapValue);
    if(iter != m_mStdImgs.end() && iter->second < m_vImages.size()) {
        return m_vImages[iter->second];
    }
    return NULL;
}

void OrderedRenderEngine::freeImage(uint id) {
    if(id < m_vImages.size() && m_vImages[id] != NULL) {
        delete m_vImages[id];
        m_vImages[id] = NULL;
    }
}

void OrderedRenderEngine::mapStandardImage(uint mapValue, uint id) {
    m_mStdImgs[mapValue] = id;
}

void OrderedRenderEngine::addInOrder(GameObject *obj) {
    for(list<GameObject*>::iterator it = m_lsObjsOnScreen.begin();
            it != m_lsObjsOnScreen.end(); ++it) {
        if(comesBefore(obj, *it)) { //obj->getRenderModel()->getPosition() < (*it)->getRenderModel()->getPosition()) { //adjust this so it measures order
            m_lsObjsOnScreen.insert(it, obj);
            obj->setFlag(ORE_ON_SCREEN, true);
            return;
        }
    }
    m_lsObjsOnScreen.push_back(obj);
    obj->setFlag(ORE_ON_SCREEN, true);
}

void OrderedRenderEngine::resort(GameObject *obj) {
    //TODO: implement an actual resorting algorithm!
    remove(obj);
    addInOrder(obj);
}

void OrderedRenderEngine::remove(GameObject *obj) {
    for(list<GameObject*>::iterator it = m_lsObjsOnScreen.begin();
            it != m_lsObjsOnScreen.end(); ++it) {
        if(obj->getID() == (*it)->getID()) {
            m_lsObjsOnScreen.erase(it);
            obj->setFlag(ORE_ON_SCREEN, false);
            return;
        }
    }
}

void OrderedRenderEngine::clearScreen() {
    for(list<GameObject*>::iterator it = m_lsObjsOnScreen.begin();
            it != m_lsObjsOnScreen.end(); ++it) {
        (*it)->setFlag(ORE_ON_SCREEN, false);
    }
    m_lsObjsOnScreen.clear();
}

bool OrderedRenderEngine::comesBefore(GameObject *obj1, GameObject *obj2) {
    Box bx1 = obj1->getPhysicsModel()->getCollisionVolume(),
        bx2 = obj2->getPhysicsModel()->getCollisionVolume();
    float iOrder1 = obj1->getRenderModel()->getPosition().z,
          iOrder2 = obj2->getRenderModel()->getPosition().z;
    return iOrder1 < iOrder2
        || ((bx1.z + bx1.h < bx2.z))
        || ((bx1.z + bx1.h > bx2.z) && (bx1.z < bx2.z + bx2.h) && (bx1.y < bx2.y))
        || ((bx1.z + bx1.h > bx2.z) && (bx1.z < bx2.z + bx2.h) && (bx1.y == bx2.y) && (bx1.x < bx2.x));
}

GameObject *OrderedRenderEngine::getObjAtPos(float x, float y) {
    for(list<GameObject*>::iterator it = m_lsObjsOnScreen.end();
            it != m_lsObjsOnScreen.begin(); ) {
        --it;
        Rect rcArea = (*it)->getRenderModel()->getDrawArea();
        rcArea.x -= m_rcScreenArea.x;
        rcArea.y -= m_rcScreenArea.y;
        if(ptInRect(Point(x,y,0), rcArea)) {
            return (*it);
        }
    }
    return NULL;
}
