#pragma once
#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <new>
#include <stdexcept>

using namespace std;

template <typename T>
class PtrVector {
public:
    PtrVector() = default;

    // ������ ������ ���������� �� ����� �������� �� other
    PtrVector(const PtrVector& other) {
        T* ptr_ = nullptr;
        items_.reserve(other.items_.size());
        try {
            for (auto ptr : other.items_) {
                if (ptr == nullptr) {
                    items_.push_back(nullptr);
                }
                else {
                    ptr_ = new T(*ptr);
                    items_.push_back(ptr_);
                    ptr_ = nullptr;
                }
            }
        }
        catch (const bad_alloc&) {
            delete ptr_;
            throw bad_alloc();
        }
    }

    // ���������� ������� ������� � ����, �� ������� ��������� ���������,
    // � ������� items_
    ~PtrVector() {
        for (auto* del_ptr : items_) {
            delete del_ptr;
        }
    }

    // ���������� ������ �� ������ ����������+
    vector<T*>& GetItems() noexcept {
        return items_;
    }

    // ���������� ����������� ������ �� ������ ����������
    vector<T*> const& GetItems() const noexcept {
        return items_;
    }

    PtrVector& operator=(const PtrVector& rhs) {
        if (this != &rhs) {
            // ���������� �������� ������������ � ������� ������ Copy-and-swap.
            // ���� ���������� ����� ���������, �� �� ������� ������ ��� �� ��������.
            auto rhs_copy(rhs);

            // rhs_copy �������� ����� ������� ���������.
            // ������������ � ��� �������.
            swap(rhs_copy);

            // ������ ������� ������ �������� ����� ������� ���������,
            // � rhs_copy - ������� ��������� �������� �������, ������� ��� ������
            // �� ����� ����� ���������.
        }

        return *this;
    }

    void swap(PtrVector& other) noexcept {
        std::swap(items_, other.items_);
    }

private:
    vector<T*> items_;
};
