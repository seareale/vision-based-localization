#include "knuCamera.h"

KCvCamera::KCvCamera() {
}

KCvCamera::~KCvCamera() {
}

void KCvCamera::close() {
	mCapture.release();
}

bool KCvCamera::init(int camNum_get, bool bThreadMode) {
	if(mCapture.open(camNum_get)){
		camNum = camNum_get;
		threadMode = bThreadMode;	
		if(bThreadMode){
			std::thread thread1(&KCvCamera::getCamImage_thread, this);
			thread1.detach();
		}
		return true;
	}
	return false;
}

void KCvCamera::getCamImage_thread(){
	while(isOpened()){
		mtx.lock();
		mCapture >> cpyImg;
		mtx.unlock();

		usleep(30);
	}
}

bool KCvCamera::getCamImage(cv::Mat &img) {
	if(isOpened()){
		if(threadMode){
			mtx.lock();
			img = cpyImg;
			mtx.unlock();
		}
		else{
			mCapture >> img;
		}
		return true;
	}
	return false;
}

bool KCvCamera::getArucoMarkers(cv::Mat img, vector< int > &ids, vector< vector< Point2f > > &corners) {
	if(img.empty()) return false;

	vector< vector< Point2f > > rejected;
	aruco::detectMarkers(img, dictionary, corners, ids, detectorParams, rejected);

	if(corners.size() == 0) return false;
	
	return true;
}

bool KCvCamera::getMarkerPose(int markId, glm::mat4 &camPose, cv::Mat &img, int cols) {
	int i;
	cv::Mat imgOri;
	cv::Mat matRvec, matTvec, matR;

	cv::Mat matRvec_tmp, matTvec_tmp;
	
	vector< int > ids;
	vector< vector< Point2f > > corners;

	if(!getCamImage(imgOri) || imgOri.empty()) return false;
	if(!getArucoMarkers(imgOri, ids, corners)) return false;


	// read calibration info
	Mat camMatrix, distCoeffs;
	FileStorage fs("../calibration/output.txt", FileStorage::READ);
	if (!fs.isOpened()) return false;
	fs["camera_matrix"] >> camMatrix;
	fs["distortion_coefficients"] >> distCoeffs;

	vector<cv::Point3f> markerCorners3d;
	markerCorners3d.push_back(cv::Point3f(-0.5f, 0.5f, 0));
	markerCorners3d.push_back(cv::Point3f(0.5f, 0.5f, 0));
	markerCorners3d.push_back(cv::Point3f(0.5f, -0.5f, 0));
	markerCorners3d.push_back(cv::Point3f(-0.5f, -0.5f, 0));

	for (i = 0; i < int(corners.size()); i++) {
		// calculate only markId
		vector<Point2f> m = corners[i];

		if(ids[i] == markId) {
			// get tvec, rvec
			solvePnP(markerCorners3d, m, camMatrix, distCoeffs, matRvec, matTvec);
			aruco::drawAxis(imgOri, camMatrix, distCoeffs, matRvec, matTvec, 1.0);

			cv::Rodrigues(matRvec, matR);
			cv::Mat T = cv::Mat::eye(4, 4, matR.type()); // T is 4x4 unit matrix.
			for(unsigned int row=0; row<3; ++row) {
				for(unsigned int col=0; col<3; ++col) {
					T.at<double>(row, col) = matR.at<double>(row, col);
				}
				T.at<double>(row, 3) = matTvec.at<double>(row, 0);
			}

			//Convert CV to GL
			cv::Mat cvToGl = cv::Mat::zeros(4, 4, CV_64F);
			cvToGl.at<double>(0, 0) =  1.0f;
			cvToGl.at<double>(1, 1) = -1.0f; // Invert the y axis
			cvToGl.at<double>(2, 2) = -1.0f; // invert the z axis
			cvToGl.at<double>(3, 3) =  1.0f;
			T = cvToGl * T;

			//Convert to cv::Mat to glm::mat4.
			for(int i=0; i < T.cols; i++) {
				for(int j=0; j < T.rows; j++) {
					camPose[j][i] = *T.ptr<double>(i, j);
				}
			}
		} 
		else {
			// 다른 마커 축 그리기 -> 추가 필요
			solvePnP(markerCorners3d, m, camMatrix, distCoeffs, matRvec_tmp, matTvec_tmp);
			aruco::drawAxis(imgOri, camMatrix, distCoeffs, matRvec_tmp, matTvec_tmp, 1.0);	
		}
	}

	// 좌표축 그린 이미지 
	img = imgOri;

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

KGlCamera::KGlCamera() {
}

KGlCamera::~KGlCamera() {
}

