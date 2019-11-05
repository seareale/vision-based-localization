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
		return true;
	}
	return false;
}

bool KCvCamera::getCamImage(cv::Mat &img) {
	if(isOpened()){
		mCapture >> img;

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
		if(ids[i] == markId) {
			vector<Point2f> m = corners[i];
	
			// get tvec, rvec
			solvePnP(markerCorners3d, m, camMatrix, distCoeffs, matRvec, matTvec);
			aruco::drawAxis(imgOri, camMatrix, distCoeffs, matRvec, matTvec, 1.0);

			img = imgOri;

			break;
		} 	
	}

	if(i == int(corners.size())) return false;

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

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

KGlCamera::KGlCamera() {
}

KGlCamera::~KGlCamera() {
}

