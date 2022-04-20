#pragma once
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace std;


// Породы кошек
enum class CatBreed {
    Bengal,
    Balinese,
    Persian,
    Siamese,
    Siberian,
    Sphynx, SuccessSuccess
};

// Пол
enum class Gender {
    Male,
    Female,
};

struct Cat {
    string name;
    Gender gender;
    CatBreed breed;
    int age;
};

string CatBreedToString(CatBreed breed) {
    switch (breed) {
    case CatBreed::Bengal:
        return "Bengal"s;
    case CatBreed::Balinese:
        return "Balinese"s;
    case CatBreed::Persian:
        return "Persian"s;
    case CatBreed::Siamese:
        return "Siamese"s;
    case CatBreed::Siberian:
        return "Siberian";
    case CatBreed::Sphynx:
        return "Sphynx"s;
    default:
        throw invalid_argument("Invalid cat breed"s);
    }
}

ostream& operator<<(ostream& out, CatBreed breed) {
    out << CatBreedToString(breed);
    return out;
}

ostream& operator<<(ostream& out, Gender gender) {
    out << (gender == Gender::Male ? "male"s : "female"s);
    return out;
}

ostream& operator<<(ostream& out, const Cat& cat) {
    out << '{' << cat.name << ", "s << cat.gender;
    out << ", breed: "s << cat.breed << ", age:"s << cat.age << '}';
    return out;
}

// Возвращает массив указателей на элементы вектора cats, отсортированные с использованием
// компаратора comp. Компаратор comp - функция, принимающая два аргумента типа const Cat&
// и возвращающая true, если значения упорядочены, и false в ином случае
template <typename Comparator>
vector<const Cat*> GetSortedCats(const vector<Cat>& cats, const Comparator& comp) {
    vector<const Cat*> sorted_cat_pointers;
    for (const auto& cat : cats) {
        sorted_cat_pointers.push_back(&cat);
    }

    sort(sorted_cat_pointers.begin(), sorted_cat_pointers.end(), [comp](const auto* lhs, const auto* rhs) {
        return comp(*lhs, *rhs);
        });

    /*
    Напишите тело функции самостоятельно. Подсказка:
    1) Поместите в массив sorted_cat_pointers адреса объектов из массива cats.
    2) Отсортируйте массив sorted_cat_pointers с помощью переданного компаратора comp.
       Так как comp сравнивает ссылки на объекты, а отсортировать нужно указатели,
       передайте в sort лямбда функцию, принимающую указатели и сравнивающую объекты
       при помощи компаратора comp:
       [comp](const Cat* lhs, const Cat* rhs) {
           return comp(*lhs, *rhs);
       }
    */
    return sorted_cat_pointers;
}

// Выводит в поток out значения объектов, на который ссылаются указатели вектора cat_pointers.
// Пример вывода элементов vector<const Cat*>:
// {{Tom, male, breed: Bengal, age:2}, {Charlie, male, breed: Balinese, age:7}}
void PrintCatPointerValues(const vector<const Cat*>& cat_pointers, ostream& out) {
    // Напишите функцию самостоятельно
    out << '{';
    bool is_first = true;
    for (auto cat_ptr : cat_pointers) {
        if (!is_first) {
            out << ", "s;
        }
        out << *cat_ptr;
        is_first = false;
    }

    out << '}';
}
