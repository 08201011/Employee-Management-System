#pragma once
#include "Worker.h"

class Employee : public Worker {
public:
    Employee(int id, const std::string& name, int deptId);
    std::wstring getDeptName() const override;
    std::wstring getDuty() const override;
};
