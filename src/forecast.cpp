#include <iostream>
#include <fstream>
#include <string>
#include <curl/curl.h>          // HTTP 요청을 위한 라이브러리
#include <nlohmann/json.hpp>    // JSON 파싱을 위한 외부 라이브러리
using namespace std;

// 콜백 함수 정의 -> curl이 응답 데이터를 받을 때 호출되며 데이터를 string에 저장
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// 주어진 도시의 날씨 예보 JSON데이터를 가져옴
string get_weather_forecast(const string& api_key, const string& city) {
    CURL* curl;
    CURLcode res;                   // curl결과 코드 저장 변수
    string readBuffer;              // 서버 응답 저장할 버퍼

    curl = curl_easy_init();        // curl 초기화
    if (curl) {
        // 요청 URL 구성
        string url = "https://api.openweathermap.org/data/2.5/forecast?q=" + city + "&appid=" + api_key + "&units=metric";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)                    // 실패시 에러 출력
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << "\n";
        curl_easy_cleanup(curl);
    }
    return readBuffer;
}

void recommend_items_by_forecast(const nlohmann::json& forecast_json, ostream& out) {
    if (!forecast_json.contains("list")) {
        out << "날씨 예보 정보가 없습니다.\n";
        return;
    }

    time_t now = time(nullptr);         // 현재 시간 
    tm* today_tm = localtime(&now);
    char today_date[11];  // "YYYY-MM-DD"
    strftime(today_date, sizeof(today_date), "%Y-%m-%d", today_tm);     // 날짜 문자열 생성

    double temp_sum = 0;
    int count = 0;
    double max_temp = -1000;
    double min_temp = 1000;
    bool rain_expected = false;
    bool snow_expected = false;

    for (const auto& entry : forecast_json["list"]) {
        string dt_txt = entry["dt_txt"];
        if (dt_txt.rfind(today_date, 0) == 0) {  // 오늘 날짜와 일치하는 항목만 처리
            if (entry.contains("main") && entry["main"].contains("temp")) {
                double temp = entry["main"]["temp"].get<double>();          // 기온 가져오기
                temp_sum += temp;
                count++;

                if (temp > max_temp) max_temp = temp;                      // 최고 기온     
                if (temp < min_temp) min_temp = temp;                      // 최저 기온
            }

            if (entry.contains("weather") && entry["weather"].is_array()) {
                std::string weather_main = entry["weather"][0]["main"];    // 날씨 요약
                if (weather_main == "Rain" || weather_main == "Drizzle") rain_expected = true;
                if (weather_main == "Snow") snow_expected = true;
            }
        }
    }

    if (count == 0) {
        out << "오늘의 날씨 데이터가 충분하지 않습니다.\n";
        return;
    }

    double avg_temp = temp_sum / count;                 // 평균 기온 계산
    double temp_diff = max_temp - min_temp;             // 일교차 계산
    
    out << "📅 오늘의 날씨 예보\n";
    out << "평균 기온: " << avg_temp << "°C\n";
    out << "최고 기온: " << max_temp << "°C\n";
    out << "최저 기온: " << min_temp << "°C\n";

    // 기온에 따른 아이템 추천
    if (avg_temp <= 5) {
        out << "추천 아이템: 두꺼운 코트, 목도리, 장갑\n";
    } else if (avg_temp <= 15) {
        out << "추천 아이템: 가디건, 자켓\n";
    } else if (avg_temp <= 25) {
        out << "추천 아이템: 긴팔 티셔츠, 청바지\n";
    } else {
        out << "추천 아이템: 반팔 티셔츠, 반바지, 선글라스\n";
    }

    if (temp_diff >= 10) {
        out << "⚠️ 일교차가 큽니다! 외투를 챙기세요.\n";
    }

    if (rain_expected)
        out << "☔ 비가 예상됩니다. 우산을 챙기세요!\n";
    else if (!snow_expected)
        out << "☀️ 비나 눈 예보는 없습니다. 좋은 하루 되세요!\n";

    if (snow_expected)
        out << "❄ 눈이 예상돼요. 조심하세요!\n";
}

int main() {
    string api_key = "c06ef023b235507e0cd1b95e67c2a465";
    string city = "Seoul";

    string forecast_data = get_weather_forecast(api_key, city);    // 날씨 예보 가져오기
    if (forecast_data.empty()) {
        cerr << "날씨 예보 정보를 가져오지 못했습니다.\n";
        return 1;
    }

    try {
        auto forecast_json = nlohmann::json::parse(forecast_data);  // 문자열 JSON 파싱

        ofstream outfile("weather_result.txt");                // 결과 파일 열기
        if (!outfile) {
            cerr << "파일을 열 수 없습니다.\n";
            return 1;
        }

        recommend_items_by_forecast(forecast_json, cout);           // 콘솔에 출력
        recommend_items_by_forecast(forecast_json, outfile);        // 파일에 저장    

        outfile.close();
    } catch (const exception& e) {
        cerr << "JSON 파싱 오류: " << e.what() << "\n";          // 예외 발생 시 출력
        return 1;
    }

    return 0;
}

