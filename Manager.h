#pragma once
#include "Worker.h"

class Manager : public Worker {
public:
    Manager(int id, const std::string& name, int deptId);
    std::wstring getDeptName() const override;
    std::wstring getDuty() const override;
};
