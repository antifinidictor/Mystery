#include "ContainerRenderModel.h"
#include "pgl.h"

ContainerRenderModel::ContainerRenderModel(Rect rcArea) {
    m_rcTotalArea = rcArea;
    m_ptOffset = Point();
}

//This is a hack
ContainerRenderModel::ContainerRenderModel(Rect rcArea, Point ptOffset) {
    m_rcTotalArea = rcArea;
    m_ptOffset = ptOffset;
}

ContainerRenderModel::~ContainerRenderModel() {
    clear();
}

void
ContainerRenderModel::render(RenderEngine *re) {
    glPushMatrix();
    glTranslatef(m_rcTotalArea.x, m_rcTotalArea.y, 0.f);
    for(std::map<uint, RenderModel*>::iterator iter = m_mModels.begin();
            iter != m_mModels.end(); ++iter) {
        iter->second->render(re);
    }
    glPopMatrix();
}

void
ContainerRenderModel::moveBy(const Point &ptShift) {
    m_rcTotalArea += ptShift;
    /*
    for(std::map<uint, RenderModel*>::iterator iter = m_mModels.begin();
            iter != m_mModels.end(); ++iter) {
        iter->second->moveBy(ptShift);
    }
    */
}

Point
ContainerRenderModel::getPosition() {
    return Point(m_rcTotalArea) + m_ptOffset;
}

void
ContainerRenderModel::add(uint id, RenderModel *mdl) {
    m_mModels[id] = mdl;
}

void
ContainerRenderModel::remove(uint id) {
    m_mModels.erase(id);
}

void
ContainerRenderModel::erase(uint id) {
    std::map<uint,RenderModel*>::iterator it = m_mModels.find(id);
    if(it != m_mModels.end()) {
        delete it->second;
        m_mModels.erase(it);
    }
}

void
ContainerRenderModel::clear() {
    for(std::map<uint, RenderModel*>::iterator iter = m_mModels.begin();
            iter != m_mModels.end(); ++iter) {
        delete iter->second;
    }
    m_mModels.clear();
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
