#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>
using namespace std;

int main() {
    // 리스트 읽기
    vector<string> known_items;
    ifstream list_file("list.txt");
    string item;
    while (getline(list_file, item)) {
        if (!item.empty()) known_items.push_back(item);        // 빈 줄 제외하고 추가
    }
    list_file.close();

    // result.txt 읽기
    unordered_set<string> detected_items;           // 감지된 항목 저장
    ifstream infile("result.txt");
    string line;
    while (getline(infile, line)) {
        if (!line.empty()) detected_items.insert(line);
    }
    infile.close();

    // 파일 출력
    ofstream out("compare_result.txt");
    out << "[matched]\n";
    for (const auto& item : known_items) {
        if (detected_items.count(item)) out << item << "\n";
    }

    out << "[extra]\n";
    for (const auto& item : detected_items) {
        if (find(known_items.begin(), known_items.end(), item) == known_items.end()) {
            out << item << "\n";
        }
    }

    out << "[missing]\n";
    for (const auto& item : known_items) {
        if (detected_items.count(item) == 0) out << item << "\n";
    }

    out.close();        // 파일 저장 후 종료
    return 0;
}
