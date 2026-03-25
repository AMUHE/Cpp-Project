#include <windows.h>
#include <vector>
#include <algorithm>
#include <cstdio>
using namespace std;

struct Student {
    int id;
    char name[20];
    int age;
    float score;
};

vector<Student> stuList;
int sortMode = 0; // 0=默认 1=按学号 2=按成绩

HWND hEditID, hEditName, hEditAge, hEditScore;
HWND hBtnAdd, hBtnDel, hBtnMod, hBtnSearch, hBtnShow, hBtnSort;
HWND hListBox;

int GetInt(HWND hWnd);
float GetFloat(HWND hWnd);
void AddStudent();
void DeleteStudent();
void ModifyStudent();
void SearchStudent();
void ShowAllStudents();
void ClearInput();
bool CompareID(Student a, Student b);
bool CompareScore(Student a, Student b);

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HBRUSH hBgBrush;
    static HFONT hFont;

    switch (msg) {
        case WM_CREATE: {
            hFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                               DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                               CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                               DEFAULT_PITCH | FF_DONTCARE, "微软雅黑");

            hBgBrush = CreateSolidBrush(RGB(240, 230, 250));

            CreateWindowA("STATIC", "学号", WS_VISIBLE | WS_CHILD, 30, 25, 50, 25, hWnd, NULL, NULL, NULL);
            CreateWindowA("STATIC", "姓名", WS_VISIBLE | WS_CHILD, 30, 60, 50, 25, hWnd, NULL, NULL, NULL);
            CreateWindowA("STATIC", "年龄", WS_VISIBLE | WS_CHILD, 30, 95, 50, 25, hWnd, NULL, NULL, NULL);
            CreateWindowA("STATIC", "成绩", WS_VISIBLE | WS_CHILD, 30, 130, 50, 25, hWnd, NULL, NULL, NULL);

            hEditID = CreateWindowA("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 85, 25, 140, 26, hWnd, NULL, NULL, NULL);
            hEditName = CreateWindowA("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 85, 60, 140, 26, hWnd, NULL, NULL, NULL);
            hEditAge = CreateWindowA("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 85, 95, 140, 26, hWnd, NULL, NULL, NULL);
            hEditScore = CreateWindowA("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 85, 130, 140, 26, hWnd, NULL, NULL, NULL);

            hBtnAdd = CreateWindowA("BUTTON", "添加", WS_VISIBLE | WS_CHILD, 240, 25, 90, 28, hWnd, NULL, NULL, NULL);
            hBtnDel = CreateWindowA("BUTTON", "删除", WS_VISIBLE | WS_CHILD, 240, 60, 90, 28, hWnd, NULL, NULL, NULL);
            hBtnMod = CreateWindowA("BUTTON", "修改", WS_VISIBLE | WS_CHILD, 240, 95, 90, 28, hWnd, NULL, NULL, NULL);
            hBtnSearch = CreateWindowA("BUTTON", "查询", WS_VISIBLE | WS_CHILD, 240, 130, 90, 28, hWnd, NULL, NULL, NULL);
            hBtnShow = CreateWindowA("BUTTON", "显示全部", WS_VISIBLE | WS_CHILD, 340, 25, 100, 28, hWnd, NULL, NULL, NULL);
            hBtnSort = CreateWindowA("BUTTON", "排序切换", WS_VISIBLE | WS_CHILD, 340, 60, 100, 28, hWnd, NULL, NULL, NULL);

            hListBox = CreateWindowA("LISTBOX", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 30, 170, 410, 210, hWnd, NULL, NULL, NULL);

            SendMessage(hEditID, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hEditName, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hEditAge, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hEditScore, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnAdd, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnDel, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnMod, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnSearch, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnShow, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnSort, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hListBox, WM_SETFONT, (WPARAM)hFont, TRUE);
            break;
        }

        case WM_CTLCOLORDLG:
        case WM_CTLCOLORSTATIC:
            SetBkColor((HDC)wParam, RGB(240, 230, 250));
            SetTextColor((HDC)wParam, RGB(100, 50, 150));
            return (LRESULT)hBgBrush;

        case WM_CTLCOLORBTN:
            return (LRESULT)CreateSolidBrush(RGB(180, 140, 220));

        case WM_COMMAND:
            if ((HWND)lParam == hBtnAdd) AddStudent();
            if ((HWND)lParam == hBtnDel) DeleteStudent();
            if ((HWND)lParam == hBtnMod) ModifyStudent();
            if ((HWND)lParam == hBtnSearch) SearchStudent();
            if ((HWND)lParam == hBtnShow) ShowAllStudents();
            if ((HWND)lParam == hBtnSort) {
                if (sortMode == 0 || sortMode == 2) {
                    sort(stuList.begin(), stuList.end(), CompareID);
                    MessageBoxA(NULL, "已按学号从小到大排序", "提示", MB_OK);
                    sortMode = 1;
                } else {
                    sort(stuList.begin(), stuList.end(), CompareScore);
                    MessageBoxA(NULL, "已按成绩从高到低排序", "提示", MB_OK);
                    sortMode = 2;
                }
                ShowAllStudents();
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProcA(hWnd, msg, wParam, lParam);
    }
    return 0;
}

// 按学号升序
bool CompareID(Student a, Student b) {
    return a.id < b.id;
}

// 按成绩降序
bool CompareScore(Student a, Student b) {
    return a.score > b.score;
}

int GetInt(HWND hWnd) {
    char buf[50];
    GetWindowTextA(hWnd, buf, 50);
    return atoi(buf);
}

float GetFloat(HWND hWnd) {
    char buf[50];
    GetWindowTextA(hWnd, buf, 50);
    return (float)atof(buf);
}

void AddStudent() {
    Student s;
    s.id = GetInt(hEditID);
    GetWindowTextA(hEditName, s.name, 20);
    s.age = GetInt(hEditAge);
    s.score = GetFloat(hEditScore);
    stuList.push_back(s);
    MessageBoxA(NULL, "添加成功！", "提示", MB_OK);
    ShowAllStudents();
    ClearInput();
}

void DeleteStudent() {
    int id = GetInt(hEditID);
    for (vector<Student>::iterator it = stuList.begin(); it != stuList.end(); it++) {
        if (it->id == id) {
            stuList.erase(it);
            MessageBoxA(NULL, "删除成功！", "提示", MB_OK);
            ShowAllStudents();
            ClearInput();
            return;
        }
    }
    MessageBoxA(NULL, "未找到该学生！", "错误", MB_OK | MB_ICONERROR);
}

void ModifyStudent() {
    int id = GetInt(hEditID);
    for (vector<Student>::iterator it = stuList.begin(); it != stuList.end(); it++) {
        if (it->id == id) {
            GetWindowTextA(hEditName, it->name, 20);
            it->age = GetInt(hEditAge);
            it->score = GetFloat(hEditScore);
            MessageBoxA(NULL, "修改成功！", "提示", MB_OK);
            ShowAllStudents();
            ClearInput();
            return;
        }
    }
    MessageBoxA(NULL, "未找到该学生！", "错误", MB_OK | MB_ICONERROR);
}

void SearchStudent() {
    int id = GetInt(hEditID);
    SendMessageA(hListBox, LB_RESETCONTENT, 0, 0);
    for (vector<Student>::iterator it = stuList.begin(); it != stuList.end(); it++) {
        if (it->id == id) {
            char str[100];
            sprintf(str, "学号：%d   姓名：%s   年龄：%d   成绩：%.1f", it->id, it->name, it->age, it->score);
            SendMessageA(hListBox, LB_ADDSTRING, 0, (LPARAM)str);
            return;
        }
    }
    MessageBoxA(NULL, "未找到该学生！", "错误", MB_OK | MB_ICONERROR);
}

void ShowAllStudents() {
    SendMessageA(hListBox, LB_RESETCONTENT, 0, 0);
    char str[100];
    for (vector<Student>::iterator it = stuList.begin(); it != stuList.end(); it++) {
        sprintf(str, "学号：%d   姓名：%s   年龄：%d   成绩：%.1f", it->id, it->name, it->age, it->score);
        SendMessageA(hListBox, LB_ADDSTRING, 0, (LPARAM)str);
    }
}

void ClearInput() {
    SetWindowTextA(hEditID, "");
    SetWindowTextA(hEditName, "");
    SetWindowTextA(hEditAge, "");
    SetWindowTextA(hEditScore, "");
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEXA wc = {0};
    wc.cbSize        = sizeof(WNDCLASSEXA);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = "StudentUI";
    wc.hbrBackground = CreateSolidBrush(RGB(240, 230, 250));

    RegisterClassExA(&wc);

    HWND hWnd = CreateWindowExA(0, "StudentUI", "学生管理系统",
                                WS_OVERLAPPEDWINDOW, 100, 100, 480, 430, NULL, NULL, hInstance, NULL);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}