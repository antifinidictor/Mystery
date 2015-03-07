#include "ContainerRenderModel.h"
#include "pgl.h"

ContainerRenderModel::ContainerRenderModel(Positionable *pParent, Rect rcArea) :
    m_rcTotalArea(rcArea),
    m_ptOffset()
{
    m_pParent = pParent;
}

//This is a hack
ContainerRenderModel::ContainerRenderModel(Positionable *pParent, Rect rcArea, Point ptOffset) :
    m_rcTotalArea(rcArea),
    m_ptOffset(ptOffset)
{
    m_pParent = pParent;
}

ContainerRenderModel::~ContainerRenderModel() {
    clear();
}

void
ContainerRenderModel::render(RenderEngine *re) {
    glPushMatrix();

    //This bit is unnecessary because all render models respond to their parent object, and when a render model is added to
    //a container, its parent object becomes the container.  So, the container's position is already factored in.
    //There is probably a better way to do this; I'm pretty sure the matrix stack is better than the nested getParent() position
    //calls, but it would require additional fiddling.
    //Point ptPos = getParentPosition();
    //glTranslatef(m_rcTotalArea.x + ptPos.x, m_rcTotalArea.y + ptPos.y, ptPos.z);
/*
    glBindTexture( GL_TEXTURE_2D, 0);
    glBegin(GL_LINES);
        //Bottom edges
        glColor3f(0.f, 1.f, 1.f);

        glVertex3f(              0,             0, 0);
        glVertex3f(              0, m_rcTotalArea.h, 0);

        glVertex3f(              0, m_rcTotalArea.h, 0);
        glVertex3f(m_rcTotalArea.w, m_rcTotalArea.h, 0);

        glVertex3f(m_rcTotalArea.w, m_rcTotalArea.h, 0);
        glVertex3f(m_rcTotalArea.w,             0, 0);

        glVertex3f(m_rcTotalArea.w,             0, 0);
        glVertex3f(              0,             0, 0);
    glEnd();
*/
    for(std::map<uint, RenderModel*>::iterator iter = m_mModels.begin();
            iter != m_mModels.end(); ++iter) {
        iter->second->render(re);
    }
    glPopMatrix();
}

void
ContainerRenderModel::moveBy(const Point &ptShift) {
    m_rcTotalArea += ptShift;
}

Point
ContainerRenderModel::getPosition() {
    return Point(m_rcTotalArea) + m_ptOffset + getParentPosition();
}

void
ContainerRenderModel::add(uint id, RenderModel *mdl) {
    m_mModels[id] = mdl;
    mdl->setParent(this);
}

void
ContainerRenderModel::remove(uint id) {
    std::map<uint,RenderModel*>::iterator it = m_mModels.find(id);
    if(it != m_mModels.end()) {
        it->second->setParent(NULL);
        m_mModels.erase(it);
    }
}

void
ContainerRenderModel::erase(uint id) {
    std::map<uint,RenderModel*>::iterator it = m_mModels.find(id);
    if(it != m_mModels.end()) {
        it->second->setParent(NULL);
        delete it->second;
        m_mModels.erase(it);
    }
}

void
ContainerRenderModel::clear() {
static std::string spaces = "";
    //printf(__FILE__" %d: Number of models = %d for obj %x {\n", __LINE__, m_mModels.size(), this);
    for(std::map<uint, RenderModel*>::iterator iter = m_mModels.begin();
            iter != m_mModels.end(); ++iter) {
        //printf(__FILE__" %d: Obj %d @ %x, child of %x, child of %x\n", __LINE__, iter->first, iter->second, this, m_pParent);
printf("%s%x:%d; %x {\n", spaces.c_str(), iter->second, iter->first, this);
spaces.push_back('\t');
        iter->second->setParent(NULL);
        delete iter->second;
//spaces.pop_back();
spaces.erase(spaces.end()-1);
printf("%s} (%x:%d; %x)\n", spaces.c_str(), iter->second, iter->first, this);
    }
    m_mModels.clear();
    //printf(__FILE__" %d: Number of remaining models = %d for obj %x }\n", __LINE__, m_mModels.size(), this);
}
/*
template <class RenderModelType>
RenderModelType
ContainerRenderModel::get(uint id) {
    std::map<uint, RenderModel*>::iterator it = m_mModels.find(id);
    if(it != m_mModels.end()) {
        return dynamic_cast<RenderModelType>(it->second);
    } else {
        return NULL;
    }
}
*/
