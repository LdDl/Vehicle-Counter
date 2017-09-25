# VCounter
This project is targeted for counting vehicles by different approaches (HaarCascade, BackgroundSubstraction MOG2 and BackgroundSubstraction KNN) with OpenCV.

Grabbing and processing frames are separated into two threads (for improving perfomance, I hope...)

### Prerequisites
1) First of all you need installed OpenCV (https://github.com/opencv/opencv) for your platform. I've tested this project with version 3.3.0 (don't know how it works with version <3.3.0).

2) Also you need JSON parser for setting up initial parameters (such as "name of cascade", "scale factor of image" and etc.). I prefer this one https://github.com/nlohmann/json, because it is simple for usage (https://github.com/nlohmann/json#integration)

### Installing
When you're done with prerequisites (included OpenCV and JSON parser), just build project with any tool you familiar.

### Usage
You need to create configuration JSON-file with next fields:
```
{
	"video_filename": "your video file / rtsp link / webcam",
  
	"detector_type": "fmog2", //here are 3 options: haar_cascade, fmog2, fknn
  
	"cascade_path": "cascade_filename.xml", //in case of using Haar Cascade method you need to provide file with prepared cascade
  
	"line_position": { //Fixed horizontal line position
		"x": 0,
		"y": 350
	},
  
	"line_length": 5000, //line of length
  
	"scale_factor": 3, //scale factor
  
	"direction": true //if it is true -> count vehicles moving TO us; false -> count vehicles moving FROM us
}
```
Then you can start program executing command:
```
./main init_pars.json
```
Example for MOG2 (you can get example JSON and video-file from here: https://github.com/LdDl/Vehicle-Counter/tree/master/result):
```
Notice: There are a lot of false detections when we are not ignoring shadows. Image below.
```
![alt text](https://raw.githubusercontent.com/LdDl/Vehicle-Counter/master/result/img1.png)
```
Notice: There are better detections when ignoring shadows. Image below.
```
![alt text](https://raw.githubusercontent.com/LdDl/Vehicle-Counter/master/result/img3.png)

### TODO
 1. Add vertical borders for horizontal line.
 2. Better blobs drawing.
 3. Add some new detectors.
 4. Add shadows detector choice into JSON configuration.
 5. Test programm on several single board computers.
 
### P.S.
I'm stil working on this. Will be glad to read any ideas how to improve the project.
