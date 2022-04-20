#pragma once
#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <new>
#include <stdexcept>

using namespace std;


// ����� ���������, ��������� ��������� ������ ��� ���� ����������.
// �������� ������� T ����� ��� �������, �� ������� ��������� ���������
template <typename T>
class ScopedPtr {
public:
    // ����������� �� ��������� ������ ������� ���������,
    // ��� ��� ���� ptr_ ����� �������� �� ��������� nullptr
    ScopedPtr() = default;

    // ������ ���������, ����������� �� ���������� raw_ptr.
    // raw_ptr ��������� ���� �� ������, ��������� � ���� ��� ������ new,
    // ���� �������� ������� ����������
    // ������������ noexcept ����������, ��� ����� �� ������� ����������
    explicit ScopedPtr(T* raw_ptr) noexcept
        : ptr_(raw_ptr) {}

    // ������� � ������ ����������� �����������
    ScopedPtr(const ScopedPtr&) = delete;

    // ����������. ������� ������, �� ������� ��������� ����� ���������.
    ~ScopedPtr() {
        delete ptr_;
    }

    // ���������� ���������, ���������� ������ ScopedPtr
    T* GetRawPtr() const noexcept {
        return ptr_;
    }

    // ���������� �������� ��������, �� ������� ��������� ����� ���������.
    // ���������� ������� �������� "������" ��������� � ������������� ���� ptr_ � null
    T* Release() noexcept {
        T* p = ptr_;
        ptr_ = nullptr;
        return p;
    }

    // �������� ���������� � ���� bool ��������� ������, ��������� �� ����� ���������
    // �� �����-���� ������
    explicit operator bool() const noexcept {
        return (ptr_ != nullptr ? true : false);
    }

    // �������� ������������� ���������� ������ �� ������
    // ����������� ���������� std::logic_error, ���� ��������� �������
    T& operator*() const {
        if (!ptr_) {
            throw std::logic_error("Scoped ptr is null"s);
        }
        return *ptr_;
    }

    // �������� -> ������ ���������� ��������� �� ������
    // ����������� ���������� std::logic_error, ���� ��������� �������
    T* operator->() const {
        if (!ptr_) {
            throw std::logic_error("Scoped ptr is null"s);
        }
        return ptr_;
    }

private:
    T* ptr_ = nullptr;
};
