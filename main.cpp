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
#define IDC_FIND_EDIT     1011  // ���������
#define ID_FIND_CONFIRM   1012  // ����ȷ�ϰ�ť

// ������¼�Ի�����ض���
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

        // ��һ�У�ID
        std::wstring idText = std::to_wstring(workers[i]->m_Id);
        wchar_t* idBuffer = new wchar_t[idText.size() + 1];
        wcscpy_s(idBuffer, idText.size() + 1, idText.c_str());
        item.pszText = idBuffer;

        int nItem = ListView_InsertItem(hList, &item);
        delete[] idBuffer;

        if (nItem == -1) continue; // ����ʧ��������

        // �ڶ��У�����
        std::wstring name(workers[i]->m_Name.begin(), workers[i]->m_Name.end());
        wchar_t* nameBuffer = new wchar_t[name.size() + 1];
        wcscpy_s(nameBuffer, name.size() + 1, name.c_str());
        ListView_SetItemText(hList, nItem, 1, nameBuffer);
        delete[] nameBuffer;

        // �����У�����
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

// 1. �����������ҶԻ���Ĵ��ڹ��̺���
LRESULT CALLBACK FindDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

// ��¼�Ի��򴰿ڹ���
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

            // ��֤�û���������
            if (wcscmp(username, L"admin") == 0 && wcscmp(password, L"123456") == 0) {
                // ��¼�ɹ�����ʾ�����ڲ��رյ�¼��
                HWND hMainWnd = (HWND)GetProp(hDlg, L"MAIN_WINDOW");
                ShowWindow(hMainWnd, SW_SHOW);
                DestroyWindow(hDlg);
            }
            else {
                MessageBox(hDlg, L"�û������������", L"��¼ʧ��", MB_ICONERROR);
            }
        }
        break;
    }
    case WM_CLOSE:
        PostQuitMessage(0); // ֱ���˳�����
        break;
    default:
        return DefWindowProc(hDlg, msg, wParam, lParam);
    }
    return 0;
}

// ������¼�Ի���ĺ���
void CreateLoginDialog(HWND hMainWnd)
{
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = LoginDialogProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"LoginDialog";
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClass(&wc);

    HWND hDlg = CreateWindow(
        L"LoginDialog", L"��ְ������ϵͳ��¼",
        WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, 300, 200,
        hMainWnd, NULL, NULL, NULL);

    // �����ؼ�
    CreateWindow(L"STATIC", L"�û���:", WS_VISIBLE | WS_CHILD,
        20, 20, 60, 20, hDlg, NULL, NULL, NULL);
    CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER,
        90, 20, 180, 25, hDlg, (HMENU)IDC_LOGIN_USERNAME, NULL, NULL);

    CreateWindow(L"STATIC", L"����:", WS_VISIBLE | WS_CHILD,
        20, 60, 60, 20, hDlg, NULL, NULL, NULL);
    CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_PASSWORD,
        90, 60, 180, 25, hDlg, (HMENU)IDC_LOGIN_PASSWORD, NULL, NULL);

    CreateWindow(L"BUTTON", L"��¼", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        100, 110, 100, 30, hDlg, (HMENU)IDC_LOGIN_BUTTON, NULL, NULL);

    // ������ʾ
    RECT rc;
    GetWindowRect(hDlg, &rc);
    SetWindowPos(hDlg, NULL,
        (GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2,
        (GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 2,
        0, 0, SWP_NOSIZE);

    // �洢�����ھ��
    SetProp(hDlg, L"MAIN_WINDOW", hMainWnd);

    ShowWindow(hDlg, SW_SHOW);
    UpdateWindow(hDlg);
}

// 2. ���ҶԻ�����ʾ����
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

    // �������λ��
    RECT rcParent;
    GetWindowRect(hParent, &rcParent);
    int x = rcParent.left + (rcParent.right - rcParent.left - 350) / 2;
    int y = rcParent.top + (rcParent.bottom - rcParent.top - 200) / 2;

    // �������ҶԻ���
    HWND hDlg = CreateWindow(L"FindDialogClass", L"����ְ��",
        WS_POPUP | WS_CAPTION | WS_SYSMENU,
        x, y, 350, 200,
        hParent, NULL, NULL, NULL);


    // �����ؼ�
// 1. ID�����
    CreateWindow(L"STATIC", L"����ְ��ID:", WS_VISIBLE | WS_CHILD,
        20, 20, 80, 20, hDlg, NULL, NULL, NULL);
    HWND hEditId = CreateWindow(L"EDIT", L"",
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER | WS_TABSTOP,
        120, 20, 200, 25, hDlg, (HMENU)1101, NULL, NULL);

    // 2. ���Ұ�ť
    HWND hBtnFind = CreateWindow(L"BUTTON", L"����",
        WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | WS_TABSTOP,
        120, 70, 100, 30, hDlg, (HMENU)IDOK, NULL, NULL);

    // 3. ȡ����ť
    HWND hBtnCancel = CreateWindow(L"BUTTON", L"ȡ��",
        WS_VISIBLE | WS_CHILD | WS_TABSTOP,
        120, 120, 100, 30, hDlg, (HMENU)IDCANCEL, NULL, NULL);

    // ��ʾ�Ի���
    ShowWindow(hDlg, SW_SHOW);
    SetFocus(hEditId);

    // ��Ϣѭ��
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) && IsWindow(hDlg)) {
        if (!IsDialogMessage(hDlg, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

// 3. ���ҶԻ�����̺���
LRESULT CALLBACK FindDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case IDOK: {  // ���Ұ�ť
            // ��ȡ�����ID
            wchar_t idText[32] = { 0 };
            GetDlgItemText(hDlg, 1101, idText, 32);

            if (wcslen(idText) == 0) {
                MessageBox(hDlg, L"������ְ��ID", L"��ʾ", MB_ICONWARNING);
                break;
            }

            int id = _wtoi(idText);

            // ִ�в�ѯ
            bool found = false;
            std::wstring info;
            auto workers = g_WorkerManager.GetAllWorkers();

            for (auto worker : workers) {
                if (worker->m_Id == id) {
                    info = L"ְ��ID: " + std::to_wstring(worker->m_Id) + L"\n"
                        L"����: " + std::wstring(worker->m_Name.begin(), worker->m_Name.end()) + L"\n"
                        L"����: " + worker->getDeptName();
                    found = true;
                    break;
                }
            }

            // ��ʾ���
            MessageBox(hDlg,
                found ? info.c_str() : L"δ�ҵ�ָ��ID��ְ��",
                L"��ѯ���",
                MB_OK);

            // ���رնԻ��򣬿��Լ�����ѯ
            SetDlgItemText(hDlg, 1101, L"");
            SetFocus(GetDlgItem(hDlg, 1101));
            break;
        }
        case IDCANCEL:  // ȡ����ť
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

    HWND hDlg = CreateWindow(L"AddDialogClass", L"���ְ��",
        WS_POPUP | WS_CAPTION | WS_SYSMENU, 350, 250, 350, 250,
        hParent, NULL, NULL, NULL);


    // �ؼ���������...
      // ... [����ԭ����ӶԻ���ؼ��������벻��]
       // 3. �������пؼ�����֮ǰ��ͬ��
      // 3. �������пؼ�������ϸ����˵����
      // ��������
    CreateWindow(L"STATIC", L"ְ������:", WS_VISIBLE | WS_CHILD,
        20, 20, 80, 20, hDlg, NULL, NULL, NULL);
    HWND hEditId = CreateWindow(L"EDIT", L"",
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER | WS_TABSTOP,
        120, 20, 200, 25, hDlg, (HMENU)1008, NULL, NULL);

    // ��������
    CreateWindow(L"STATIC", L"ְ������:", WS_VISIBLE | WS_CHILD,
        20, 60, 80, 20, hDlg, NULL, NULL, NULL);
    HWND hEditName = CreateWindow(L"EDIT", L"",
        WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP,
        120, 60, 200, 25, hDlg, (HMENU)1009, NULL, NULL);

    // ְλѡ��
    CreateWindow(L"STATIC", L"ְλ����:", WS_VISIBLE | WS_CHILD,
        20, 100, 80, 20, hDlg, NULL, NULL, NULL);
    HWND hComboDept = CreateWindow(L"COMBOBOX", L"",
        WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | WS_TABSTOP,
        120, 100, 200, 150, hDlg, (HMENU)1010, NULL, NULL);

    // ��ʼ��������
    SendMessage(hComboDept, CB_ADDSTRING, 0, (LPARAM)L"��ͨԱ��");
    SendMessage(hComboDept, CB_ADDSTRING, 0, (LPARAM)L"���ž���");
    SendMessage(hComboDept, CB_ADDSTRING, 0, (LPARAM)L"��˾�ϰ�");
    SendMessage(hComboDept, CB_SETCURSEL, 0, 0);

    // ������ť
    HWND hBtnOk = CreateWindow(L"BUTTON", L"ȷ��",
        WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | WS_TABSTOP,
        80, 160, 80, 30, hDlg, (HMENU)IDOK, NULL, NULL);
    HWND hBtnCancel = CreateWindow(L"BUTTON", L"ȡ��",
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

// �޸ĺ�ĶԻ�����ʾ����
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

    // �������λ��
    RECT rcParent;
    GetWindowRect(hParent, &rcParent);
    int x = rcParent.left + (rcParent.right - rcParent.left - 350) / 2;
    int y = rcParent.top + (rcParent.bottom - rcParent.top - 250) / 2;

    // �޸�CreateWindowW���ã����pWorker����
    HWND hDlg = CreateWindowW(
        L"ModifyDialogClass",
        L"�޸�ְ����Ϣ",
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        x, y, 350, 250,
        hParent,
        NULL,
        GetModuleHandleW(NULL),
        (LPVOID)pWorker  // �ؼ��޸ģ�����ָ�����
    );

    // ���ô�������
    SetWindowLongPtrW(hDlg, GWLP_USERDATA, (LONG_PTR)pWorker);

     // ��Ϣѭ������Ҫ���⴦��
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
            // ��ȡ�ؼ����
            HWND hEditId = GetDlgItem(hDlg, 1008);
            HWND hEditName = GetDlgItem(hDlg, 1009);
            HWND hComboDept = GetDlgItem(hDlg, 1010);

            // ��ȡ��������
            wchar_t idText[32] = { 0 }, nameText[128] = { 0 };
            GetWindowText(hEditId, idText, 32);
            GetWindowText(hEditName, nameText, 128);
            int deptId = SendMessage(hComboDept, CB_GETCURSEL, 0, 0) + 1;

            // ������֤
            if (wcslen(idText) == 0 || wcslen(nameText) == 0) {
                MessageBox(hDlg, L"����д������Ϣ��", L"��ʾ", MB_ICONWARNING);
                break;
            }

            // ��ӵ�����ϵͳ
            std::wstring wname(nameText);
            g_WorkerManager.AddEmp(_wtoi(idText), std::string(wname.begin(), wname.end()), deptId);

            // ˢ���������б�
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

LRESULT CALLBACK ModifyDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    static Worker* pWorker = nullptr;

    switch (msg) {
    case WM_CREATE: {
        // ��ȷ��ȡ��������
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pWorker = reinterpret_cast<Worker*>(pCreate->lpCreateParams);

        if (!pWorker) {
            MessageBoxW(hDlg, L"���ݼ���ʧ�ܣ���Чָ��", L"����", MB_ICONERROR);
            return -1;  // ����-1����ֹ���ڴ���
        }

        // ��ָ�뱣�浽��������
        SetWindowLongPtrW(hDlg, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWorker));

        // �����ؼ�
        CreateWindowW(L"STATIC", L"ְ��ID:",
            WS_VISIBLE | WS_CHILD | SS_LEFT,
            20, 20, 80, 20, hDlg, NULL, NULL, NULL);

        // ID�༭��ֻ����
        CreateWindowW(L"EDIT", std::to_wstring(pWorker->m_Id).c_str(),
            WS_VISIBLE | WS_CHILD | ES_READONLY | WS_BORDER,
            120, 20, 200, 25, hDlg, (HMENU)IDC_ID_EDIT, NULL, NULL);

        // �����༭��
        CreateWindowW(L"STATIC", L"����:",
            WS_VISIBLE | WS_CHILD | SS_LEFT,
            20, 60, 80, 20, hDlg, NULL, NULL, NULL);

        std::wstring name(pWorker->m_Name.begin(), pWorker->m_Name.end());
        CreateWindowW(L"EDIT", name.c_str(),
            WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL | WS_BORDER | WS_TABSTOP,
            120, 60, 200, 25, hDlg, (HMENU)IDC_NAME_EDIT, NULL, NULL);

        // ����������
        CreateWindowW(L"STATIC", L"����:",
            WS_VISIBLE | WS_CHILD | SS_LEFT,
            20, 100, 80, 20, hDlg, NULL, NULL, NULL);

        HWND hCombo = CreateWindowW(L"COMBOBOX", L"",
            WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,
            120, 100, 200, 150, hDlg, (HMENU)IDC_DEPT_COMBO, NULL, NULL);

        // �������ѡ��
        const wchar_t* depts[] = { L"��ͨԱ��", L"���ž���", L"��˾�ϰ�" };
        for (int i = 0; i < 3; ++i) {
            SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)depts[i]);
        }
        SendMessageW(hCombo, CB_SETCURSEL, pWorker->m_DeptId - 1, 0);

        // ������ť
        CreateWindowW(L"BUTTON", L"ȷ��",
            WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | WS_TABSTOP,
            80, 160, 80, 30, hDlg, (HMENU)IDOK, NULL, NULL);

        CreateWindowW(L"BUTTON", L"ȡ��",
            WS_VISIBLE | WS_CHILD | WS_TABSTOP,
            200, 160, 80, 30, hDlg, (HMENU)IDCANCEL, NULL, NULL);

        SetFocus(GetDlgItem(hDlg, IDC_NAME_EDIT));
        return 0;  // ���뷵��0��ʾ�ɹ�����
    }
    case WM_COMMAND: {
        pWorker = reinterpret_cast<Worker*>(GetWindowLongPtrW(hDlg, GWLP_USERDATA));

        switch (LOWORD(wParam)) {
        case IDOK: {
            if (!pWorker) {
                MessageBoxW(hDlg, L"���ݴ����쳣", L"����", MB_ICONERROR);
                break;
            }

            // ��ȡ��������
            wchar_t nameBuf[128] = { 0 };
            GetDlgItemTextW(hDlg, IDC_NAME_EDIT, nameBuf, _countof(nameBuf));
            if (wcslen(nameBuf) == 0) {
                MessageBoxW(hDlg, L"��������Ϊ��", L"�������", MB_ICONWARNING);
                break;
            }

            // ��ȡ����ѡ��
            int deptSel = SendDlgItemMessageW(hDlg, IDC_DEPT_COMBO, CB_GETCURSEL, 0, 0);
            if (deptSel == CB_ERR) {
                MessageBoxW(hDlg, L"��ѡ����Ч����", L"�������", MB_ICONWARNING);
                break;
            }

            // ��������
            pWorker->m_Name = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(nameBuf);
            pWorker->m_DeptId = deptSel + 1;

            // g_WorkerManager.SaveToFile();

             // ˢ��������

            HWND hParent = GetParent(hDlg);
            if (hParent) {

                HWND hList = GetDlgItem(hParent, IDC_LISTVIEW);
                RefreshListView(hList);
            }

            DestroyWindow(hDlg);
            return TRUE;
        }
        case IDCANCEL:
            DestroyWindow(hDlg);
            return TRUE;
        }
        break;
    }
    case WM_CLOSE:
        DestroyWindow(hDlg);
        return 0;
    case WM_DESTROY:
        RemoveProp(hDlg, L"PARENT_HWND");
        // PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProcW(hDlg, msg, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        INITCOMMONCONTROLSEX icex = { sizeof(INITCOMMONCONTROLSEX), ICC_LISTVIEW_CLASSES };
        InitCommonControlsEx(&icex);

        HWND hList = CreateWindow(WC_LISTVIEW, L"",
            WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SINGLESEL,
            10, 10, 760, 400, hWnd, (HMENU)IDC_LISTVIEW, NULL, NULL);

        LVCOLUMN lvc = { LVCF_WIDTH | LVCF_TEXT };
        lvc.cx = 100;
        lvc.pszText = const_cast<LPWSTR>(L"ID");
        ListView_InsertColumn(hList, 0, &lvc);

        lvc.cx = 150;
        lvc.pszText = const_cast<LPWSTR>(L"����");
        ListView_InsertColumn(hList, 1, &lvc);

        lvc.cx = 200;
        lvc.pszText = const_cast<LPWSTR>(L"ְλ");
        ListView_InsertColumn(hList, 2, &lvc);

        int y = 420;
        const struct {
            int id; LPCWSTR text;
        } buttons[] = {
            {IDC_ADD_BUTTON, L"���ְ��"},
            {IDC_DELETE_BUTTON, L"ɾ��ְ��"},
            {IDC_MODIFY_BUTTON, L"�޸�ְ��"},
            {IDC_FIND_BUTTON, L"����ְ��"},
            {IDC_SORT_BUTTON, L"����"},
            {IDC_CLEAR_BUTTON, L"�������"}
        };

        for (int i = 0; i < 6; ++i) {
            CreateWindow(L"BUTTON", buttons[i].text, WS_VISIBLE | WS_CHILD,
                20 + i * 120, y, 100, 30, hWnd, (HMENU)buttons[i].id, NULL, NULL);
        }

        RefreshListView(hList);
        break;
    }

    case WM_COMMAND: {
        HWND hList = GetDlgItem(hWnd, IDC_LISTVIEW);

        switch (LOWORD(wParam)) {
        case IDC_ADD_BUTTON:
            ShowAddDialog(hWnd);
            break;

        case IDC_MODIFY_BUTTON: {
            int selected = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
            if (selected == -1) {
                MessageBox(hWnd, L"����ѡ��ְ��", L"��ʾ", MB_ICONINFORMATION);
                break;
            }

            LVITEM item = { 0 };
            item.mask = LVIF_PARAM;
            item.iItem = selected;
            if (!ListView_GetItem(hList, &item)) {
                MessageBox(hWnd, L"��ȡѡ����ʧ��", L"����", MB_ICONERROR);
                break;
            }

            Worker* pWorker = reinterpret_cast<Worker*>(item.lParam);
            if (!pWorker) {
                MessageBox(hWnd, L"��Ч��ְ������", L"����", MB_ICONERROR);
                break;
            }

            ShowModifyDialog(hWnd, pWorker); // ȷ��������Чָ��
            RefreshListView(hList);
            break;
        }

        case IDC_DELETE_BUTTON: {
            int sel = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
            if (sel == -1) {
                MessageBox(hWnd, L"����ѡ��Ҫɾ����ְ��", L"��ʾ", MB_ICONINFORMATION);
                break;
            }

            LVITEM item = { LVIF_PARAM };
            item.iItem = sel;
            ListView_GetItem(hList, &item);
            Worker* pWorker = reinterpret_cast<Worker*>(item.lParam);

            if (pWorker) {
                g_WorkerManager.DeleteEmp(pWorker->m_Id);
                RefreshListView(hList);
            }
            break;
        }


                              // 4. �������ڵ�IDC_FIND_BUTTON�����е���
        case IDC_FIND_BUTTON: {
            wchar_t idText[32];
            if (GetDlgItemText(hWnd, IDC_ID_EDIT, idText, 32) == 0) {
                MessageBox(hWnd, L"������Ҫ���ҵ�ְ��ID", L"��ʾ", MB_ICONINFORMATION);
                break;
            }

            g_WorkerManager.FindEmp(_wtoi(idText));
            break;
        }

        case IDC_SORT_BUTTON: {
            g_WorkerManager.SortById();
            RefreshListView(hList);
            break;
        }



        case IDC_CLEAR_BUTTON:
            if (MessageBox(hWnd, L"ȷ��Ҫ�������������", L"����",
                MB_YESNO | MB_ICONWARNING) == IDYES) {
                g_WorkerManager.CleanFile();
                RefreshListView(hList);
            }
            break;
        }
        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

// �޸ĺ��WinMain����
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    // 1. ע����������
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"StaffManagement";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wc);

    // 2. ����������(����������ʾ)
    HWND hWnd = CreateWindow(L"StaffManagement", L"��ְ������ϵͳ",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        800, 600, NULL, NULL, hInstance, NULL);

    // 3. ��������ʾ��¼�Ի���
    CreateLoginDialog(hWnd);

    // 4. ��Ϣѭ��
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}