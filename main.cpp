#include <iostream>
#include "blobie.hpp"

#include "fifo_buffer.hpp"
#include "framedata.hpp"

//Global variables
FiFoBuffer<FrameData> buff(60);
atomic<bool> grabbing;
atomic<bool> processing;
Mat fgMaskMOG2;
Mat fgMaskKNN;
Ptr<BackgroundSubtractorMOG2> pMOG2 = createBackgroundSubtractorMOG2(500, 36.0, false);
Ptr<BackgroundSubtractor> pKNN = createBackgroundSubtractorKNN();
bool blnFirstFrame = true;
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
    if (argc > 1) {
        cout << "Using configuration file: " << argv[1] << endl;
        mainParameters.SetParams(argv[1]);
    } else {
        cout << "No configuration file has been provided!" << endl;
        return 0;
    }

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
        if (mp.detector_type == "fmog2") {
            ProcessingDataMOG2(frame, blobies, mp);
        } else if (mp.detector_type == "haar_cascade") {
            ProcessingDataHaar(frame, blobies, mp);
        } else if (mp.detector_type == "fknn") {
            ProcessingDataKNN(frame, blobies, mp);
        } else {
            //...handle somehow
        }
    }
    size_t remaining = buff.Size();
    cout << endl << "Processing thread >> Flushing buffer: remaining " << remaining << " frames..." << endl;
    while (buff.Pop(frame)) {
        if (mp.detector_type == "fmog2") {
            //ProcessingDataMOG2(frame);
            ProcessingDataMOG2(frame, blobies, mp);
        } else if (mp.detector_type == "haar_cascade") {
            ProcessingDataHaar(frame, blobies, mp);
        } else if (mp.detector_type == "fknn") {
            ProcessingDataKNN(frame, blobies, mp);
        } else {
            //...handle somehow
        }
        cout << "remaining: " << remaining-- << endl;
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
        if (blnFirstFrame == true) {
            for (auto &currentFrameBlob : currentFrameBlobs) {
                blobies.push_back(currentFrameBlob);
            }
        } else {
            matchCurrentFrameBlobsToExistingBlobs(blobies, currentFrameBlobs);
        }
        for (auto &blob : blobies) {
            if (blob.isCrossedTheLine(mp.crossingLine[0].y, carCounter)) {
                cout << "unique object id: " << &blob - &blobies[0] << " has crossed the line!\n";
                int rx = blob.currentBoundingRect.x * mp.scale_factor;
                int ry = blob.currentBoundingRect.y * mp.scale_factor;
                int rwidth = blob.currentBoundingRect.width * mp.scale_factor;
                int rheight = blob.currentBoundingRect.height * mp.scale_factor;
                Rect objectCrop(rx, ry, rwidth, rheight);
                imshow("plate for id: " + to_string(&blob - &blobies[0]), fdata.image_truesize(objectCrop));
            }
            blob.drawTrack(imgCopy, to_string(&blob - &blobies[0]));
        }
        blnFirstFrame = false;
    }
    imshow("src", imgCopy);
    waitKey(1);
}


void ProcessingDataKNN(FrameData &fdata, vector<blobie> &blobies, InitialParameters &mp) {
    vector<blobie> blobies_currentFrame;
    Mat foreground;
    Mat &frame = fdata.image;
    pKNN->apply(frame, fgMaskKNN);
    erode(fgMaskKNN, foreground, cv::Mat());
    dilate(foreground, foreground, cv::Mat());
    std::vector<std::vector<cv::Point> > contours;
    findContours(foreground, contours,  CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
    drawContours(frame, contours, -1, cv::Scalar(0,0,255), 2);

    medianBlur(fgMaskKNN, fgMaskKNN, 5);
    imshow("gauss blur", fgMaskKNN);
    morphologyEx(fgMaskKNN, fgMaskKNN, MORPH_CLOSE, getStructuringElement(MORPH_RECT, Size(5, 5)));
    morphologyEx(fgMaskKNN, fgMaskKNN, MORPH_OPEN, getStructuringElement(MORPH_RECT, Size(5, 5)));
    imshow("morphology", fgMaskKNN);

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

    if (blnFirstFrame == true) {
        for (auto &currentFrameBlob : blobies_currentFrame) {
            blobies.push_back(currentFrameBlob);
        }
    } else {
        matchCurrentFrameBlobsToExistingBlobs(blobies, blobies_currentFrame);
    }

    line(frame, mp.crossingLine[0], mp.crossingLine[1], Scalar(0, 0, 255), 3);
    for (auto &blob : blobies) {
        if (blob.isCrossedTheLine(mp.crossingLine[0].y, carCounter)) {
            cout << "crossed" << endl;
            line(frame, mp.crossingLine[0], mp.crossingLine[1], Scalar(0, 255, 0), 3);
        } else {
      }
      blob.drawTrack(frame, to_string(&blob - &blobies[0]));
    }

    blobies_currentFrame.clear();
    blnFirstFrame = false;
    imshow("fgMaskKNN", fgMaskKNN);
    imshow("frame", frame);
    waitKey(1);
}

void ProcessingDataMOG2(FrameData &fdata, vector<blobie> &blobies, InitialParameters &mp) {
    vector<blobie> blobies_currentFrame;
    Mat foreground;
    Mat &frame = fdata.image;
    pMOG2->apply(frame, fgMaskMOG2);
    erode(fgMaskMOG2, foreground, Mat());
    dilate(foreground, foreground, Mat());
    std::vector<PointsVector> contours;
    findContours(foreground, contours,  CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
    drawContours(frame, contours, -1, Scalar(0,0,255), 2);

    medianBlur(fgMaskMOG2, fgMaskMOG2, 5);
    imshow("gauss blur", fgMaskMOG2);
    morphologyEx(fgMaskMOG2, fgMaskMOG2, MORPH_CLOSE, getStructuringElement(MORPH_RECT, Size(5, 5)));
    morphologyEx(fgMaskMOG2, fgMaskMOG2, MORPH_OPEN, getStructuringElement(MORPH_RECT, Size(5, 5)));
    imshow("morphology", fgMaskMOG2);

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

    if (blnFirstFrame == true) {
        for (auto &currentFrameBlob : blobies_currentFrame) {
            blobies.push_back(currentFrameBlob);
        }
    } else {
        matchCurrentFrameBlobsToExistingBlobs(blobies, blobies_currentFrame);
    }

    line(frame, mp.crossingLine[0], mp.crossingLine[1], Scalar(0, 0, 255), 3);
    for (auto &blob : blobies) {
        if (blob.isCrossedTheLine(mp.crossingLine[0].y, carCounter)) {
            cout << "crossed" << endl;
            line(frame, mp.crossingLine[0], mp.crossingLine[1], Scalar(0, 255, 0), 3);
        } else {
        }
        blob.drawTrack(frame, to_string(&blob - &blobies[0]));
    }

    blobies_currentFrame.clear();
    blnFirstFrame = false;

    imshow("fgMaskMOG2", fgMaskMOG2);
    imshow("frame", frame);
    waitKey(1);
}
