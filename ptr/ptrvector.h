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

    // Создаёт вектор указателей на копии объектов из other
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

    // Деструктор удаляет объекты в куче, на которые ссылаются указатели,
    // в векторе items_
    ~PtrVector() {
        for (auto* del_ptr : items_) {
            delete del_ptr;
        }
    }

    // Возвращает ссылку на вектор указателей+
    vector<T*>& GetItems() noexcept {
        return items_;
    }

    // Возвращает константную ссылку на вектор указателей
    vector<T*> const& GetItems() const noexcept {
        return items_;
    }

    PtrVector& operator=(const PtrVector& rhs) {
        if (this != &rhs) {
            // Реализация операции присваивания с помощью идиомы Copy-and-swap.
            // Если исключение будет выброшено, то на текущий объект оно не повлияет.
            auto rhs_copy(rhs);

            // rhs_copy содержит копию правого аргумента.
            // Обмениваемся с ним данными.
            swap(rhs_copy);

            // Теперь текущий объект содержит копию правого аргумента,
            // а rhs_copy - прежнее состояние текущего объекта, которое при выходе
            // из блока будет разрушено.
        }

        return *this;
    }

    void swap(PtrVector& other) noexcept {
        std::swap(items_, other.items_);
    }

private:
    vector<T*> items_;
};
