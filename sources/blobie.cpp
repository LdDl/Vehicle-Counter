#include "blobie.hpp"

blobie::blobie(const PointsVector &_contour) {
    currentContour = _contour;
    currentBoundingRect = boundingRect(_contour);
    Point currentCenter;
    currentCenter.x = (currentBoundingRect.x + currentBoundingRect.x + currentBoundingRect.width) / 2;
    currentCenter.y = (currentBoundingRect.y + currentBoundingRect.y + currentBoundingRect.height) / 2;
    track.push_back(currentCenter);
    area = contourArea(_contour);
    diagonal = sqrt(pow(currentBoundingRect.width, 2) + pow(currentBoundingRect.height, 2));
    aspect_ration = (float)currentBoundingRect.width / (float)currentBoundingRect.height;
    blnStillBeingTracked = true;
    blnCurrentMatchFoundOrNewBlob = true;
    intNumOfConsecutiveFramesWithoutAMatch = 0;
}

blobie::blobie(const Rect &_contour) {
    currentBoundingRect = _contour;
    Point currentCenter;
    currentCenter.x = (currentBoundingRect.x + currentBoundingRect.x + currentBoundingRect.width) / 2;
    currentCenter.y = (currentBoundingRect.y + currentBoundingRect.y + currentBoundingRect.height) / 2;
    track.push_back(currentCenter);
    area = currentBoundingRect.area();
    diagonal = sqrt(pow(currentBoundingRect.width, 2) + pow(currentBoundingRect.height, 2));
    aspect_ration = (float)currentBoundingRect.width / (float)currentBoundingRect.height;
    blnStillBeingTracked = true;
    blnCurrentMatchFoundOrNewBlob = true;
    intNumOfConsecutiveFramesWithoutAMatch = 0;
}

void blobie::predictNextPosition(void) {
    int account = std::min(5, (int)track.size());
    auto prev = track.rbegin();
    auto current = prev;
    std::advance(current, 1);
    int deltaX = 0, deltaY = 0, sum = 0;
    for (int i = 1; i < account; ++i) {
        deltaX += (current->x - prev->x) * i;
        deltaY += (current->y - prev->y) * i;
        sum += i;
    }
    if (sum > 0) {
        deltaX /= sum;
        deltaY /= sum;
    }
    predictedNextPosition.x = track.back().x + deltaX;
    predictedNextPosition.y = track.back().y + deltaY;
}

void addBlobToExistingBlobs(blobie &currentFrameBlob, vector<blobie> &existingBlobs, int &intIndex) {
    existingBlobs[intIndex].currentBoundingRect = currentFrameBlob.currentBoundingRect;
    existingBlobs[intIndex].track.push_back(currentFrameBlob.track.back());
    existingBlobs[intIndex].diagonal = currentFrameBlob.diagonal;
    existingBlobs[intIndex].aspect_ration = currentFrameBlob.aspect_ration;
    existingBlobs[intIndex].blnStillBeingTracked = true;
    existingBlobs[intIndex].blnCurrentMatchFoundOrNewBlob = true;
}

void addNewBlob(blobie &currentFrameBlob, vector<blobie> &existingBlobs) {
    currentFrameBlob.blnCurrentMatchFoundOrNewBlob = true;
    existingBlobs.push_back(currentFrameBlob);
}

void matchCurrentFrameBlobsToExistingBlobs(std::vector<blobie> &existingBlobs, std::vector<blobie> &currentFrameBlobs) {
    for (auto &existingBlob : existingBlobs) {
        existingBlob.blnCurrentMatchFoundOrNewBlob = false;
        existingBlob.predictNextPosition();
    }
    for (auto &currentFrameBlob : currentFrameBlobs) {
        int intIndexOfLeastDistance = 0;
        double dblLeastDistance = 200000.0;
        for (unsigned int i = 0; i < existingBlobs.size(); i++) {
            if (existingBlobs[i].blnStillBeingTracked == true) {
                double dblDistance = distanceBetweenPoints(currentFrameBlob.track.back(), existingBlobs[i].predictedNextPosition);
                if (dblDistance < dblLeastDistance) {
                    dblLeastDistance = dblDistance;
                    intIndexOfLeastDistance = i;
                }
            }
        }
        if (dblLeastDistance < currentFrameBlob.diagonal * 0.5) {
            addBlobToExistingBlobs(currentFrameBlob, existingBlobs, intIndexOfLeastDistance);
        }
        else {
            addNewBlob(currentFrameBlob, existingBlobs);
        }
    }
    for (auto &existingBlob : existingBlobs) {
        if (existingBlob.blnCurrentMatchFoundOrNewBlob == false) {
            existingBlob.intNumOfConsecutiveFramesWithoutAMatch++;
        }
        if (existingBlob.intNumOfConsecutiveFramesWithoutAMatch >= 5) {
            existingBlob.blnStillBeingTracked = false;
        }
    }
}

bool blobie::checkIfBlobsCrossedTheLine(int &intHorizontalLinePosition, int &carCount) {
    if (this->blnStillBeingTracked == true && this->track.size() >=2) {
        int prevFrame = (int)this->track.size() - 2;
        int currFrame = (int)this->track.size() - 1;
        if (this->track[prevFrame].y <= intHorizontalLinePosition && this->track[currFrame].y > intHorizontalLinePosition) {
            carCount++;
            return true;
        }
    }
    return false;
}

void blobie::drawBlobTrack(cv::Mat &src) {
    if(this->blnStillBeingTracked == true) {
        for (auto &center : this->track) {
            cv::circle(src, center, 4, cv::Scalar(0, 0, 255), 1);
        }
    }
}

void drawBlobInfoOnImage(vector<blobie> &blobs, Mat &imgFrame2Copy) {
    vector<PointsVector> contours_poly(blobs.size());
    vector<float>radius(blobs.size());
    vector<cv::Point2f>center(blobs.size());
    for (unsigned int i = 0; i < blobs.size(); i++) {
        cv::approxPolyDP(Mat(blobs[i].currentContour), contours_poly[i], 3, true );
        cv::minEnclosingCircle((Mat)contours_poly[i], center[i], radius[i]);
    }
    if (blobs.size() > 1) {
        for (unsigned int i = 0; i < blobs.size(); i++) {
                //rectangle(imgFrame2Copy, blobs[i].currentBoundingRect, SCALAR_RED, 2);
            if (blobs[i].blnStillBeingTracked == true) {
                rectangle(imgFrame2Copy, blobs[i].currentBoundingRect, Scalar(0, 0, 255), 2);
                circle(imgFrame2Copy, center[i], (int)radius[i], Scalar(0, 255, 0), 2);
            }
        }
    }
}
