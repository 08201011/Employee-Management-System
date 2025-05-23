#pragma once
#include <string>
#include <windows.h>

class Worker {
public:
    virtual ~Worker() {}

    int m_Id;           // ְ�����
    std::string m_Name; // ְ������
    int m_DeptId;       // ���ű��

    virtual std::wstring getDeptName() const = 0;
    virtual std::wstring getDuty() const = 0;
};
