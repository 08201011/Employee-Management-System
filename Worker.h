#pragma once
#include <string>
#include <windows.h>

class Worker {
public:
    virtual ~Worker() {}

    int m_Id;           // 职工编号
    std::string m_Name; // 职工姓名
    int m_DeptId;       // 部门编号

    virtual std::wstring getDeptName() const = 0;
    virtual std::wstring getDuty() const = 0;
};
