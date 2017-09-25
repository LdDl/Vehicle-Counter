#pragma once
#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include "json.hpp"

using std::cout;
using std::endl;
using std::vector;
using std::string;

using cv::Mat;
using cv::Rect;
using cv::Point;
using cv::Size;
using cv::Scalar;
using cv::resize;
using cv::cvtColor;
using cv::equalizeHist;
using cv::CascadeClassifier;
typedef vector<Point> PointsVector;

using json = nlohmann::json;

class InitialParameters {
    public:
        InitialParameters(const char *path);
        string detector_type = "";
        string videoSource = "";
        int scale_factor = 1;
        bool direction = true; //true - ON us, false - FROM us
        CascadeClassifier cascade_plates;
        Point crossingLine[2];
};

double distanceBetweenPoints(Point point1, Point point2);
vector<Rect> detectRegionsOfInterest(Mat &frame, CascadeClassifier &cascade);

#endif // UTILS_HPP
