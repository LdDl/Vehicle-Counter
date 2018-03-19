#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>

#include "headers/blobie.hpp"

#include "headers/fifo_buffer.hpp"
#include "headers/framedata.hpp"

//Global variables
FiFoBuffer<FrameData> buff(60);
atomic<bool> grabbing;
atomic<bool> processing;
Mat fgMaskMOG2;
Mat fgMaskKNN;
Ptr<BackgroundSubtractorMOG2> pMOG2 = createBackgroundSubtractorMOG2(500, 36.0, true);
Ptr<BackgroundSubtractor> pKNN = createBackgroundSubtractorKNN(500, 36.0, true);
int carCounter = 0;

//Thread functions
void Grabber(InitialParameters &mp);
void Processing(InitialParameters &mp);
//Proccesing functions
void ProcessingDataMOG2(FrameData &fdata, vector<blobie> &blobies, InitialParameters &mp);
void ProcessingDataKNN(FrameData &fdata, vector<blobie> &blobies, InitialParameters &mp);
void ProcessingDataHaar(FrameData &fdata, vector<blobie> &blobies, InitialParameters &mp);

int main(int argc, char* argv[]) {

    InitialParameters mainParameters;
//    if (argc > 1) {
//        cout << "Using configuration file: " << argv[1] << endl;
//        mainParameters.SetParams(argv[1]);
//    } else {
//        cout << "No configuration file has been provided!" << endl;
//        return 0;
//    }

    mainParameters.SetParams("D:/hp/Documents/QtProjects/build-vcounter-Desktop_Qt_5_8_0_MSVC2015_64bit-Release/release/initial_params.json");
    cout << "Starting...\n";

    grabbing.store(true);
    processing.store(true);
    thread grab(Grabber, std::ref(mainParameters));
    thread proc(Processing, std::ref(mainParameters));

    cout << endl << "Press Enter to stop grabbing...\n";
    cin.get();

    grabbing.store(false);
    processing.store(false);
    grab.join();
    proc.join();

    cin.get();

    return 0;
}


void Grabber(InitialParameters &mp) {
    processing.store(true);
    VideoCapture vidSrc;
    FrameData frame;
    vidSrc.open(mp.videoSource);
    int frameCounter = 0;
    int defaultDelay = 100;
    int delay = defaultDelay;
    vidSrc >> frame.image;
    frame.image_truesize = frame.image.clone();
    Size imSrc_size(frame.image.cols / mp.scale_factor, frame.image.rows / mp.scale_factor);
    resize(frame.image, frame.image, imSrc_size);
    while (grabbing.load() == true) {
        vidSrc >> frame.image;
        frame.image_truesize = frame.image.clone();
        resize(frame.image, frame.image, imSrc_size);
        if (frame.image.empty()) {
            continue;
        }
        frameCounter++;
        frame.vars.timestamp = getTickCount();
        frame.vars.frameNum = frameCounter;
        if (!buff.Push(frame)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            delay += 80;
        } else {
            if (delay > defaultDelay) {
                delay = 100;
            }
        }
    }
    processing.store(false);
}

void Processing(InitialParameters &mp) {
    FrameData frame;
    vector<blobie> blobies;
    while (processing.load() == true) {
        if (buff.Pop(frame) == false) {
            continue;
        }
        if (mp.detector_type == "mog2") {
            ProcessingDataMOG2(frame, blobies, mp);
        } else if (mp.detector_type == "haar_cascade") {
            ProcessingDataHaar(frame, blobies, mp);
        } else if (mp.detector_type == "knn") {
            ProcessingDataKNN(frame, blobies, mp);
        } else {
            //...handle somehow
        }
    }
    size_t remaining = buff.Size();
    cout << endl << "Processing thread >> Flushing buffer: remaining " << remaining << " frames..." << endl;
    while (buff.Pop(frame)) {
        if (mp.detector_type == "mog2") {
            ProcessingDataMOG2(frame, blobies, mp);
        } else if (mp.detector_type == "haar_cascade") {
            ProcessingDataHaar(frame, blobies, mp);
        } else if (mp.detector_type == "knn") {
            ProcessingDataKNN(frame, blobies, mp);
        } else {
            //...handle somehow
        }
        cout << "remaining amount of frames: " << remaining-- << endl;
    }
    cout << "Processing thread: done!" << endl;
}

void ProcessingDataHaar(FrameData &fdata, vector<blobie> &blobies, InitialParameters &mp) {
    Mat imgCopy = fdata.image.clone();
    vector<blobie> currentFrameBlobs;
    vector<Rect> plates = detectRegionsOfInterest(imgCopy, mp.cascade_plates);
    line(imgCopy, mp.crossingLine[0], mp.crossingLine[1], Scalar(0, 0, 255), 3);
    if (plates.size() != 0 ) {
        for (auto &pl : plates) {
            Rect myROI(pl.x,  pl.y, pl.width, pl.height);
            blobie possibleBlob(myROI);
            currentFrameBlobs.push_back(possibleBlob);
        }
        if (mp.blnFirstFrame == true) {
            for (auto &currentFrameBlob : currentFrameBlobs) {
                blobies.push_back(currentFrameBlob);
            }
        } else {
            matchCurrentFrameBlobsToExistingBlobs(blobies, currentFrameBlobs);
        }
        for (auto &blob : blobies) {
            if (blob.isCrossedTheLine(mp.crossingLine[0].y, carCounter, mp.direction)) {
                cout << "unique object id: " << &blob - &blobies[0] << " has crossed the line!\n";
                int rx = blob.currentBoundingRect.x * mp.scale_factor;
                int ry = blob.currentBoundingRect.y * mp.scale_factor;
                int rwidth = blob.currentBoundingRect.width * mp.scale_factor;
                int rheight = blob.currentBoundingRect.height * mp.scale_factor;
                Rect objectCrop(rx, ry, rwidth, rheight);
                if (mp.imshow_active) {
                    double angle = mp.angle_rotation;
                    Mat croppedImg = fdata.image_truesize(objectCrop);
                    cv::Point2f center(croppedImg.cols/2.0, croppedImg.rows/2.0);
                    cv::Mat rot = cv::getRotationMatrix2D(center, angle, 1.0);
                    cv::Rect bbox = cv::RotatedRect(center, croppedImg.size(), angle).boundingRect();
                    rot.at<double>(0,2) += bbox.width/2.0 - center.x;
                    rot.at<double>(1,2) += bbox.height/2.0 - center.y;
                    cv::Mat dst;
                    cv::warpAffine(croppedImg, dst, rot, bbox.size());
                    imshow("plate for id: " + to_string(&blob - &blobies[0]), dst);
                } else {
                    //no imshow
                }
                line(imgCopy, mp.crossingLine[0], mp.crossingLine[1], Scalar(0, 255, 0), 15);
            } else {
                //nothing here
            }
            blob.drawTrack(imgCopy, to_string(&blob - &blobies[0]));
        }
        mp.blnFirstFrame = false;
    }

    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
    auto text = oss.str();
    putText(imgCopy, text, Point(50, 50), cv::FONT_HERSHEY_SCRIPT_SIMPLEX, 2, Scalar(0, 255, 255), 3, 8);
    if (mp.imshow_active) {
        imshow("src", imgCopy);
    } else {
        //no imshow
    }
    waitKey(1);
}

void ProcessingDataKNN(FrameData &fdata, vector<blobie> &blobies, InitialParameters &mp) {
    vector<blobie> blobies_currentFrame;
    Mat foreground;
    Mat &frame = fdata.image;
    pKNN->apply(frame, fgMaskKNN);
    erode(fgMaskKNN, foreground, cv::Mat());
    dilate(foreground, foreground, cv::Mat());
    foreground.setTo(0, foreground == 127); // Ignore shadow!
    std::vector<std::vector<cv::Point> > contours;
    findContours(foreground, contours,  CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
    drawContours(frame, contours, -1, cv::Scalar(0,0,255), 2);
    medianBlur(fgMaskKNN, fgMaskKNN, 5);
    if (mp.imshow_active) {
        imshow("gauss blur", fgMaskKNN);
    } else {
        // no imshow
    }
    morphologyEx(fgMaskKNN, fgMaskKNN, MORPH_CLOSE, getStructuringElement(MORPH_RECT, Size(5, 5)));
    morphologyEx(fgMaskKNN, fgMaskKNN, MORPH_OPEN, getStructuringElement(MORPH_RECT, Size(5, 5)));
    if (mp.imshow_active) {
        imshow("morphology", fgMaskKNN);
    } else {
        //no imshow
    }
    vector<vector<Point> > convexHulls(contours.size());
    for (unsigned int i = 0; i < contours.size(); i++) {
        convexHull(contours[i], convexHulls[i]);
    }
    for (auto &convexHull : convexHulls) {
        blobie possible_blobie(convexHull);
        if (possible_blobie.currentBoundingRect.width > 30 &&
                possible_blobie.currentBoundingRect.height > 30 &&
                possible_blobie.aspect_ratio >= 0.8 &&
                possible_blobie.aspect_ratio <= 2.0) {
            //
            blobies_currentFrame.push_back(possible_blobie);
        }
    }
    if (mp.blnFirstFrame == true) {
        for (auto &currentFrameBlob : blobies_currentFrame) {
            blobies.push_back(currentFrameBlob);
        }
    } else {
        matchCurrentFrameBlobsToExistingBlobs(blobies, blobies_currentFrame);
    }
    line(frame, mp.crossingLine[0], mp.crossingLine[1], Scalar(0, 0, 255), 3);
    for (auto &blob : blobies) {
        if (blob.isCrossedTheLine(mp.crossingLine[0].y, carCounter, mp.direction)) {
            cout << "unique object id: " << &blob - &blobies[0] << " has crossed the line!\n";
            line(frame, mp.crossingLine[0], mp.crossingLine[1], Scalar(0, 255, 0), 15);
        } else {
            //nothing here
        }
      blob.drawTrack(frame, to_string(&blob - &blobies[0]));
    }
    blobies_currentFrame.clear();
    mp.blnFirstFrame = false;
    if (mp.imshow_active) {
        imshow("MaskKNN", fgMaskKNN);
    } else {
        //no imshow
    }
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
    auto text = oss.str();
    putText(frame, text, Point(50, 50), cv::FONT_HERSHEY_SCRIPT_SIMPLEX, 2, Scalar(0, 255, 255), 3, 8);
    if (mp.imshow_active) {
        imshow("frame", frame);
    } else {
        //no imshow
    }
    waitKey(1);
}

void ProcessingDataMOG2(FrameData &fdata, vector<blobie> &blobies, InitialParameters &mp) {
    vector<blobie> blobies_currentFrame;
    Mat foreground;
    Mat &frame = fdata.image;
    pMOG2->apply(frame, fgMaskMOG2);
    erode(fgMaskMOG2, foreground, Mat());
    dilate(foreground, foreground, Mat());
    foreground.setTo(0, foreground == 127); // Ignore shadow!
    std::vector<PointsVector> contours;
    findContours(foreground, contours,  CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
    drawContours(frame, contours, -1, Scalar(0,0,255), 2);
    medianBlur(fgMaskMOG2, fgMaskMOG2, 5);
    if (mp.imshow_active) {
        imshow("gauss blur", fgMaskMOG2);
    } else {
        //no imshow
    }
    morphologyEx(fgMaskMOG2, fgMaskMOG2, MORPH_CLOSE, getStructuringElement(MORPH_RECT, Size(5, 5)));
    morphologyEx(fgMaskMOG2, fgMaskMOG2, MORPH_OPEN, getStructuringElement(MORPH_RECT, Size(5, 5)));
    if (mp.imshow_active) {
        imshow("morphology", fgMaskMOG2);
    } else {
        //no imshow
    }
    vector<vector<Point> > convexHulls(contours.size());
    for (unsigned int i = 0; i < contours.size(); i++) {
        convexHull(contours[i], convexHulls[i]);
    }
    for (auto &convexHull : convexHulls) {
        blobie possible_blobie(convexHull);
        if (possible_blobie.currentBoundingRect.width > 30 &&
                possible_blobie.currentBoundingRect.height > 30 &&
                possible_blobie.aspect_ratio >= 0.8 &&
                possible_blobie.aspect_ratio <= 2.0) {
            //
            blobies_currentFrame.push_back(possible_blobie);
        }
    }
    if (mp.blnFirstFrame == true) {
        for (auto &currentFrameBlob : blobies_currentFrame) {
            blobies.push_back(currentFrameBlob);
        }
    } else {
        matchCurrentFrameBlobsToExistingBlobs(blobies, blobies_currentFrame);
    }
    line(frame, mp.crossingLine[0], mp.crossingLine[1], Scalar(0, 0, 255), 3);
    for (auto &blob : blobies) {
        if (blob.isCrossedTheLine(mp.crossingLine[0].y, carCounter, mp.direction)) {
            cout << "unique object id: " << &blob - &blobies[0] << " has crossed the line!\n";
            line(frame, mp.crossingLine[0], mp.crossingLine[1], Scalar(0, 255, 0), 15);
        } else {
            //nothing here
        }
        blob.drawTrack(frame, to_string(&blob - &blobies[0]));
    }
    blobies_currentFrame.clear();
    mp.blnFirstFrame = false;
    if (mp.imshow_active) {
        imshow("MaskMOG2", fgMaskMOG2);
    } else {
        //no imshow
    }
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
    auto text = oss.str();
    putText(frame, text, Point(50, 50), cv::FONT_HERSHEY_SCRIPT_SIMPLEX, 2, Scalar(0, 255, 255), 3, 8);
    if (mp.imshow_active) {
        imshow("Frame", frame);
    } else {
        //no imshow
    }
    waitKey(1);
}
