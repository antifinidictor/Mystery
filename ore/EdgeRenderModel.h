/*
 * EdgeRenderModel
 * Ordered 2D render model
 */

#ifndef EDGE_RENDER_MODEL_H
#define EDGE_RENDER_MODEL_H

#include "mge/RenderModel.h"
#include "mge/RenderEngine.h"
#include "mge/Image.h"
#include <vector>

#define Z_TO_Y(z) (-z * 0.75)

//A class that lets you iterate through edge volumes.
class EdgeList {
public:
    virtual Box getVolume() = 0;    //Gets our volume
    virtual bool hasNext() = 0;
    virtual Box nextVolume() = 0;   //Gets the next volume from the specified list
    virtual void setList(int iDir) = 0;
};

struct Strip {
    Rect m_rcArea;
    int  m_iFrame;
    int  m_iVReps;
    Strip(Rect rcArea, int iFrame, int iVReps) {
        m_rcArea = rcArea;
        m_iFrame = iFrame;
        m_iVReps = iVReps;
    }
};

enum HorizFrames {
    HF_TOP_NORTH = 0,
    HF_TOP,
    HF_TOP_SOUTH,
    HF_BTM,
    HF_BTM_SOUTH,
    HF_NUM_FRAMES
};

enum VertFrames {
    VF_TOP_WEST = 0,
    VF_TOP_EAST,
    VF_BTM_WEST,
    VF_BTM_EAST,
    VF_NUM_FRAMES
};

enum CornerFrames {
    CF_TOP_NORTH_WEST_OUT = 0,
    CF_TOP_NORTH_EAST_OUT,
    CF_TOP_SOUTH_WEST_OUT,
    CF_TOP_SOUTH_EAST_OUT,
    CF_BTM_SOUTH_WEST_OUT,
    CF_BTM_SOUTH_EAST_OUT,
    CF_TOP_NORTH_WEST_IN,
    CF_TOP_NORTH_EAST_IN,
    CF_TOP_SOUTH_WEST_IN,
    CF_TOP_SOUTH_EAST_IN,
    CF_BTM_SOUTH_WEST_IN,
    CF_BTM_SOUTH_EAST_IN,
    CF_NUM_FRAMES
};

class EdgeRenderModel : public RenderModel {
public:
    EdgeRenderModel(EdgeList *pEdgeList, Rect rcArea, float fZ, int iLayer);
    virtual ~EdgeRenderModel() {
    }

    virtual void render(RenderEngine *re);


    virtual void moveBy(Point ptShift) {
        m_rcDrawArea += ptShift;
        m_rcDrawArea.y += Z_TO_Y(ptShift.z);
    }

    virtual Point getPosition() {
        return Point(m_rcDrawArea.x, m_rcDrawArea.y, m_iLayer);
    }

    void setLayer(int iLayer) { m_iLayer = iLayer; }

    virtual Rect getDrawArea() { return m_rcDrawArea; }
    Image *getImage() { return m_pImageHEdges; }

private:
    Image *m_pImageVEdges,  //Texture for vertical edges
          *m_pImageHEdges,  //Texture for horizontal edges and center/side
          *m_pImageCorners; //Corner textures
    Rect m_rcDrawArea;
    EdgeList *m_pEdgeList;
    std::vector<Strip> m_vStrips;

    virtual void addStrip(Rect rcArea, int iFrame, int iVReps);
    virtual void renderStrip(int m_iFrameW, Rect rcDA, bool bHoriz);
    virtual void updateStrips();
    virtual void updateHorizEdge(int list, int frame);
    virtual void updateVertEdge(int list, int frame);

    int m_iLayer;
};

#endif
