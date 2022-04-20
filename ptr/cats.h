#pragma once
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace std;


// ������ �����
enum class CatBreed {
    Bengal,
    Balinese,
    Persian,
    Siamese,
    Siberian,
    Sphynx, SuccessSuccess
};

// ���
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

// ���������� ������ ���������� �� �������� ������� cats, ��������������� � ��������������
// ����������� comp. ���������� comp - �������, ����������� ��� ��������� ���� const Cat&
// � ������������ true, ���� �������� �����������, � false � ���� ������
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
    �������� ���� ������� ��������������. ���������:
    1) ��������� � ������ sorted_cat_pointers ������ �������� �� ������� cats.
    2) ������������ ������ sorted_cat_pointers � ������� ����������� ����������� comp.
       ��� ��� comp ���������� ������ �� �������, � ������������� ����� ���������,
       ��������� � sort ������ �������, ����������� ��������� � ������������ �������
       ��� ������ ����������� comp:
       [comp](const Cat* lhs, const Cat* rhs) {
           return comp(*lhs, *rhs);
       }
    */
    return sorted_cat_pointers;
}

// ������� � ����� out �������� ��������, �� ������� ��������� ��������� ������� cat_pointers.
// ������ ������ ��������� vector<const Cat*>:
// {{Tom, male, breed: Bengal, age:2}, {Charlie, male, breed: Balinese, age:7}}
void PrintCatPointerValues(const vector<const Cat*>& cat_pointers, ostream& out) {
    // �������� ������� ��������������
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
