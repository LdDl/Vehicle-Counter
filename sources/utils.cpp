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
    this->img_width = j["image_size"]["width"];
    this->img_height = j["image_size"]["height"];
    this->scaled_width = j["scaled_size"]["width"];
    this->scaled_height = j["scaled_size"]["height"];
    this->scale_factor = this->img_width / this->scaled_width;
    this->direction = j["direction"];
//    this->imshow_active = j["imshow_active"];
    if (this->detector_type == "haar_cascade") {
        string cascade_path = j["cascade_path"];
        if (!this->cascade_plates.load(cascade_path)) {
            cout << "Can not find cascade path: " << cascade_path << endl;
        }
    } else if (this->detector_type == "fmog2") {

    }
    this->angle_rotation = j["angle_rotation"];
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
    this->img_width = j["image_size"]["width"];
    this->img_height = j["image_size"]["height"];
    this->scaled_width = j["scaled_size"]["width"];
    this->scaled_height = j["scaled_size"]["height"];
    this->scale_factor = this->img_width / this->scaled_width;
    this->direction = j["direction"];
    this->imshow_active = j["imshow_active"];
    if (this->detector_type == "haar_cascade") {
        string cascade_path = j["cascade_path"];
        if (!this->cascade_plates.load(cascade_path)) {
            cout << "Can not find cascade path: " << cascade_path << endl;
        }
    } else if (this->detector_type == "fmog2") {

    }
    this->angle_rotation = j["angle_rotation"];
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
    cascade.detectMultiScale(eq_img, plates, 1.8, 3, 0, Size(15, 15)); //scale 1.2
    return plates;
}
