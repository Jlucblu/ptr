#pragma once
#include <new> // ��� ���������� bad_alloc
#include <vector>
#include <stdexcept>

#include "ptrvector.h"
#include "scopedptr.h"

using namespace std;


// ��������
class Tentacle {
public:
    explicit Tentacle(int id) noexcept
        : id_(id) {
    }

    int GetId() const noexcept {
        return id_;
    }

    Tentacle* GetLinkedTentacle() const noexcept {
        return linked_tentacle_;
    }
    void LinkTo(Tentacle& tentacle) noexcept {
        linked_tentacle_ = &tentacle;
    }
    void Unlink() noexcept {
        linked_tentacle_ = nullptr;
    }

private:
    int id_ = 0;
    Tentacle* linked_tentacle_ = nullptr;
};

// ��������
class Octopus {
public:
    Octopus()
        : Octopus(8) {
    }

    explicit Octopus(int num_tentacles) {
        Tentacle* t = nullptr;
        try {
            for (int i = 1; i <= num_tentacles; ++i) {
                t = new Tentacle(i);      // ����� ��������� ���������� bad_alloc
                tentacles_.GetItems().push_back(t);  // ����� ��������� ���������� bad_alloc

                // �������� ��������� �� ��������, ������� ��� �������� � tentacles_,
                // ����� �� ������� ��� � ����������� catch ��������
                t = nullptr;
            }
        }
        catch (const std::bad_alloc&) {
            // ������� ��������, ������� ������ ������� � ��������� tentacles_
            // ������� ��������, ������� �������, �� �� �������� � tentacles_
            delete t;
            // ����������� �� ���� ������� ��������� � ������� ����������,
            // ������� ����������� ����������, ����� �������� ����������� ���� �� ������
            // throw ��� ���������� ������ catch ��������� ���������� ���������� ����������
            throw;
        }
    }

    ~Octopus() {
        // �������� ������� ��������� � ������������ ������ (��������),
        // ������� ������ ���� ������� ��� ��� ����������.
        // ���������� - ������ �����, ����� ���������� �� �����.
    }

    Tentacle& AddTentacle() {
        Tentacle* t = new Tentacle(tentacles_.GetItems().size() + 1);
        tentacles_.GetItems().push_back(t);
        return *t;
    }

    // ��������� ����� �������� � ���������������,
    // ������ (����������_������� + 1):
    // 1, 2, 3, ...
    // ���������� ������ �� ����������� ��������
    // Tentacle& AddTentacle() {
    //     ���������� ���������� �������� ��������������
    // }

    int GetTentacleCount() const noexcept {
        return static_cast<int>(tentacles_.GetItems().size());
    }

    const Tentacle& GetTentacle(size_t index) const {
        return *tentacles_.GetItems().at(index);
    }
    Tentacle& GetTentacle(size_t index) {
        return *tentacles_.GetItems().at(index);
    }

private:
    // ������ ������ ��������� �� ��������. ���� ������� ������� ��������� � ����
    PtrVector<Tentacle> tentacles_;
};
