openCV 4.1.1  
  
- aruco 마커 생성코드 컴파일
<pre>
$ g++ -o create_marker create_marker.cpp $(pkg-config opencv4 --cflags --libs)
</pre>
<br>

- 마커 생성 옵션
<pre>
$ ./create_marker -d=10 --id=0 --ms=200 --bb=1 marker1.png
</pre>
<br>

- d :
  - 미리 정의된 dictionary의 인덱스. 10은 DICT_6X6_250.
  - 크기가 6X6인 마커가 250개 포함된 dictionary 사용.
  - 6X6은 2차원 비트패턴의 크기를 의미. 검은색 테두리 영역 제외.
<br>

- id :  
  - dicationary에 속한 마커들 중 사용할 마커 id의 인덱스 값.
  - DICT_6x6의 경우, 0부터 249까지 가능.  
<br>  

- ms :  
  - 마커 이미지의 크기를 픽셀단위로 지정.
  - 200의 경우 200X200 픽셀 크기의 이미지로 저장.  
<br> 

- bb :  
  - 검은색 테두리 영역의 크기.  
  - 내부 2차원 비트패턴에서 하나의 셀 크기의 배수로 지정.
