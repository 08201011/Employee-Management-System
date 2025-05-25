#pragma once
#include "Worker.h"

class Boss : public Worker {
public:
    Boss(int id, const std::string& name, int deptId);
    std::wstring getDeptName() const override;
    std::wstring getDuty() const override;
};