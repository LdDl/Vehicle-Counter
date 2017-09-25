#pragma once
#ifndef BLOBIE_HPP
#define BLOBIE_HPP

#include <opencv2/opencv.hpp>
#include "utils.hpp"

class blobie {
    public:
        Rect currentBoundingRect;
        PointsVector currentContour;
        PointsVector track;
        double diagonal;
        double aspect_ration;
        double area;
        bool blnCurrentMatchFoundOrNewBlob;
        bool blnStillBeingTracked;
        int intNumOfConsecutiveFramesWithoutAMatch;
        Point predictedNextPosition;
        bool checkIfBlobsCrossedTheLine(int &intHorizontalLinePosition, int &carCount);
        void drawBlobTrack(Mat &src);
        blobie(const PointsVector &_contour);
        blobie(const Rect &_contour);
        void predictNextPosition(void);
};

#endif // BLOBIE_HPP
