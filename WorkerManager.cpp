#include "WorkerManager.h"
#include <fstream>
#include <algorithm>
#include "Employee.h"  // ����Employee�ඨ��
#include "Manager.h"   // ����Manager�ඨ��
#include "Boss.h"      // ����Boss�ඨ��

#define FILENAME "D:\\employee.dat"

WorkerManager::WorkerManager() : m_EmpNum(0), m_EmpArray(nullptr), m_FileIsEmpty(true) {
    loadFromFile();
}

WorkerManager::~WorkerManager() {
    if (m_EmpArray) {
        for (int i = 0; i < m_EmpNum; i++) {
            delete m_EmpArray[i];
        }
        delete[] m_EmpArray;
    }
}

void WorkerManager::AddEmp(int id, const std::string& name, int deptId) {
    Worker* worker = nullptr;
    switch (deptId) {
    case 1: worker = new Employee(id, name, deptId); break;
    case 2: worker = new Manager(id, name, deptId); break;
    case 3: worker = new Boss(id, name, deptId); break;
    default: return;
    }

    Worker** newArray = new Worker * [m_EmpNum + 1];
    for (int i = 0; i < m_EmpNum; i++) {
        newArray[i] = m_EmpArray[i];
    }
    newArray[m_EmpNum] = worker;

    delete[] m_EmpArray;
    m_EmpArray = newArray;
    m_EmpNum++;

    saveToFile();
}

void WorkerManager::ShowAll() {
    auto workers = GetAllWorkers();
    for (auto worker : workers) {
        std::wstring name(worker->m_Name.begin(), worker->m_Name.end());
        MessageBoxW(NULL,
            (L"ID: " + std::to_wstring(worker->m_Id) + L"\n" +
                L"����: " + name + L"\n" +
                L"ְλ: " + worker->getDeptName() + L"\n" +
                L"ְ��: " + worker->getDuty()).c_str(),
            L"ְ����Ϣ", MB_OK);
    }
}

void WorkerManager::DeleteEmp(int id) {
    int index = getWorkerIndex(id);
    if (index == -1) {
        MessageBoxW(NULL, L"δ�ҵ���ְ��", L"����", MB_ICONERROR);
        return;
    }

    delete m_EmpArray[index];
    for (int i = index; i < m_EmpNum - 1; i++) {
        m_EmpArray[i] = m_EmpArray[i + 1];
    }
    m_EmpNum--;

    Worker** newArray = new Worker * [m_EmpNum];
    for (int i = 0; i < m_EmpNum; i++) {
        newArray[i] = m_EmpArray[i];
    }

    delete[] m_EmpArray;
    m_EmpArray = newArray;

    saveToFile();
    MessageBoxW(NULL, L"ɾ���ɹ�", L"��ʾ", MB_OK);
}


void WorkerManager::ModifyEmp(int id) {
    int index = getWorkerIndex(id);
    if (index == -1) {
        MessageBoxW(NULL, L"δ�ҵ���ְ��", L"����", MB_ICONERROR);
        return;
    }

    // ��ȡ����Ϣ��ʵ��Ӧ����Ӧ��ͨ���Ի����ȡ��
    std::string newName = "�޸ĺ������";
    int newDeptId = 1; // Ĭ��Ϊ��ͨԱ��

    delete m_EmpArray[index];

    switch (newDeptId) {
    case 1: m_EmpArray[index] = new Employee(id, newName, newDeptId); break;
    case 2: m_EmpArray[index] = new Manager(id, newName, newDeptId); break;
    case 3: m_EmpArray[index] = new Boss(id, newName, newDeptId); break;
    }

    saveToFile();
    MessageBoxW(NULL, L"�޸ĳɹ�", L"��ʾ", MB_OK);
}


void WorkerManager::FindEmp(int id) {
    int index = getWorkerIndex(id);
    if (index == -1) {
        MessageBoxW(NULL, L"δ�ҵ���ְ��", L"����", MB_ICONERROR);
        return;
    }

    Worker* worker = m_EmpArray[index];
    std::wstring name(worker->m_Name.begin(), worker->m_Name.end());
    std::wstring message = L"�ҵ�ְ��:\n"
        L"ID: " + std::to_wstring(worker->m_Id) + L"\n" +
        L"����: " + std::wstring(name.begin(), name.end()) + L"\n" +
        L"ְλ: " + worker->getDeptName();

    MessageBoxW(NULL, message.c_str(), L"���ҽ��", MB_OK);
}

void WorkerManager::SortById() {
    std::sort(m_EmpArray, m_EmpArray + m_EmpNum, [](Worker* a, Worker* b) {
        return a->m_Id < b->m_Id;
        });
    saveToFile();
    MessageBoxW(NULL, L"�Ѱ�ID����", L"��ʾ", MB_OK);
}

void WorkerManager::CleanFile() {
    if (MessageBoxW(NULL, L"ȷ��Ҫ�������������", L"����", MB_YESNO | MB_ICONWARNING) == IDNO) {
        return;
    }

    for (int i = 0; i < m_EmpNum; i++) {
        delete m_EmpArray[i];
    }
    delete[] m_EmpArray;

    m_EmpNum = 0;
    m_EmpArray = nullptr;
    m_FileIsEmpty = true;

    std::ofstream ofs(FILENAME, std::ios::trunc);
    ofs.close();

    MessageBoxW(NULL, L"�������������", L"��ʾ", MB_OK);
}

std::vector<Worker*> WorkerManager::GetAllWorkers() const {
    return std::vector<Worker*>(m_EmpArray, m_EmpArray + m_EmpNum);
}

void WorkerManager::saveToFile() {
    std::ofstream ofs(FILENAME, std::ios::binary);
    if (!ofs) return;

    for (int i = 0; i < m_EmpNum; i++) {
        Worker* worker = m_EmpArray[i];
        size_t nameLength = worker->m_Name.size();

        ofs.write(reinterpret_cast<const char*>(&worker->m_Id), sizeof(int));
        ofs.write(reinterpret_cast<const char*>(&nameLength), sizeof(size_t));
        ofs.write(worker->m_Name.c_str(), nameLength);
        ofs.write(reinterpret_cast<const char*>(&worker->m_DeptId), sizeof(int));
    }

    m_FileIsEmpty = (m_EmpNum == 0);
    ofs.close();
}

void WorkerManager::loadFromFile() {
    std::ifstream ifs(FILENAME, std::ios::binary);
    if (!ifs) {
        m_FileIsEmpty = true;
        return;
    }

    // �����¼��
    ifs.seekg(0, std::ios::end);
    std::streampos fileSize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    if (fileSize == 0) {
        m_FileIsEmpty = true;
        return;
    }

    std::vector<Worker*> workers;
    while (ifs.peek() != EOF) {
        int id, deptId;
        size_t nameLength;
        std::string name;

        ifs.read(reinterpret_cast<char*>(&id), sizeof(int));
        ifs.read(reinterpret_cast<char*>(&nameLength), sizeof(size_t));
        name.resize(nameLength);
        ifs.read(&name[0], nameLength);
        ifs.read(reinterpret_cast<char*>(&deptId), sizeof(int));

        Worker* worker = nullptr;
        switch (deptId) {
        case 1: worker = new Employee(id, name, deptId); break;
        case 2: worker = new Manager(id, name, deptId); break;
        case 3: worker = new Boss(id, name, deptId); break;
        }

        if (worker) workers.push_back(worker);
    }

    m_EmpNum = workers.size();
    m_EmpArray = new Worker * [m_EmpNum];
    for (int i = 0; i < m_EmpNum; i++) {
        m_EmpArray[i] = workers[i];
    }

    m_FileIsEmpty = (m_EmpNum == 0);
    ifs.close();
}

int WorkerManager::getWorkerIndex(int id) const {
    for (int i = 0; i < m_EmpNum; i++) {
        if (m_EmpArray[i]->m_Id == id) {
            return i;
        }
    }
    return -1;
}