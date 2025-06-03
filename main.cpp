#pragma comment(lib, "comctl32.lib")
#pragma comment(linker, "/subsystem:windows")
#include <windows.h>
#include <commctrl.h>
#include <string>
#include <vector>
#include <fstream>
#include "WorkerManager.h"
#include <locale>
#include <codecvt>

#define IDC_LISTVIEW      1001
#define IDC_ADD_BUTTON    1002
#define IDC_DELETE_BUTTON 1003
#define IDC_MODIFY_BUTTON 1004
#define IDC_FIND_BUTTON   1005
#define IDC_SORT_BUTTON   1006
#define IDC_CLEAR_BUTTON  1007
#define IDC_ID_EDIT       1008
#define IDC_NAME_EDIT     1009
#define IDC_DEPT_COMBO    1010
#define IDC_FIND_EDIT     1011  // 查找输入框
#define ID_FIND_CONFIRM   1012  // 查找确认按钮

// 新增登录对话框相关定义
#define IDC_LOGIN_USERNAME 2001
#define IDC_LOGIN_PASSWORD 2002
#define IDC_LOGIN_BUTTON   2003

WorkerManager g_WorkerManager;

void RefreshListView(HWND hList) {
    SendMessage(hList, WM_SETREDRAW, FALSE, 0);
    ListView_DeleteAllItems(hList);

    auto workers = g_WorkerManager.GetAllWorkers();
    for (size_t i = 0; i < workers.size(); ++i) {
        LVITEM item = { 0 };
        item.mask = LVIF_TEXT | LVIF_PARAM;
        item.iItem = static_cast<int>(i);
        item.lParam = reinterpret_cast<LPARAM>(workers[i]);

        // 第一列：ID
        std::wstring idText = std::to_wstring(workers[i]->m_Id);
        wchar_t* idBuffer = new wchar_t[idText.size() + 1];
        wcscpy_s(idBuffer, idText.size() + 1, idText.c_str());
        item.pszText = idBuffer;

        int nItem = ListView_InsertItem(hList, &item);
        delete[] idBuffer;

        if (nItem == -1) continue; // 插入失败则跳过

        // 第二列：姓名
        std::wstring name(workers[i]->m_Name.begin(), workers[i]->m_Name.end());
        wchar_t* nameBuffer = new wchar_t[name.size() + 1];
        wcscpy_s(nameBuffer, name.size() + 1, name.c_str());
        ListView_SetItemText(hList, nItem, 1, nameBuffer);
        delete[] nameBuffer;

        // 第三列：部门
        std::wstring dept = workers[i]->getDeptName();


        wchar_t* deptBuffer = new wchar_t[dept.size() + 1];
        wcscpy_s(deptBuffer, dept.size() + 1, dept.c_str());
        ListView_SetItemText(hList, nItem, 2, deptBuffer);

        delete[] deptBuffer;


    }

    SendMessage(hList, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(hList, NULL, TRUE);
}

LRESULT CALLBACK AddDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ModifyDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

// 1. 首先声明查找对话框的窗口过程函数
LRESULT CALLBACK FindDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

// 登录对话框窗口过程
LRESULT CALLBACK LoginDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_INITDIALOG:
        SetFocus(GetDlgItem(hDlg, IDC_LOGIN_USERNAME));
        return TRUE;

    case WM_COMMAND: {
        if (LOWORD(wParam) == IDC_LOGIN_BUTTON) {
            wchar_t username[32] = { 0 };
            wchar_t password[32] = { 0 };

            GetDlgItemText(hDlg, IDC_LOGIN_USERNAME, username, 32);
            GetDlgItemText(hDlg, IDC_LOGIN_PASSWORD, password, 32);

            // 验证用户名和密码
            if (wcscmp(username, L"admin") == 0 && wcscmp(password, L"123456") == 0) {
                // 登录成功，显示主窗口并关闭登录框
                HWND hMainWnd = (HWND)GetProp(hDlg, L"MAIN_WINDOW");
                ShowWindow(hMainWnd, SW_SHOW);
                DestroyWindow(hDlg);
            }
            else {
                MessageBox(hDlg, L"用户名或密码错误", L"登录失败", MB_ICONERROR);
            }
        }
        break;
    }
    case WM_CLOSE:
        PostQuitMessage(0); // 直接退出程序
        break;
    default:
        return DefWindowProc(hDlg, msg, wParam, lParam);
    }
    return 0;
}

// 创建登录对话框的函数
void CreateLoginDialog(HWND hMainWnd)
{
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = LoginDialogProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"LoginDialog";
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClass(&wc);

    HWND hDlg = CreateWindow(
        L"LoginDialog", L"教职工管理系统登录",
        WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, 300, 200,
        hMainWnd, NULL, NULL, NULL);

    // 创建控件
    CreateWindow(L"STATIC", L"用户名:", WS_VISIBLE | WS_CHILD,
        20, 20, 60, 20, hDlg, NULL, NULL, NULL);
    CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER,
        90, 20, 180, 25, hDlg, (HMENU)IDC_LOGIN_USERNAME, NULL, NULL);

    CreateWindow(L"STATIC", L"密码:", WS_VISIBLE | WS_CHILD,
        20, 60, 60, 20, hDlg, NULL, NULL, NULL);
    CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_PASSWORD,
        90, 60, 180, 25, hDlg, (HMENU)IDC_LOGIN_PASSWORD, NULL, NULL);

    CreateWindow(L"BUTTON", L"登录", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        100, 110, 100, 30, hDlg, (HMENU)IDC_LOGIN_BUTTON, NULL, NULL);

    // 居中显示
    RECT rc;
    GetWindowRect(hDlg, &rc);
    SetWindowPos(hDlg, NULL,
        (GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2,
        (GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 2,
        0, 0, SWP_NOSIZE);

    // 存储主窗口句柄
    SetProp(hDlg, L"MAIN_WINDOW", hMainWnd);

    ShowWindow(hDlg, SW_SHOW);
    UpdateWindow(hDlg);
}

// 2. 查找对话框显示函数
void ShowFindDialog(HWND hParent) {
    static bool classRegistered = false;
    if (!classRegistered) {
        WNDCLASS wc = { 0 };
        wc.lpfnWndProc = FindDialogProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = L"FindDialogClass";
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        RegisterClass(&wc);
        classRegistered = true;
    }

    // 计算居中位置
    RECT rcParent;
    GetWindowRect(hParent, &rcParent);
    int x = rcParent.left + (rcParent.right - rcParent.left - 350) / 2;
    int y = rcParent.top + (rcParent.bottom - rcParent.top - 200) / 2;

    // 创建查找对话框
    HWND hDlg = CreateWindow(L"FindDialogClass", L"查找职工",
        WS_POPUP | WS_CAPTION | WS_SYSMENU,
        x, y, 350, 200,
        hParent, NULL, NULL, NULL);
