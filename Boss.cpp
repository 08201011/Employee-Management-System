#include "Boss.h"

Boss::Boss(int id, const std::string& name, int deptId) {
    this->m_Id = id;
    this->m_Name = name;
    this->m_DeptId = deptId;
}

std::wstring Boss::getDeptName() const {
    return L"老板";
}

std::wstring Boss::getDuty() const {
    return L"管理公司所有事物";
}
