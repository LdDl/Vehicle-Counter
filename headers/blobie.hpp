#pragma once
#ifndef BLOBIE_HPP
#define BLOBIE_HPP

#include "utils.hpp"

class blobie {
    public:
        Rect currentBoundingRect;
        PointsVector currentContour;
        PointsVector track;
        double diagonal;
        double aspect_ratio;
        double area;
        bool isExists;
        bool isStillBeingTracked;
        int intNumOfConsecutiveFramesWithoutAMatch;
        Point predictedNextPosition;
        bool isCrossedTheLine(int &intHorizontalLinePosition, int &carCount, bool &direction);
        void drawTrack(Mat &src, string id);
        blobie(const PointsVector &_contour);
        blobie(const Rect &_contour);
        void predictNextPosition(void);
};

void matchCurrentFrameBlobsToExistingBlobs(vector<blobie> &existingBlobs, vector<blobie> &currentFrameBlobs);
void drawBlobInfoOnImage(vector<blobie> &blobs, Mat &imgFrame2Copy);

#endif // BLOBIE_HPP
