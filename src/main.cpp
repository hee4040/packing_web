#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>   // opencv의 기본 기능과 딥러닝 모듈
#include <iostream>       // 입출력
#include <fstream>     // 파일 처리
using namespace std;

// 객체 인식에서 사용할 신뢰도 임계값, NMS 임계값, 입력 이미지 크기 정의
const float CONFIDENCE_THRESHOLD = 0.5;
const float NMS_THRESHOLD = 0.4;
const int INPUT_WIDTH = 640;
const int INPUT_HEIGHT = 480;

// COCO 클래스 이름 불러오고 vector에 저장
vector<string> loadClassList(const string& path) {
    vector<std::string> classes;
    ifstream file(path);
    string line;
    while (getline(file, line))
        classes.push_back(line);
    return classes;
}

int main() {
    string modelPath = "yolov8m.onnx";    // 사용할 모델
    string imagePath = "test.jpg";        // 인식할 이미지
    string classFile = "coco.names";      // 클래스 이름이 저장된 파일

    // 클래스 이름 불러오기
    vector<string> classList = loadClassList(classFile);

    // ONNX 형식의 YOLO 모델을 OpenCV DNN 모듈로 로드
    cv::dnn::Net net = cv::dnn::readNetFromONNX(modelPath);
    net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

    // 이미지 불러오기
    cv::Mat image = cv::imread(imagePath);
    if (image.empty()) {
        cerr << "이미지를 열 수 없습니다.\n";           // 에러 메시지 즉시 출력
        return -1;
    }

    // 전처리 -> DNN입력형식으로 변환
    cv::Mat blob;
    cv::dnn::blobFromImage(image, blob, 1.0 / 255.0, cv::Size(INPUT_WIDTH, INPUT_HEIGHT), cv::Scalar(), true, false);
    net.setInput(blob);         // 전처리된 이미지를 DNN모델에 입력으로 설정

    // 추론
    vector<cv::Mat> outputs;
    net.forward(outputs, net.getUnconnectedOutLayersNames());

    // 결과 파싱
    vector<int> classIds;           // 클래스ID
    vector<float> confidences;      // 신뢰도
    vector<cv::Rect> boxes;         // 박스

    // YOLO의 출력 텐서 형태를 파악하고 해당 데이터를 직접 포인터로 접근
    const int rows = outputs[0].size[1];
    const int dimensions = outputs[0].size[2];
    float* data = (float*)outputs[0].data;

    for (int i = 0; i < rows; ++i) {
        float confidence = data[i * dimensions + 4];
        if (confidence >= CONFIDENCE_THRESHOLD) {
            float* classesScores = data + i * dimensions + 5;
            cv::Mat scores(1, classList.size(), CV_32FC1, classesScores);
            cv::Point classIdPoint;
            double maxClassScore;
            minMaxLoc(scores, 0, &maxClassScore, 0, &classIdPoint);

            if (maxClassScore > CONFIDENCE_THRESHOLD) {
                int centerX = (int)(data[i * dimensions] * image.cols);
                int centerY = (int)(data[i * dimensions + 1] * image.rows);
                int width = (int)(data[i * dimensions + 2] * image.cols);
                int height = (int)(data[i * dimensions + 3] * image.rows);
                int left = centerX - width / 2;
                int top = centerY - height / 2;

                classIds.push_back(classIdPoint.x);
                confidences.push_back((float)maxClassScore);
                boxes.push_back(cv::Rect(left, top, width, height));
            }
        }
    }

    // NMS(비최대 억제) -> 중복된 박스 제거
    vector<int> indices;
    cv::dnn::NMSBoxes(boxes, confidences, CONFIDENCE_THRESHOLD, NMS_THRESHOLD, indices);

    ofstream outFile("result.txt");
    for (int idx : indices) {               // indices에 들어있는 값을 하나씩 idx에 넣어 사용
        cv::Rect box = boxes[idx];
        string label = classList[classIds[idx]];
        outFile << label << "\n";
        cv::rectangle(image, box, cv::Scalar(0, 255, 0), 2);
        cv::putText(image, label, box.tl(), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 2);
    }

    outFile.close();
    cv::imwrite("result_with_boxes.jpg", image);

    cout << "인식 완료: result.txt에 저장됨.\n";
    return 0;
}
