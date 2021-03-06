// RemoteKeyExam.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"


#include "RemoteExam.h"




int main()
{
	Mat standardImage = imread("C:/tensorflow/remote_image/good/11.bmp", IMREAD_GRAYSCALE);
	Mat test;
	absdiff(standardImage, standardImage, test);

	namedWindow("blob", WINDOW_KEEPRATIO);
	cv::imshow("blob", standardImage);
	namedWindow("warp", WINDOW_KEEPRATIO);

	ofstream fileWrite;
	fileWrite.open("C:/tensorflow/remote_image/bad.txt");
	
	

	string bad_dir = "C:/tensorflow/remote_image/bad/mask/";
	string keys_dir = "C:/tensorflow/remote_image/keyAvg/";
	for (auto& a : directory_iterator(bad_dir)) {
		Mat	remotePic1 = imread(a.path().string(), IMREAD_GRAYSCALE);
		cout << "image: " << a.path().filename().string() << endl;
		fileWrite << "image: " << a.path().filename().string() << endl;
		int imageCount = 0;
		
		for (auto& k : directory_iterator(bad_dir)) {
			Mat buttonPic = imread(k.path().string(), IMREAD_GRAYSCALE);
			cout << "key: " << k.path().filename().string() << endl;
			fileWrite << "key: " << k.path().filename().string() << endl;
			Mat buttonAvg = Mat::zeros(buttonPic.size(), CV_64F);
			Mat matchResult;
			matchTemplate(remotePic1, buttonPic, matchResult, TM_CCORR_NORMED);
			double minVal, maxVal, minValWarp, maxValWarp;
			Point minPoint, maxPoint, minPointWarp, maxPointWarp;
			minMaxLoc(matchResult, &minVal, &maxVal, &minPoint, &maxPoint);
			
			Rect sub = Rect(maxPoint.x, maxPoint.y, buttonPic.size().width, buttonPic.size().height);
			Mat buttonMatch = remotePic1(sub).clone();

			//feature matching and homography
			Mat templateBin;
			threshold(buttonPic, templateBin, 100, 255, THRESH_BINARY);
			Mat matchBin;
			threshold(buttonMatch, matchBin, 100, 255, THRESH_BINARY);
			int minHessian = 400;
			Ptr<SURF> detector = SURF::create(minHessian);
			vector<KeyPoint> keyPointsTemplate, keyPointsMatch;
			Mat descriptorsTemplate, descriptorsMatch;
			detector->detectAndCompute(templateBin, noArray(), keyPointsTemplate, descriptorsTemplate);
			detector->detectAndCompute(matchBin, noArray(), keyPointsMatch, descriptorsMatch);
			
			Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::FLANNBASED);
			vector<vector<DMatch>> knn_matches;
			matcher->knnMatch(descriptorsMatch, descriptorsTemplate, knn_matches, 2);

			const float ratio_threshold = 0.20f;
			vector<DMatch> good_matches;
			for (size_t i = 0; i < knn_matches.size(); i++) {
				if (knn_matches[i][0].distance < ratio_threshold * knn_matches[i][1].distance) {
					good_matches.push_back(knn_matches[i][0]);
				}
			}

			Mat button_matches;
			drawMatches(matchBin, keyPointsMatch, templateBin, keyPointsTemplate, good_matches, button_matches, Scalar::all(-1), Scalar::all(-1), vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
			imshow("warp", button_matches);
			waitKey(1);

			vector<Point2f> good_keyPointsMatch, good_keyPointsTemplate;
			Mat buttonTransform, templateWarp;
			for (DMatch m : good_matches) {
				good_keyPointsMatch.push_back(keyPointsMatch[m.queryIdx].pt);
				good_keyPointsTemplate.push_back(keyPointsTemplate[m.trainIdx].pt);
			}
			if (good_keyPointsMatch.size() > 5) {
				
				buttonTransform = findHomography(good_keyPointsTemplate, good_keyPointsMatch, CV_RANSAC);
				
				warpPerspective(buttonPic, templateWarp, buttonTransform, buttonPic.size(), INTER_LINEAR, BORDER_REPLICATE);
				matchTemplate(remotePic1, templateWarp, matchResult, TM_CCORR_NORMED);
				minMaxLoc(matchResult, &minValWarp, &maxValWarp, &minPointWarp, &maxPointWarp);
				
			}
			else {
				maxValWarp = -1;
			}
			
			
			cout << "key: " << k.path().filename().string() << " max: " << maxPoint << " maxVal: " << maxVal <<" maxWarp: "<< maxValWarp<< endl;
			fileWrite << "key: " << k.path().filename().string() << " max: " << maxPoint << " maxVal: " << maxVal << " maxWarp: " << maxValWarp << endl;


			//waitKey(1);
			


			buttonMatch.convertTo(buttonMatch, CV_64F);
			buttonAvg += buttonMatch;

			
			int channels[] = { 0 };
			int histSize[] = { 50 };
			float ranges1[] = { 0,1 };
			const float* ranges[] = { ranges1 };
			Mat histogram;
			cv::calcHist(&matchResult, 1, channels, Mat(), histogram, 1, histSize, ranges, true, false);
			cv::imshow("warp", matchResult);

			imageCount++;
		}
		//buttonAvg /= imageCount;
		//imwrite(k.path().parent_path().parent_path().append("/keyAvg/").append(k.path().filename()).string(), buttonAvg);
	}
	fileWrite.close();
	
	Point2f standardCircles[4];
	getCornerCircles(standardImage, standardCircles);
	Mat standardMask = imread("C:/tensorflow/remote_image/remote standard3.bmp", IMREAD_GRAYSCALE);
	Mat  testImage = imread("C:/tensorflow/remote_image/good/23.bmp", IMREAD_GRAYSCALE);;
	
	Mat outImage;
	Point2f testCircles[4];
	//getCornerCircles(testImage, testCircles);

	Mat transform;
	//= getPerspectiveTransform(testCircles, standardCircles);
	/*warpPerspective(testImage, outImage, transform, testImage.size());
	imshow("warp", outImage);*/

	string good_dir = "C:/tensorflow/remote_image/bad/";
	for (auto& a : directory_iterator(good_dir)) {
		//rotating images in a folder to make them overlap with template keys

		Mat image = imread(a.path().string(), IMREAD_GRAYSCALE);
		cv::setWindowTitle("warp", a.path().string());
		if (image.size().empty()) {
			continue;
		}
		getCornerCircles(image, testCircles);
		//waitKey(0);
		transform = getPerspectiveTransform(testCircles, standardCircles);
		warpPerspective(image, outImage, transform, image.size());
		
		//(Mat::ones(Size(image.size().width, image.size().height), CV_8U)*255 - standardMask)/255*200;
		outImage = outImage.mul(standardMask / 255.0) + (Mat::ones(Size(image.size().width, image.size().height), CV_8U) * 255 - standardMask) / 255 * 200;
		cv::imshow("warp", outImage);
		
		imwrite(a.path().parent_path().append("/mask/").append(a.path().filename()).string(), outImage);
		waitKey(1);
	}

	waitKey(0);
    std::cout << "Hello World!\n"; 
}

bool keyPointCompare(KeyPoint p1, KeyPoint p2) 
{ 
	return (p1.pt.x < p2.pt.x); 
}



void getCornerCircles(Mat image, Point2f(&corners)[4])
{
	SimpleBlobDetector::Params blobParams;
	blobParams.minThreshold = 80;
	blobParams.maxThreshold = 255;
	blobParams.filterByColor = false;
	blobParams.filterByArea = true;
	blobParams.minArea = 4000;
	blobParams.maxArea = 6000;
	blobParams.filterByCircularity = false;
	blobParams.minCircularity = 0.6;
	blobParams.filterByInertia = true;
	blobParams.minInertiaRatio = 0.7;
	blobParams.maxInertiaRatio = 1.0;
	blobParams.filterByConvexity = true;
	blobParams.minConvexity = 0.7;
	blobParams.maxConvexity = 1.0;

	Ptr<SimpleBlobDetector> blobDetector1 = SimpleBlobDetector::create(blobParams);
	//blobDetector1.create(blobParams);
	vector<KeyPoint> circles;
	blobDetector1->detect(image, circles);
	drawKeypoints(image, circles, image, Scalar(0, 0, 255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	cv::imshow("warp", image);
	sort(circles.begin(), circles.end(), keyPointCompare);
	int circleCount = circles.size();
	corners[0] = Point2f(circles.at(0).pt.x, circles.at(0).pt.y);
	corners[1] = Point2f(circles.at(1).pt.x, circles.at(1).pt.y);
	corners[2] = Point2f(circles.at(circleCount-2).pt.x, circles.at(circleCount - 2).pt.y);
	corners[3] = Point2f(circles.at(circleCount - 1).pt.x, circles.at(circleCount - 1).pt.y);
	Point2f temp;

	if (corners[0].y > corners[1].y) {
		temp = Point2f(corners[0]);
		corners[0] = Point2f(corners[1]);
		corners[1] = Point2f(temp);
	}

	if (corners[2].y > corners[3].y) {
		temp = Point2f(corners[2]);
		corners[2] = Point2f(corners[3]);
		corners[3] = Point2f(temp);
	}

	if ((corners[0] - corners[1]).dot(corners[0] - corners[1]) > (corners[2] - corners[3]).dot(corners[2] - corners[3])) {
		temp = Point2f(corners[0]);
		corners[0] = Point2f(corners[3]);
		corners[3] = Point2f(temp);
		temp = Point2f(corners[1]);
		corners[1] = Point2f(corners[2]);
		corners[2] = Point2f(temp);
	}
}
// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file


