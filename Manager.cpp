#include "Manager.h"

Manager::Manager(int id, const std::string& name, int deptId) {
    this->m_Id = id;
    this->m_Name = name;
    this->m_DeptId = deptId;
}

std::wstring Manager::getDeptName() const {
    return L"����";
}

std::wstring Manager::getDuty() const {
    return L"����ϰ彻�������񣬲��·������Ա��";
}