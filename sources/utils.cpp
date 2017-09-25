#include "headers/utils.hpp"

InitialParameters::InitialParameters() {}

InitialParameters::InitialParameters(const char *path) {
    std::ifstream ifs(path);
    json j;
    ifs >> j;
    this->detector_type = j["detector_type"];
    this->videoSource = j["video_filename"];
    this->crossingLine[0].x = j["line_position"]["x"];
    this->crossingLine[0].y = j["line_position"]["y"];
    this->crossingLine[1].x = j["line_length"];
    this->crossingLine[1].x += this->crossingLine[0].x;
    this->crossingLine[1].y = this->crossingLine[0].y;
    this->scale_factor = j["scale_factor"];
    if (this->scale_factor <= 0) {
        this->scale_factor = 1;
        cout << "Bad scale factor, using default value: 1" << endl;
    } else {
        this->scale_factor = j["scale_factor"];
    }
    this->direction = j["direction"];

    if (this->detector_type == "haar_cascade") {
        string cascade_path = j["cascade_path"];
        if (!this->cascade_plates.load(cascade_path)) {
            cout << "Can not find cascade path: " << cascade_path << endl;
        }
    } else if (this->detector_type == "fmog2") {

    }
}

void InitialParameters::SetParams(const char *path) {
    std::ifstream ifs(path);
    json j;
    ifs >> j;
    this->detector_type = j["detector_type"];
    this->videoSource = j["video_filename"];
    this->crossingLine[0].x = j["line_position"]["x"];
    this->crossingLine[0].y = j["line_position"]["y"];
    this->crossingLine[1].x = j["line_length"];
    this->crossingLine[1].x += this->crossingLine[0].x;
    this->crossingLine[1].y = this->crossingLine[0].y;
    this->scale_factor = j["scale_factor"];
    if (this->scale_factor <= 0) {
        this->scale_factor = 1;
        cout << "Bad scale factor, using default value: 1" << endl;
    } else {
        this->scale_factor = j["scale_factor"];
    }
    this->direction = j["direction"];

    if (this->detector_type == "haar_cascade") {
        string cascade_path = j["cascade_path"];
        if (!this->cascade_plates.load(cascade_path)) {
            cout << "Can not find cascade path: " << cascade_path << endl;
        }
    } else if (this->detector_type == "fmog2") {

    }
}

double distanceBetweenPoints(Point point1, Point point2) {
    int intX = abs(point1.x - point2.x);
    int intY = abs(point1.y - point2.y);
    return (sqrt(pow(intX, 2) + pow(intY, 2)));
}

vector<Rect> detectRegionsOfInterest(Mat &frame, CascadeClassifier &cascade) {
    vector<Rect> plates;
    Mat frameClone = frame.clone();
    Mat gray_img;
    Mat eq_img;
    Size resized_image_size(frame.cols/1, frame.rows/1);
    resize(frameClone, frameClone, resized_image_size);
    cvtColor(frameClone, gray_img, CV_BGR2GRAY);
    equalizeHist(gray_img, eq_img);
    cascade.detectMultiScale(eq_img, plates, 1.18, 3, 0, Size(15, 15)); //scale 1.2

    return plates;
}
