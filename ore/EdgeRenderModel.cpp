/*
 * EdgeRenderModel.cpp
 * I'm sick of giving render models only a .h file.
 */

#include "EdgeRenderModel.h"
#include "OrderedRenderEngine.h"
#include "game/GameDefs.h"

#define HORIZ true
#define VERT false
using namespace std;

EdgeRenderModel::EdgeRenderModel(EdgeList *pEdgeList, Rect rcArea, float fZ, int iLayer) {
    m_rcDrawArea = Rect(rcArea.x, rcArea.y + Z_TO_Y(fZ), rcArea.w, rcArea.l);
    ORE *ore = ORE::get();
    m_pImageVEdges = ore->getMappedImage(IMG_V_EDGES);
    m_pImageHEdges = ore->getMappedImage(IMG_H_EDGES);
    m_pImageCorners = ore->getMappedImage(IMG_CORNERS);
    m_iLayer = iLayer;
    m_pEdgeList = pEdgeList;
    
    //Test stuff
    float fTileW = m_pImageHEdges->w,
          fTileL = m_pImageVEdges->h;
    addStrip(Rect(fTileW,fTileL*0,fTileW * 5, fTileL), HF_TOP_NORTH, 1);
    addStrip(Rect(fTileW,fTileL*1,fTileW * 5, fTileL), HF_TOP, 4);
    addStrip(Rect(fTileW,fTileL*5,fTileW * 5, fTileL), HF_TOP_SOUTH, 1);
    addStrip(Rect(fTileW,fTileL*6,fTileW * 5, fTileL), HF_BTM, 2);
    addStrip(Rect(fTileW,fTileL*8,fTileW * 5, fTileL), HF_BTM_SOUTH, 1);

    addStrip(Rect(fTileW * 0, fTileL,     fTileW, fTileL * 4), VF_TOP_WEST, 0);
    addStrip(Rect(fTileW * 6, fTileL,     fTileW, fTileL * 4), VF_TOP_EAST, 0);
    addStrip(Rect(fTileW * 0, fTileL * 6, fTileW, fTileL * 2), VF_BTM_WEST, 0);
    addStrip(Rect(fTileW * 6, fTileL * 6, fTileW, fTileL * 2), VF_BTM_EAST, 0);
}

void EdgeRenderModel::addStrip(Rect rcArea, int iFrame, int iVReps) {
    m_vStrips.push_back(Strip(rcArea, iFrame, iVReps));
}

void EdgeRenderModel::updateStrips() {
    //Uses the edgelist thing to fill the strip list with appropriate strips.
    m_vStrips.clear();  //We'll be refilling this anyway

/*
    updateHorizEdge(NORTH, HF_TOP_NORTH);
    updateHorizEdge(SOUTH, HF_TOP_SOUTH);
    updateHorizEdge(SOUTH, HF_BTM_SOUTH);   //Not the most efficient, but it should work

    updateVertEdge(WEST, VF_TOP_WEST);
    updateVertEdge(EAST, VF_TOP_EAST);
    //Need to find a way to handle height for the vertical bottom edges

    //Central strips
*/

    /*   _  Imagine this is a cube.
        |_| Each line represents an edge we need to check.
        |_| Otherwise, the rest can be divided into horizontal strips
            stretching all the way across the volume.
     */
    Box bxOurVolume = m_pEdgeList->getVolume();
    int iTileW = m_pImageHEdges->w,
        iTileL = m_pImageVEdges->h;
    //Top
    float x = bxOurVolume.x,
          y = bxOurVolume.y + Z_TO_Y(bxOurVolume.z + bxOurVolume.h);
    int w = bxOurVolume.w,
        l = iTileL,
        reps = bxOurVolume.l / iTileL;
    addStrip(Rect(x,y,w,l), HF_TOP, reps);
    
    //Side
    // x, w, l do not change
    y = bxOurVolume.y + bxOurVolume.l + Z_TO_Y(bxOurVolume.z + bxOurVolume.h) + iTileL;
    reps = -Z_TO_Y(bxOurVolume.h) / iTileL - 1;
    if(reps > 0) {  //Entirely possible for reps == 0: top-south edge takes up an entire tile length
        addStrip(Rect(x,y,w,l), HF_BTM, reps);
    }
    
    Rect rcTopNorth = Rect(bxOurVolume.x, bxOurVolume.y + Z_TO_Y(bxOurVolume.z + bxOurVolume.h) - iTileL, bxOurVolume.w, iTileL),
         rcTopEast  = Rect(),
         rcTopSouth = Rect(),
         rcBtmSouth = Rect(),
         rcTopWest  = Rect();
    
    //Actually, better way: Divide volume into strips, and iterate over them.
    // You can do special handling when you get to edge ones.
}

void EdgeRenderModel::updateHorizEdge(int list, int frame) {
    float fTileL = m_pImageHEdges->h;
    Box bxOurVolume = m_pEdgeList->getVolume(),
        bxTheirVolume;
    float x, y;
    int w, l;

    m_pEdgeList->setList(list);
    //y values will be the same for all horiz edges
    y = bxOurVolume.y - fTileL;
    l = fTileL;
    while(m_pEdgeList->hasNext()) {
        bxTheirVolume = m_pEdgeList->nextVolume();
        //Only render an edge if our volume's top is above theirs. If they are
        // equal, no edge should be rendered. We'll work on corners once the
        // edges themselves work; it will probably involve tracking info about
        // the last edge rendered
        if(bxOurVolume.z + bxOurVolume.h > bxTheirVolume.z + bxTheirVolume.h) {
            //Calculate the render area
            if(bxOurVolume.x < bxTheirVolume.x) {
                x = bxTheirVolume.x;
            } else {
                x = bxOurVolume.x;
            }
            if(bxOurVolume.x + bxOurVolume.w > bxTheirVolume.x + bxTheirVolume.w) {
                w = bxTheirVolume.x + bxTheirVolume.w - x;
            } else {
                w = bxOurVolume.x + bxOurVolume.w - x;
            }
            addStrip(Rect(x,y,w,l), frame, 1);
        }
    }
}

void EdgeRenderModel::updateVertEdge(int list, int frame) {
    float fTileW = m_pImageVEdges->w;
    Box bxOurVolume = m_pEdgeList->getVolume(),
        bxTheirVolume;
    float x, y;
    int w, l;

    m_pEdgeList->setList(list);
    //x values will be the same for all horiz edges
    x = bxOurVolume.x - fTileW;
    w = fTileW;
    while(m_pEdgeList->hasNext()) {
        bxTheirVolume = m_pEdgeList->nextVolume();
        //Only render an edge if our volume's top is above theirs. If they are
        // equal, no edge should be rendered. We'll work on corners once the
        // edges themselves work; it will probably involve tracking info about
        // the last edge rendered
        if(bxOurVolume.z + bxOurVolume.h > bxTheirVolume.z + bxTheirVolume.h) {
            //Calculate the render area
            if(bxOurVolume.y < bxTheirVolume.y) {
                y += bxTheirVolume.y;
            } else {
                y += bxOurVolume.y;
            }
            if(bxOurVolume.y + bxOurVolume.l > bxTheirVolume.y + bxTheirVolume.l) {
                l += bxTheirVolume.y + bxTheirVolume.l - y;
            } else {
                l += bxOurVolume.y + bxOurVolume.l - y;
            }
            addStrip(Rect(x,y,w,l), frame, 0);
        }
    }
}

void EdgeRenderModel::render(RenderEngine *re) {
    Point ptScreenOffset = re->getRenderOffset();
    float //fTileW = m_pImageHEdges->w,
          fTileL = m_pImageVEdges->h;
    for(vector<Strip>::iterator it = m_vStrips.begin();
            it != m_vStrips.end();
            ++it) {
        if(it->m_iVReps <= 0) {
            //vertical strip
            renderStrip(it->m_iFrame, Rect(it->m_rcArea.x - ptScreenOffset.x,  it->m_rcArea.y - ptScreenOffset.y, it->m_rcArea.w, it->m_rcArea.l), VERT);
        } else {
            for(int i = 0; i < it->m_iVReps; ++i) {
                //horizontal strip, possibly repeated vertically
                renderStrip(it->m_iFrame, Rect(it->m_rcArea.x - ptScreenOffset.x,  it->m_rcArea.y + i * fTileL - ptScreenOffset.y, it->m_rcArea.w, it->m_rcArea.l), HORIZ);
            }
        }
    }

    //I'll worry about implementing corners some other time.
    // Probably best to create a sort of strip-list of rects that need to be rendered;
    // this only needs to be changed when someone alters a surface, which won't be very often.
    // Striplist struct:
    // Rect area of 1st strip
    // int number of times strip is repeated vertically.  If 0, horizontal
    // int frame
    /*
    Box bxOurVolume = m_pEdgeList->getVolume(),
        bxTheirVolume;
    float fTileW = m_pImageVEdges->w,
          fTileL = m_pImageHEdges->h;
    float x, y;
    int w, h;

    //North border
    m_pEdgeList->setList(NORTH);
    //y values will be the same for all north edges
    y = bxOurVolume.y - ptScreenOffset.y - fTileL;
    l = fTileL;
    while(m_pEdgeList->hasNext()) {
        bxTheirVolume = m_pEdgeList->nextVolume();
        //Only render an edge if our volume's top is above theirs. If they are
        // equal, no edge should be rendered. We'll work on corners once the
        // edges themselves work; it will probably involve tracking info about
        // the last edge rendered
        if(bxOurVolume.z + bxOurVolume.h > bxTheirVolume.z + bxTheirVolume.h) {
            //Calculate the render area
            x = -ptScreenOffset.x;
            if(bxOurVolume.x < bxTheirVolume.x) {
                x += bxTheirVolume.x;
            } else {
                x += bxOurVolume.x;
            }
            w = -ptScreenOffset.x;
            if(bxOurVolume.x + bxOurVolume.w > bxTheirVolume.x + bxTheirVolume.w) {
                w = bxTheirVolume.x + bxTheirVolume.w - x;
            } else {
                w = bxOurVolume.x + bxOurVolume.w - x;
            }
            renderStrip(HF_TOP_NORTH, Rect(x,y,w,l));
        }
    }
    */
/*
    //Get the temporary texture rectangle
    float fTexLeft   = m_iFrameW * 1.0F / m_pImage->m_iNumFramesW,
          fTexTop    = m_iFrameH * 1.0F / m_pImage->m_iNumFramesH,
          fTexRight  = m_iFrameW * 1.0F / m_pImage->m_iNumFramesW + m_iRepsW * 1.0F / m_pImage->m_iNumFramesW,
          fTexBottom = m_iFrameH * 1.0F / m_pImage->m_iNumFramesH + m_iRepsH * 1.0F / m_pImage->m_iNumFramesH;

    //Get the temporary drawing rectangle
    float fDrawLeft   = m_rcDrawArea.x - ptScreenOffset.x,
          fDrawTop    = m_rcDrawArea.y - ptScreenOffset.y,
          fDrawRight  = m_rcDrawArea.x - ptScreenOffset.x + m_rcDrawArea.w,
          fDrawBottom = m_rcDrawArea.y - ptScreenOffset.y + m_rcDrawArea.l;

    //Bind the texture to which subsequent calls refer to
    glBindTexture( GL_TEXTURE_2D, m_pImage->m_uiTexture );

    glBegin( GL_QUADS );
        //Top-left vertex (corner)
        glTexCoord2f(fTexLeft, fTexTop);
        glVertex3f(fDrawLeft, fDrawTop, 0.0f);

        //Top-right vertex (corner)
        glTexCoord2f(fTexRight, fTexTop);
        glVertex3f(fDrawRight, fDrawTop, 0.f);

        //Bottom-right vertex (corner)
        glTexCoord2f(fTexRight, fTexBottom);
        glVertex3f(fDrawRight, fDrawBottom, 0.f);

        //Bottom-left vertex (corner)
        glTexCoord2f(fTexLeft, fTexBottom);
        glVertex3f(fDrawLeft, fDrawBottom, 0.f);
    glEnd();
*/
}

void EdgeRenderModel::renderStrip(int iFrame, Rect rcDA, bool bHoriz) {

    uint uiTexture;
    float fTexLeft, fTexTop, fTexRight, fTexBottom;
    if(bHoriz) {
        uiTexture = m_pImageHEdges->m_uiTexture;
        fTexLeft   = 0;
        fTexTop    = iFrame * 1.0F / m_pImageHEdges->m_iNumFramesH;
        fTexRight  = rcDA.w * 1.0F / m_pImageHEdges->w;
        fTexBottom = fTexTop + 1.0F / m_pImageHEdges->m_iNumFramesH;
    } else {
        uiTexture = m_pImageVEdges->m_uiTexture;
        fTexLeft   = iFrame * 1.0F / m_pImageVEdges->m_iNumFramesW;
        fTexTop    = 0;
        fTexRight  = fTexLeft + 1.0F / m_pImageVEdges->m_iNumFramesW;
        fTexBottom = rcDA.l * 1.0F / m_pImageVEdges->h;
    }
    //Bind the texture to which subsequent calls refer to
    glBindTexture( GL_TEXTURE_2D, uiTexture );

    glBegin( GL_QUADS );
        //Top-left vertex (corner)
        glTexCoord2f(fTexLeft, fTexTop);
        glVertex3f(rcDA.x, rcDA.y, 0.0f);

        //Top-right vertex (corner)
        glTexCoord2f(fTexRight, fTexTop);
        glVertex3f(rcDA.x + rcDA.w, rcDA.y, 0.f);

        //Bottom-right vertex (corner)
        glTexCoord2f(fTexRight, fTexBottom);
        glVertex3f(rcDA.x + rcDA.w, rcDA.y + rcDA.l, 0.f);

        //Bottom-left vertex (corner)
        glTexCoord2f(fTexLeft, fTexBottom);
        glVertex3f(rcDA.x, rcDA.y + rcDA.l, 0.f);
    glEnd();
}
