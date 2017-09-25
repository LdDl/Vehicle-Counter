#include <iostream>
#include "blobie.hpp"

#include "fifo_buffer.hpp"
#include "framedata.hpp"
#include <atomic>

FiFoBuffer<FrameData> buff(60);
std::atomic<bool> grabbing;
std::atomic<bool> processing;

void Grabber();
void Processing();
void ProcessingDataMOG2(FrameData &fdata, vector<blobie> &blobies);
void ProcessingDataKNN(FrameData &fdata, vector<blobie> &blobies);
void ProcessingDataHaar(FrameData &fdata, vector<blobie> &blobies);

bool blnFirstFrame = true;
int carCounter = 0;

int main() {
    cout << "Start!\n";
    InitialParameters mainParameters("initial_params.json");
    return 0;
}
