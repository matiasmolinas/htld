/*  Copyright 2011 AIT Austrian Institute of Technology
*
*   This file is part of OpenTLD.
*
*   OpenTLD is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   OpenTLD is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with OpenTLD.  If not, see <http://www.gnu.org/licenses/>.
*
*/

/*
 * MedianFlowTracker.cpp
 *
 *  Created on: Nov 17, 2011
 *      Author: Georg Nebehay
 */

#include "MedianFlowTracker.h"

#include <cmath>

#include "FBTrack.h"

using namespace cv;

namespace tld
{

MedianFlowTracker::MedianFlowTracker()
{
    trackerBB = NULL;
}

MedianFlowTracker::~MedianFlowTracker()
{
    cleanPreviousData();
}

void MedianFlowTracker::cleanPreviousData()
{
    delete trackerBB;
    trackerBB = NULL;
}

#ifdef USE_HTLD
	FAST_TR_INIT_PARAMS* MedianFlowTracker::createFastTrInitStr() {
		
		FAST_TR_INIT_PARAMS *fti = (FAST_TR_INIT_PARAMS*)malloc(sizeof(FAST_TR_INIT_PARAMS));
        fti->use_initial_flows   = true;
        fti->win_size.width      = 4;
        fti->win_size.height     = 4;
        fti->iters               = 20;
        fti->max_pyr_level       = 5;
		return fti;
	}
#endif

#ifndef USE_HTLD
	void MedianFlowTracker::track(const cv::Mat &prevMat, const cv::Mat &currMat, cv::Rect *prevBB)
#else
	void MedianFlowTracker::track(const cv::Mat &prevMat, 
		                          const cv::Mat &currMat, 
								  cv::Rect *prevBB,
								  FAST_TRACKING_STR *fastTrStr)
#endif
{
    if(prevBB != NULL)
    {
        if(prevBB->width <= 0 || prevBB->height <= 0)
        {
            return;
        }

        float bb_tracker[] = {prevBB->x, prevBB->y, prevBB->width + prevBB->x - 1, prevBB->height + prevBB->y - 1};
        float scale;

        IplImage prevImg = prevMat;
        IplImage currImg = currMat;
#ifndef USE_HTLD
        int success = fbtrack(&prevImg, &currImg, bb_tracker, bb_tracker, &scale);
#else
		int success = fbtrack(&prevImg, 
			                  &currImg, 
							  bb_tracker, 
							  bb_tracker, 
							  &scale, 
							  fastTr,
							  fastTrStr);
#endif

        //Extract subimage
        float x, y, w, h;
        x = floor(bb_tracker[0] + 0.5);
        y = floor(bb_tracker[1] + 0.5);
        w = floor(bb_tracker[2] - bb_tracker[0] + 1 + 0.5);
        h = floor(bb_tracker[3] - bb_tracker[1] + 1 + 0.5);

        //TODO: Introduce a check for a minimum size
        if(!success || x < 0 || y < 0 || w <= 0 || h <= 0 || x + w > currMat.cols || y + h > currMat.rows || x != x || y != y || w != w || h != h) //x!=x is check for nan
        {
            //Leave it empty
        }
        else
        {
            trackerBB = new Rect(x, y, w, h);
        }
    }
}

} /* namespace tld */


