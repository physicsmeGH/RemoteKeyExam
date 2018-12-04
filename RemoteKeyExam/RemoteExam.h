#pragma once
#include "opencv2/core.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/features2d.hpp"
//#include "opencv2/xfeatures2d.hpp"
#include "opencv2/xfeatures2d/nonfree.hpp"

#include <filesystem>
#include <iostream>
#include <fstream>

using namespace cv;
using namespace cv::xfeatures2d;
using namespace std;
using namespace std::experimental::filesystem;

bool keyPointCompare(KeyPoint, KeyPoint);
void getCornerCircles(Mat, Point2f (&c)[4] );