#include "Employee.h"

Employee::Employee(int id, const std::string& name, int deptId) {
    this->m_Id = id;
    this->m_Name = name;
    this->m_DeptId = deptId;
}

std::wstring Employee::getDeptName() const {
    return L"普通员工";
}

std::wstring Employee::getDuty() const {
    return L"完成经理交给的任务";
}
