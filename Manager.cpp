#include "Manager.h"

Manager::Manager(int id, const std::string& name, int deptId) {
    this->m_Id = id;
    this->m_Name = name;
    this->m_DeptId = deptId;
}

std::wstring Manager::getDeptName() const {
    return L"经理";
}

std::wstring Manager::getDuty() const {
    return L"完成老板交给的任务，并下发任务给员工";
}