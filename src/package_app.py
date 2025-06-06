import subprocess
from flask import Flask, render_template, request, redirect, url_for

app = Flask(__name__)

# list.txt 파일을 읽어 리스트로 반환
def load_list():
    with open("list.txt", "r") as f:
        return [line.strip() for line in f.readlines()]

# 새로운 리스트를 list.txt 파일에 저장  
def save_list(new_items):
    with open("list.txt", "w") as f:
        for item in new_items:
            f.write(item.strip() + "\n")

# compare_result.txt 파일을 읽어 matched, extra, missing 항목 구분
def parse_compare_result():
    result = {"matched": [], "extra": [], "missing": []}
    current_key = None

    with open("compare_result.txt", "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if line.startswith("[") and line.endswith("]"):
                current_key = line[1:-1]
            elif current_key:
                result[current_key].append(line)
    
    return result

# 메인 업로드 페이지: 리스트 보여주고 이미지 업로드 처리
@app.route("/", methods=["GET", "POST"])
def upload():
    item_list = load_list()
    
    if request.method == "POST":
        # 수정된 리스트 저장
        updated_items = request.form.getlist("items")
        updated_items = [item.strip() for item in updated_items if item.strip()]
        save_list(updated_items)

        # 이미지 저장
        file = request.files['photo']
        file.save("test.jpg")  # 저장

        # C++ 실행 파일 실행: 객체 인식 및 비교
        subprocess.run(["./main"])  # 객체 인식
        subprocess.run(["./compare"])  # 결과 비교

        # 비교 결과 파싱
        compare_result = parse_compare_result()

        # 결과 페이지 렌더링
        return render_template("result.html", compare_result=compare_result)

    # GET 요청 시 리스트 보여주기
    return render_template("upload.html", item_list=item_list)

# 리스트 수정 페이지 (항목 추가/삭제)
@app.route("/edit", methods=["GET", "POST"])
def edit():
    current_items = load_list()

    if request.method == "POST":
        action = request.form['action']

        if action == "리스트에 추가":
            # 여러 줄 텍스트를 줄 단위로 분할해 추가
            new_raw = request.form['items']
            new_items = [line.strip() for line in new_raw.splitlines() if line.strip()]
            updated = list(dict.fromkeys(current_items + new_items))  # 중복 제거
            save_list(updated)

        elif action == "선택 항목 삭제":
            to_delete = request.form.getlist("delete_items")
            updated = [item for item in current_items if item not in to_delete]
            save_list(updated)

        return redirect(url_for("upload"))          # 메인으로 이동

    # GET 요청 시 현재 리스트 보여주기
    return render_template("edit.html", current_items=current_items)

# 날씨 페이지 (외부 C++ 실행 후 파일 내용 보여줌)
@app.route("/weather")
def weather():
    try:
        subprocess.run(["./forecast"])  # forecast.cpp 실행

        # 결과 파일 읽기
        with open("weather_result.txt", "r", encoding="utf-8") as f:
            weather_text = f.read()
    except Exception as e:
        weather_result = f"날씨 정보를 불러오는 데 실패했습니다: {e}"

    return render_template("weather.html", weather_text=weather_text)

# 서버 실행
if __name__ == "__main__":
    app.run(debug=True)
