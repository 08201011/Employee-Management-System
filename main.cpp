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


    // 创建控件
// 1. ID输入框
    CreateWindow(L"STATIC", L"输入职工ID:", WS_VISIBLE | WS_CHILD,
        20, 20, 80, 20, hDlg, NULL, NULL, NULL);
    HWND hEditId = CreateWindow(L"EDIT", L"",
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER | WS_TABSTOP,
        120, 20, 200, 25, hDlg, (HMENU)1101, NULL, NULL);

    // 2. 查找按钮
    HWND hBtnFind = CreateWindow(L"BUTTON", L"查找",
        WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | WS_TABSTOP,
        120, 70, 100, 30, hDlg, (HMENU)IDOK, NULL, NULL);

    // 3. 取消按钮
    HWND hBtnCancel = CreateWindow(L"BUTTON", L"取消",
        WS_VISIBLE | WS_CHILD | WS_TABSTOP,
        120, 120, 100, 30, hDlg, (HMENU)IDCANCEL, NULL, NULL);

    // 显示对话框
    ShowWindow(hDlg, SW_SHOW);
    SetFocus(hEditId);

    // 消息循环
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) && IsWindow(hDlg)) {
        if (!IsDialogMessage(hDlg, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

// 3. 查找对话框过程函数
LRESULT CALLBACK FindDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case IDOK: {  // 查找按钮
            // 获取输入的ID
            wchar_t idText[32] = { 0 };
            GetDlgItemText(hDlg, 1101, idText, 32);

            if (wcslen(idText) == 0) {
                MessageBox(hDlg, L"请输入职工ID", L"提示", MB_ICONWARNING);
                break;
            }

            int id = _wtoi(idText);

            // 执行查询
            bool found = false;
            std::wstring info;
            auto workers = g_WorkerManager.GetAllWorkers();

            for (auto worker : workers) {
                if (worker->m_Id == id) {
                    info = L"职工ID: " + std::to_wstring(worker->m_Id) + L"\n"
                        L"姓名: " + std::wstring(worker->m_Name.begin(), worker->m_Name.end()) + L"\n"
                        L"部门: " + worker->getDeptName();
                    found = true;
                    break;
                }
            }

            // 显示结果
            MessageBox(hDlg,
                found ? info.c_str() : L"未找到指定ID的职工",
                L"查询结果",
                MB_OK);

            // 不关闭对话框，可以继续查询
            SetDlgItemText(hDlg, 1101, L"");
            SetFocus(GetDlgItem(hDlg, 1101));
            break;
        }
        case IDCANCEL:  // 取消按钮
            DestroyWindow(hDlg);
            break;
        }
        break;
    }
    case WM_CLOSE:
        DestroyWindow(hDlg);
        break;
    case WM_DESTROY:
        RemoveProp(hDlg, L"PARENT_HWND");
        //PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hDlg, msg, wParam, lParam);
    }
    return 0;
}


void ShowAddDialog(HWND hParent) {
    static bool classRegistered = false;
    if (!classRegistered) {
        WNDCLASS wc = { 0 };
        wc.lpfnWndProc = AddDialogProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = L"AddDialogClass";
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        RegisterClass(&wc);
        classRegistered = true;
    }

    HWND hDlg = CreateWindow(L"AddDialogClass", L"添加职工",
        WS_POPUP | WS_CAPTION | WS_SYSMENU, 350, 250, 350, 250,
        hParent, NULL, NULL, NULL);


    // 控件创建代码...
      // ... [保持原有添加对话框控件创建代码不变]
       // 3. 创建所有控件（与之前相同）
      // 3. 创建所有控件（带详细文字说明）
      // 工号输入
    CreateWindow(L"STATIC", L"职工工号:", WS_VISIBLE | WS_CHILD,
        20, 20, 80, 20, hDlg, NULL, NULL, NULL);
    HWND hEditId = CreateWindow(L"EDIT", L"",
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER | WS_TABSTOP,
        120, 20, 200, 25, hDlg, (HMENU)1008, NULL, NULL);

    // 姓名输入
    CreateWindow(L"STATIC", L"职工姓名:", WS_VISIBLE | WS_CHILD,
        20, 60, 80, 20, hDlg, NULL, NULL, NULL);
    HWND hEditName = CreateWindow(L"EDIT", L"",
        WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP,
        120, 60, 200, 25, hDlg, (HMENU)1009, NULL, NULL);

    // 职位选择
    CreateWindow(L"STATIC", L"职位类型:", WS_VISIBLE | WS_CHILD,
        20, 100, 80, 20, hDlg, NULL, NULL, NULL);
    HWND hComboDept = CreateWindow(L"COMBOBOX", L"",
        WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | WS_TABSTOP,
        120, 100, 200, 150, hDlg, (HMENU)1010, NULL, NULL);

    // 初始化下拉框
    SendMessage(hComboDept, CB_ADDSTRING, 0, (LPARAM)L"普通员工");
    SendMessage(hComboDept, CB_ADDSTRING, 0, (LPARAM)L"部门经理");
    SendMessage(hComboDept, CB_ADDSTRING, 0, (LPARAM)L"公司老板");
    SendMessage(hComboDept, CB_SETCURSEL, 0, 0);

    // 操作按钮
    HWND hBtnOk = CreateWindow(L"BUTTON", L"确定",
        WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | WS_TABSTOP,
        80, 160, 80, 30, hDlg, (HMENU)IDOK, NULL, NULL);
    HWND hBtnCancel = CreateWindow(L"BUTTON", L"取消",
        WS_VISIBLE | WS_CHILD | WS_TABSTOP,
        200, 160, 80, 30, hDlg, (HMENU)IDCANCEL, NULL, NULL);


    SetProp(hDlg, L"PARENT_HWND", hParent);
    ShowWindow(hDlg, SW_SHOW);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) && IsWindow(hDlg)) {
        if (!IsDialogMessage(hDlg, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

// 修改后的对话框显示函数
void ShowModifyDialog(HWND hParent, Worker* pWorker) {
    static bool classRegistered = false;
    if (!classRegistered) {
        WNDCLASSW wc = { 0 };
        wc.lpfnWndProc = ModifyDialogProc;
        wc.hInstance = GetModuleHandleW(NULL);
        wc.lpszClassName = L"ModifyDialogClass";
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
        RegisterClassW(&wc);
        classRegistered = true;
    }

    // 计算居中位置
    RECT rcParent;
    GetWindowRect(hParent, &rcParent);
    int x = rcParent.left + (rcParent.right - rcParent.left - 350) / 2;
    int y = rcParent.top + (rcParent.bottom - rcParent.top - 250) / 2;

    // 修改CreateWindowW调用，添加pWorker参数
    HWND hDlg = CreateWindowW(
        L"ModifyDialogClass",
        L"修改职工信息",
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        x, y, 350, 250,
        hParent,
        NULL,
        GetModuleHandleW(NULL),
        (LPVOID)pWorker  // 关键修改：传递指针参数
    );

    // 设置窗口数据
    SetWindowLongPtrW(hDlg, GWLP_USERDATA, (LONG_PTR)pWorker);

     // 消息循环（需要特殊处理）
    if (hDlg) {
        ShowWindow(hDlg, SW_SHOW);
        UpdateWindow(hDlg);

        MSG msg;
        while (GetMessageW(&msg, NULL, 0, 0)) {
            if (!IsDialogMessageW(hDlg, &msg)) {
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
            if (!IsWindow(hDlg)) break;
        }
    }
}


LRESULT CALLBACK AddDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case IDOK: {
            // 获取控件句柄
            HWND hEditId = GetDlgItem(hDlg, 1008);
            HWND hEditName = GetDlgItem(hDlg, 1009);
            HWND hComboDept = GetDlgItem(hDlg, 1010);

            // 获取输入数据
            wchar_t idText[32] = { 0 }, nameText[128] = { 0 };
            GetWindowText(hEditId, idText, 32);
            GetWindowText(hEditName, nameText, 128);
            int deptId = SendMessage(hComboDept, CB_GETCURSEL, 0, 0) + 1;

            // 输入验证
            if (wcslen(idText) == 0 || wcslen(nameText) == 0) {
                MessageBox(hDlg, L"请填写完整信息！", L"提示", MB_ICONWARNING);
                break;
            }

            // 添加到管理系统
            std::wstring wname(nameText);
            g_WorkerManager.AddEmp(_wtoi(idText), std::string(wname.begin(), wname.end()), deptId);

            // 刷新主窗口列表
            HWND hParent = (HWND)GetProp(hDlg, L"PARENT_HWND");
            if (hParent) {
                RefreshListView(GetDlgItem(hParent, IDC_LISTVIEW));
            }

            DestroyWindow(hDlg);
            return TRUE;
        }
        case IDCANCEL: {
            DestroyWindow(hDlg);
            return TRUE;
        }
        }
        break;
    }
    case WM_CLOSE: {
        DestroyWindow(hDlg);
        return TRUE;
    }
    case WM_DESTROY: {
        RemoveProp(hDlg, L"PARENT_HWND");
        return TRUE;
    }
    }
    return DefWindowProc(hDlg, msg, wParam, lParam);

}
