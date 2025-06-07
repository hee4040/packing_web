# packing_web
### ✏️ 사용법
1. 짐 사진을 찍는다
2. 업로드 버튼을 눌러 사진을 업로드 한다
3. 결과 확인 버튼을 눌러 챙겨야 할 것, 추가적으로 챙긴 것 확인
4. 날씨 기반 추천을 통해 더 챙길 물건을 확인
<br>

### 🤔 구현방법
1. yolo모델을 통해 물건을 인식하는 cpp코드(main.cpp) 작성
2. yolo모델로 인식한 결과를 써놓은 리스트와 비교해서 결과를 알려주는 cpp코드(compare.cpp) 작성
3. opeanwheather에서 api를 이용해 날씨를 받고 날씨에 맞는 물건을 추천해주는 cpp코드(forecast.cpp)작성
4. flask를 통해 cpp파일을 불러오고 화면에 구현
<br>

### 📱 UI
<br>
<img src="https://github.com/user-attachments/assets/e110187e-e22e-4cd7-9501-086a49655368" width="300">
<img src="https://github.com/user-attachments/assets/a1439a63-482c-4644-8d19-e729f23c1388" width="300">
<br>

<img src="https://github.com/user-attachments/assets/1b9e1465-774d-4ffa-b2ef-de2a060c8e4a" width="300">

<img src="https://github.com/user-attachments/assets/36b5133a-8a21-4c31-937d-f898c142a14b" width="300">



