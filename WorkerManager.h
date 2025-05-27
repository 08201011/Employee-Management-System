#pragma once
#include <vector>
#include "Worker.h"

#define FILENAME L"employees.dat"

class WorkerManager {
public:
    WorkerManager();
    ~WorkerManager();

    void AddEmp(int id, const std::string& name, int deptId);
    void ShowAll();
    void DeleteEmp(int id);
    void ModifyEmp(int id);

    void FindEmp(int id);
    void SortById();
    void CleanFile();

    std::vector<Worker*> GetAllWorkers() const;

private:
    int m_EmpNum;
    Worker** m_EmpArray;
    bool m_FileIsEmpty;

    void saveToFile();
    void loadFromFile();
    int getWorkerIndex(int id) const;
};
