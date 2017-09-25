#pragma once
#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <vector>
#include <atomic>
#include <thread>
#include <chrono>
#include <opencv2/opencv.hpp>
#include "json.hpp"

using std::cout;
using std::endl;
using std::cin;
using std::vector;
using std::string;
using std::thread;
using std::atomic;
using std::ref;
using std::to_string;
using std::min;

using cv::Mat;
using cv::Rect;
using cv::Point;
using cv::Point2f;
using cv::Size;
using cv::Scalar;
using cv::resize;
using cv::cvtColor;
using cv::equalizeHist;
using cv::morphologyEx;
using cv::MORPH_OPEN;
using cv::MORPH_CLOSE;
using cv::MORPH_RECT;
using cv::CascadeClassifier;
using cv::VideoCapture;
using cv::getTickCount;
using cv::imshow;
using cv::imread;
using cv::waitKey;
using cv::Ptr;
using cv::BackgroundSubtractorMOG2;
using cv::BackgroundSubtractor;
using cv::BackgroundSubtractorKNN;
using cv::createBackgroundSubtractorMOG2;
using cv::createBackgroundSubtractorKNN;

typedef vector<Point> PointsVector;

using json = nlohmann::json;

class InitialParameters {
    public:
        InitialParameters();
        InitialParameters(const char *path);
        string detector_type = "";
        string videoSource = "";
        int scale_factor = 1;
        bool direction = true; //true - ON us, false - FROM us
        CascadeClassifier cascade_plates;
        Point crossingLine[2];
        void SetParams(const char *path);
};

double distanceBetweenPoints(Point point1, Point point2);
vector<Rect> detectRegionsOfInterest(Mat &frame, CascadeClassifier &cascade);

#endif // UTILS_HPP