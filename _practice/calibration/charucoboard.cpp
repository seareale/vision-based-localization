#include <opencv2/highgui.hpp>
#include <opencv2/aruco/charuco.hpp>
 
using namespace cv;
 
namespace {
    const char* about = "Create a ChArUco board image";
    const char* keys =
        "{@outfile |<none> | Output image }"
        "{w        |       | Number of squares in X direction }"
        "{h        |       | Number of squares in Y direction }"
        "{sl       |       | Square side length (in pixels) }"
        "{ml       |       | Marker side length (in pixels) }"
        "{d        |       | dictionary: DICT_4X4_50=0, DICT_4X4_100=1, DICT_4X4_250=2,"
        "DICT_4X4_1000=3, DICT_5X5_50=4, DICT_5X5_100=5, DICT_5X5_250=6, DICT_5X5_1000=7, "
        "DICT_6X6_50=8, DICT_6X6_100=9, DICT_6X6_250=10, DICT_6X6_1000=11, DICT_7X7_50=12,"
        "DICT_7X7_100=13, DICT_7X7_250=14, DICT_7X7_1000=15, DICT_ARUCO_ORIGINAL = 16}"
        "{m        |       | Margins size (in pixels). Default is (squareLength-markerLength) }"
        "{bb       | 1     | Number of bits in marker borders }"
        "{si       | false | show generated image }";
}
 
int main(int argc, char *argv[]) {
 
    int squaresX = 5;//가로방향 마커 갯수
    int squaresY = 7;//세로방향 마터 갯수
    int squareLength = 80; //검은색 테두리 포함한 정사각형의 한변 길이 , 픽셀단위
    int markerLength = 40;//마커 한 변의 길이, 픽셀단위
    int dictionaryId = 10; //DICT_6X6_250=10
    int margins = 10;//ChArUco board와 A4용지 사이의 흰색 여백 크기, 픽셀단위
    int borderBits = 1;//검은색 테두리 크기 
    bool showImage = false;
 
    String out = "board.jpg";
 
    
    Ptr<aruco::Dictionary> dictionary =
        aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME(dictionaryId));
 
    Size imageSize;
    imageSize.width = squaresX * squareLength + 2 * margins;
    imageSize.height = squaresY * squareLength + 2 * margins;
 
    Ptr<aruco::CharucoBoard> board = aruco::CharucoBoard::create(squaresX, squaresY, (float)squareLength,
        (float)markerLength, dictionary);
 
    // show created board
    Mat boardImage;
    board->draw(imageSize, boardImage, margins, borderBits);
 
    if (showImage) {
        imshow("board", boardImage);
        waitKey(0);
    }
 
    imwrite(out, boardImage);
 
    return 0;
}

