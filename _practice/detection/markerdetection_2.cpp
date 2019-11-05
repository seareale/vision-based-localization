#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/aruco.hpp> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;
using namespace cv;

int dictionaryId = 10;//DICT_6X6_250=10
Ptr<aruco::Dictionary> dictionary = aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME(dictionaryId));
Ptr<aruco::DetectorParameters> detectorParams = aruco::DetectorParameters::create();

int main(int argc, char** argv)
{
	// 정수 파라미터 확인
	if( argc >= 2 ) {
		for(int i = 1; i < argc; i++) {
			size_t size = strlen(argv[i]);
			for (int j = 0; j < (int) size; j++) {
				if (argv[i][j] < '0' || argv[i][j] > '9'){
					cout << "파라미터 문제 있음" << endl;
					exit(0);
				}
			}
		}
	} 

	int CAM_ID = 0;
	VideoCapture cap;
	cap.open(CAM_ID);
 
	while(cap.grab()){
		Mat input_image, copy_image;
		cap.retrieve(input_image);
		input_image.copyTo(copy_image);		

		vector< int > ids;
		vector< vector< Point2f > > corners, rejected;

		// 마커 탐지, 정상적인 마커만 corners에 저장
		aruco::detectMarkers(input_image, dictionary, corners, ids, detectorParams, rejected); 

		// 카메라 컬리브레이션으로 얻은 카메라 정보를 파일에서 읽어옴
		Mat camMatrix, distCoeffs;
		FileStorage fs("output.txt", FileStorage::READ);
		if (!fs.isOpened()) return false;
		fs["camera_matrix"] >> camMatrix;
		fs["distortion_coefficients"] >> distCoeffs;

		vector<cv::Point3f> markerCorners3d;
		markerCorners3d.push_back(cv::Point3f(-0.5f, 0.5f, 0));
		markerCorners3d.push_back(cv::Point3f(0.5f, 0.5f, 0));
		markerCorners3d.push_back(cv::Point3f(0.5f, -0.5f, 0));
		markerCorners3d.push_back(cv::Point3f(-0.5f, -0.5f, 0));


		for (int i = 0; i < corners.size(); i++) {
			// 파라미터에 입력된 ID만 출력
			if( argc >= 2 ) {
				bool printf = false;
				for(int j = 1; j < argc; j++) {
					if(ids[i] == atoi(argv[j])) {
						printf = true;
						break;	
					}
				}
				if(!printf) break; 
			}
			
			vector<Point2f> m = corners[i];

			Mat rotation_vector, translation_vector;
			solvePnP(markerCorners3d, m, camMatrix, distCoeffs, rotation_vector, translation_vector);    

			// rvec, tvec 값
			//cout << "rotation_vector" << endl << rotation_vector << endl;
			//cout << "translation_vector" << endl << translation_vector << endl;

			// 3D 좌표축 그리기
			aruco::drawAxis(copy_image, camMatrix, distCoeffs, rotation_vector, translation_vector, 1.0);

		}
		imshow("OPENCV_CAM", copy_image);
		
		// 'ESC' 입력 시, 종료
		if(waitKey(10) == 27) break; 
	}
	return 0;
}

