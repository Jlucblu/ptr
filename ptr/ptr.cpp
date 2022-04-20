#include "scopedptr.h"
#include "cats.h"
#include "octopus.h"
#include "ptrvector.h"
#include "arrayptr.h"
#include "singlelinkedlist.h"


template <typename Function>
void RunTestImpl(const Function& func, const string& function_name) {
    func();
    cerr << function_name << " OK" << endl;
}

#define RUN_TEST(func) RunTestImpl(func, #func)

// Этот main тестирует Cats
void TestCats() {
    const vector<Cat> cats = {
        {"Tom"s, Gender::Male, CatBreed::Bengal, 2},
        {"Leo"s, Gender::Male, CatBreed::Siberian, 3},
        {"Luna"s, Gender::Female, CatBreed::Siamese, 1},
        {"Charlie"s, Gender::Male, CatBreed::Balinese, 7},
        {"Ginger"s, Gender::Female, CatBreed::Sphynx, 5},
        {"Tom"s, Gender::Male, CatBreed::Siamese, 2},
    };

    {
        auto sorted_cats = GetSortedCats(cats, [](const Cat& lhs, const Cat& rhs) {
            return tie(lhs.breed, lhs.name) < tie(rhs.breed, rhs.name);
            });

        cout << "Cats sorted by breed and name:"s << endl;
        PrintCatPointerValues(sorted_cats, cout);
        cout << endl;
    }

    {
        auto sorted_cats = GetSortedCats(cats, [](const Cat& lhs, const Cat& rhs) {
            return tie(lhs.gender, lhs.breed) < tie(rhs.gender, rhs.breed);
            });

        cout << "Cats sorted by gender and breed:"s << endl;
        PrintCatPointerValues(sorted_cats, cout);
        cout << endl;
    }
}

// Этот main тестирует класс ScopedPtr
void TestScoped1() {
    // Вспомогательный "шпион", позволяющий узнать о своём удалении
    struct DeletionSpy {
        explicit DeletionSpy(bool& is_deleted)
            : is_deleted_(is_deleted) {
        }
        ~DeletionSpy() {
            is_deleted_ = true;
        }
        bool& is_deleted_;
    };

    // Проверяем автоматическое удаление
    {
        bool is_deleted = false;
        {
            // настраиваем "шпион", чтобы при своём удалении он выставил is_deleted в true
            DeletionSpy* raw_ptr = new DeletionSpy(is_deleted);
            ScopedPtr<DeletionSpy> p(raw_ptr);
            assert(p.GetRawPtr() == raw_ptr);
            assert(!is_deleted);
            // При выходе из блока деструктор p должен удалить "шпиона"
        }
        // Если деструктор умного указателя работает правильно, шпион перед своей "смертью"
        // должен выставить is_deleted в true
        assert(is_deleted);
    }

    // Проверяем работу метода Release
    {
        bool is_deleted = false;
        DeletionSpy* raw_ptr = new DeletionSpy(is_deleted);
        {
            ScopedPtr<DeletionSpy> scoped_ptr(raw_ptr);
            assert(scoped_ptr.Release() == raw_ptr);
            assert(scoped_ptr.GetRawPtr() == nullptr);
            // После Release умный указатель не ссылается на объект и не удаляет его при своём удалении
        }
        assert(!is_deleted);
        delete raw_ptr;
        assert(is_deleted);
    }
}

// Этот main тестирует класс ScopedPtr
void TestScoped2() {
    // Проверка работы оператора приведения к типу bool
    {
        // Для нулевого указателя приведение к типу bool возвращает false
        const ScopedPtr<int> empty_ptr;
        assert(!empty_ptr);

        // Для ненулевого указателя приведение к типу bool возвращает true
        const ScopedPtr<int> ptr_to_existing_object(new int(0));
        assert(ptr_to_existing_object);

        static_assert(noexcept(static_cast<bool>(ptr_to_existing_object)));
    }

    // Проверка работы оператора разыменования *
    {
        string* raw_ptr = new string("hello");
        ScopedPtr<string> smart_ptr(raw_ptr);
        // Ссылка, возвращаемая оператором разыменования, должна ссылаться на объект,
        // на который указывает умный указатель
        assert(&*smart_ptr == raw_ptr);

        try {
            ScopedPtr<int> empty_ptr;
            // При попытке разыменовать пустой указатель должно быть выброшено
            // исключение logic_error
            *empty_ptr;
            // Сюда попасть мы не должны
            assert(false);
        }
        catch (const logic_error&) {
            // мы там, где нужно
        }
        catch (...) {
            // Других исключений выбрасываться не должно
            assert(false);
        }
    }

    // Проверка работы оператора ->
    {
        string* raw_ptr = new string("hello");
        ScopedPtr<string> smart_ptr(raw_ptr);
        // Доступ к членам класса через умный указатель должен быть аналогичен
        // доступу через "сырой" указатель
        assert(smart_ptr->data() == raw_ptr->data());

        try {
            ScopedPtr<string> empty_ptr;
            // При попытке разыменовать пустой указатель должно быть выброшено
            // исключение logic_error
            empty_ptr->clear();
            // Сюда попасть мы не должны
            assert(false);
        }
        catch (const logic_error&) {
            // мы там, где нужно
        }
        catch (...) {
            // Других исключений выбрасываться не должно
            assert(false);
        }
    }

    // Пример использования
    {
        // На этой структуре будет проверяться работа умного указателя
        struct Object {
            Object() {
                cout << "Object is default constructed"s << endl;
            }
            void DoSomething() {
                cout << "Doing something"s << endl;
            }
            ~Object() {
                cout << "Object is destroyed"s << endl;
            }
        };

        // Сконструированный по умолчанию указатель ссылается на nullptr
        ScopedPtr<Object> empty_smart_ptr;
        // Перегруженный оператор приведения к типу bool вернёт false для пустого указателя
        assert(!empty_smart_ptr);

        ScopedPtr<Object> smart_ptr(new Object());
        // Перегруженный оператор bool вернёт true для указателя, ссылающегося на объект
        assert(smart_ptr);

        // Проверка оператора разыменования
        (*smart_ptr).DoSomething();
        // Проверка оператора доступа к членам класса
        smart_ptr->DoSomething();
    }
}

// Эта функция main тестирует шаблон класса Octopus
void TestOctopus1() {
    // Проверка конструирования осьминогов
    {
        // По умолчанию осьминог имеет 8 щупалец
        Octopus default_octopus;
        assert(default_octopus.GetTentacleCount() == 8);

        // Осьминог может иметь отличное от 8 количество щупалец
        Octopus quadropus(4);
        assert(quadropus.GetTentacleCount() == 4);

        // И даже вообще не иметь щупалец
        Octopus coloboque(0);
        assert(coloboque.GetTentacleCount() == 0);
    }

    // Осьминогу можно добавлять щупальца
    {
        Octopus octopus(1);
        Tentacle* t0 = &octopus.GetTentacle(0);
        Tentacle* t1 = &octopus.AddTentacle();
        assert(octopus.GetTentacleCount() == 2);
        Tentacle* t2 = &octopus.AddTentacle();
        assert(octopus.GetTentacleCount() == 3);

        // После добавления щупалец ранее созданные щупальца не меняют своих адресов
        assert(&octopus.GetTentacle(0) == t0);
        assert(&octopus.GetTentacle(1) == t1);
        assert(&octopus.GetTentacle(2) == t2);

        for (int i = 0; i < octopus.GetTentacleCount(); ++i) {
            assert(octopus.GetTentacle(i).GetId() == i + 1);
        }
    }

    // Осьминоги могут прицепляться к щупальцам друг друга
    {
        Octopus male(2);
        Octopus female(2);

        assert(male.GetTentacle(0).GetLinkedTentacle() == nullptr);

        male.GetTentacle(0).LinkTo(female.GetTentacle(1));
        assert(male.GetTentacle(0).GetLinkedTentacle() == &female.GetTentacle(1));

        male.GetTentacle(0).Unlink();
        assert(male.GetTentacle(0).GetLinkedTentacle() == nullptr);
    }

    // Копия осьминога имеет свою собственную копию щупалец, которые
    // копируют состояние щупалец оригинального осьминога
    {
        // Перебираем осьминогов с разным количеством щупалец
        for (int num_tentacles = 0; num_tentacles < 10; ++num_tentacles) {
            Octopus male(num_tentacles);
            Octopus female(num_tentacles);
            // Пусть они хватают друг друга за щупальца
            for (int i = 0; i < num_tentacles; ++i) {
                male.GetTentacle(i).LinkTo(female.GetTentacle(num_tentacles - 1 - i));
            }

            Octopus male_copy(male);
            // Проверяем состояние щупалец копии
            assert(male_copy.GetTentacleCount() == male.GetTentacleCount());
            for (int i = 0; i < male_copy.GetTentacleCount(); ++i) {
                // Каждое щупальце копии размещается по адресу, отличному от адреса оригинального щупальца
                assert(&male_copy.GetTentacle(i) != &male.GetTentacle(i));
                // Каждое щупальце копии прицепляется к тому же щупальцу, что и оригинальное
                assert(male_copy.GetTentacle(i).GetLinkedTentacle() == male.GetTentacle(i).GetLinkedTentacle());
            }
        }
        // Если вы видите эту надпись, то разрушение осьминогов, скорее всего,
        // прошло без неопределённого поведения
        cout << "Everything is OK"s << endl;
    }
}

// Эта функция main тестирует шаблон класса Octopus
void TestOctopus2() {
    // Проверка присваивания осьминогов
    {
        Octopus octopus1(3);

        // Настраиваем состояние исходного осьминога
        octopus1.GetTentacle(2).LinkTo(octopus1.GetTentacle(1));

        // До присваивания octopus2 имеет своё собственное состояние
        Octopus octopus2(10);

        octopus2 = octopus1;

        // После присваивания осьминогов щупальца копии имеют то же состояние,
        // что и щупальца присваиваемого объекта
        assert(octopus2.GetTentacleCount() == octopus1.GetTentacleCount());
        for (int i = 0; i < octopus2.GetTentacleCount(); ++i) {
            auto& tentacle1 = octopus1.GetTentacle(i);
            auto& tentacle2 = octopus2.GetTentacle(i);
            assert(&tentacle2 != &tentacle1);
            assert(tentacle2.GetId() == tentacle1.GetId());
            assert(tentacle2.GetLinkedTentacle() == tentacle1.GetLinkedTentacle());
        }
    }

    // Проверка самоприсваивания осьминогов
    {
        Octopus octopus(3);

        // Настраиваем состояние осьминога
        octopus.GetTentacle(0).LinkTo(octopus.GetTentacle(1));

        vector<pair<Tentacle*, Tentacle*>> tentacles;
        // Сохраняем информацию о щупальцах осьминога и его копии
        for (int i = 0; i < octopus.GetTentacleCount(); ++i) {
            tentacles.push_back({ &octopus.GetTentacle(i), octopus.GetTentacle(i).GetLinkedTentacle() });
        }

        // Выполняем самоприсваивание
        octopus = octopus;

        // После самоприсваивания состояние осьминога не должно измениться
        assert(octopus.GetTentacleCount() == static_cast<int>(tentacles.size()));
        for (int i = 0; i < octopus.GetTentacleCount(); ++i) {
            auto& tentacle_with_link = tentacles.at(i);
            assert(&octopus.GetTentacle(i) == tentacle_with_link.first);
            assert(octopus.GetTentacle(i).GetLinkedTentacle() == tentacle_with_link.second);
        }
    }
}

// Эта функция main тестирует шаблон класса PtrVector
void PtrVector1() {
    // Вспомогательный "шпион", позволяющий узнать о своём удалении
    struct DeletionSpy {
        explicit DeletionSpy(bool& is_deleted)
            : is_deleted_(is_deleted) {
        }
        ~DeletionSpy() {
            is_deleted_ = true;
        }
        bool& is_deleted_;
    };

    // Проверяем удаление элементов
    {
        bool spy1_is_deleted = false;
        DeletionSpy* ptr1 = new DeletionSpy(spy1_is_deleted);
        {
            PtrVector<DeletionSpy> ptr_vector;
            ptr_vector.GetItems().push_back(ptr1);
            assert(!spy1_is_deleted);

            // Константная ссылка на ptr_vector
            const auto& const_ptr_vector_ref(ptr_vector);
            // И константная, и неконстантная версия GetItems
            // должны вернуть ссылку на один и тот же вектор
            assert(&const_ptr_vector_ref.GetItems() == &ptr_vector.GetItems());
        }
        // При разрушении ptr_vector должен удалить все объекты, на которые
        // ссылаются находящиеся внутри него указателели
        assert(spy1_is_deleted);
    }

    // Вспомогательный «шпион», позволяющий узнать о своём копировании
    struct CopyingSpy {
        explicit CopyingSpy(int& copy_count)
            : copy_count_(copy_count) {
        }
        CopyingSpy(const CopyingSpy& rhs)
            : copy_count_(rhs.copy_count_)
        {
            ++copy_count_;
        }
        int& copy_count_;
    };

    // Проверяем копирование элементов при копировании массива указателей
    {
        // 10 элементов
        vector<int> copy_counters(10);

        PtrVector<CopyingSpy> ptr_vector;
        // Подготавливаем оригинальный массив указателей
        for (auto& counter : copy_counters) {
            ptr_vector.GetItems().push_back(new CopyingSpy(counter));
        }
        // Последний элемент содержит нулевой указатель
        ptr_vector.GetItems().push_back(nullptr);

        auto ptr_vector_copy(ptr_vector);
        // Количество элементов в копии равно количеству элементов оригинального вектора
        assert(ptr_vector_copy.GetItems().size() == ptr_vector.GetItems().size());

        // копия должна хранить указатели на новые объекты
        assert(ptr_vector_copy.GetItems() != ptr_vector.GetItems());
        // Последний элемент исходного массива и его копии - нулевой указатель
        assert(ptr_vector_copy.GetItems().back() == nullptr);
        // Проверяем, что элементы были скопированы (копирующие шпионы увеличивают счётчики копий).
        assert(all_of(copy_counters.begin(), copy_counters.end(), [](int counter) {
            return counter == 1;
            }));
    }
}

// Эта функция main тестирует шаблон класса PtvVector
void PtrVector2() {
    struct CopyingSpy {
        CopyingSpy(int& copy_count, int& deletion_count)
            : copy_count_(copy_count)
            , deletion_count_(deletion_count) {
        }
        CopyingSpy(const CopyingSpy& rhs)
            : copy_count_(rhs.copy_count_)          // счётчик копирований
            , deletion_count_(rhs.deletion_count_)  // счётчик удалений
        {
            if (rhs.throw_on_copy_) {
                throw runtime_error("copy construction failed"s);
            }
            ++copy_count_;
        }
        ~CopyingSpy() {
            ++deletion_count_;
        }
        void ThrowOnCopy() {
            throw_on_copy_ = true;
        }

    private:
        int& copy_count_;
        int& deletion_count_;
        bool throw_on_copy_ = false;
    };

    // Проверка присваивания
    {
        int item0_copy_count = 0;
        int item0_deletion_count = 0;
        {
            PtrVector<CopyingSpy> v;

            v.GetItems().push_back(new CopyingSpy(item0_copy_count, item0_deletion_count));
            v.GetItems().push_back(nullptr);
            {
                PtrVector<CopyingSpy> v_copy;
                v_copy = v;
                assert(v_copy.GetItems().size() == v.GetItems().size());
                assert(v_copy.GetItems().at(0) != v.GetItems().at(0));
                assert(v_copy.GetItems().at(1) == nullptr);
                assert(item0_copy_count == 1);
                assert(item0_deletion_count == 0);
            }
            assert(item0_deletion_count == 1);
        }
        assert(item0_deletion_count == 2);
    }

    // Проверка корректности самоприсваивания
    {
        int item0_copy_count = 0;
        int item0_deletion_count = 0;

        PtrVector<CopyingSpy> v;
        v.GetItems().push_back(new CopyingSpy(item0_copy_count, item0_deletion_count));
        CopyingSpy* first_item = v.GetItems().front();

        v = v;
        assert(v.GetItems().size() == 1);
        // При самоприсваивании объекты должны быть расположены по тем же адресам
        assert(v.GetItems().front() == first_item);
        assert(item0_copy_count == 0);
        assert(item0_deletion_count == 0);
    }

    // Проверка обеспечения строгой гарантии безопасности исключений при присваивании
    {
        int item0_copy_count = 0;
        int item0_deletion_count = 0;

        int item1_copy_count = 0;
        int item1_deletion_count = 0;

        // v хранит 2 элемента
        PtrVector<CopyingSpy> v;
        v.GetItems().push_back(new CopyingSpy(item0_copy_count, item0_deletion_count));
        v.GetItems().push_back(new CopyingSpy(item1_copy_count, item1_deletion_count));

        int other_item0_copy_count = 0;
        int other_item0_deletion_count = 0;
        // other_vector хранит 1 элемент, при копировании которого будет выброшено исключение
        PtrVector<CopyingSpy> other_vector;
        other_vector.GetItems().push_back(new CopyingSpy(other_item0_copy_count, other_item0_deletion_count));
        other_vector.GetItems().front()->ThrowOnCopy();

        // Сохраняем массив указателей
        auto v_items(v.GetItems());

        try {
            v = other_vector;
            // Операция должна выбросить исключение
            assert(false);
        }
        catch (const runtime_error&) {
        }
    }
}

// Эта функция тестирует работу SingleLinkedList, методы PushFront, Clear, GetSize, IsEmpty
void TestSLL1() {
    // Шпион, следящий за своим удалением
    struct DeletionSpy {
        DeletionSpy() = default;
        explicit DeletionSpy(int& instance_counter) noexcept
            : instance_counter_ptr_(&instance_counter)  //
        {
            OnAddInstance();
        }
        DeletionSpy(const DeletionSpy& other) noexcept
            : instance_counter_ptr_(other.instance_counter_ptr_)  //
        {
            OnAddInstance();
        }
        DeletionSpy& operator=(const DeletionSpy& rhs) noexcept {
            if (this != &rhs) {
                auto rhs_copy(rhs);
                std::swap(instance_counter_ptr_, rhs_copy.instance_counter_ptr_);
            }
            return *this;
        }
        ~DeletionSpy() {
            OnDeleteInstance();
        }

    private:
        void OnAddInstance() noexcept {
            if (instance_counter_ptr_) {
                ++(*instance_counter_ptr_);
            }
        }
        void OnDeleteInstance() noexcept {
            if (instance_counter_ptr_) {
                assert(*instance_counter_ptr_ != 0);
                --(*instance_counter_ptr_);
            }
        }

        int* instance_counter_ptr_ = nullptr;
    };

    // Проверка вставки в начало
    {
        SingleLinkedList<int> l;
        assert(l.IsEmpty());
        assert(l.GetSize() == 0u);

        l.PushFront(0);
        l.PushFront(1);
        assert(l.GetSize() == 2);
        assert(!l.IsEmpty());

        l.Clear();
        assert(l.GetSize() == 0);
        assert(l.IsEmpty());
    }

    // Проверка фактического удаления элементов
    {
        int item0_counter = 0;
        int item1_counter = 0;
        int item2_counter = 0;
        {
            SingleLinkedList<DeletionSpy> list;
            list.PushFront(DeletionSpy{ item0_counter });
            list.PushFront(DeletionSpy{ item1_counter });
            list.PushFront(DeletionSpy{ item2_counter });

            assert(item0_counter == 1);
            assert(item1_counter == 1);
            assert(item2_counter == 1);
            list.Clear();
            assert(item0_counter == 0);
            assert(item1_counter == 0);
            assert(item2_counter == 0);

            list.PushFront(DeletionSpy{ item0_counter });
            list.PushFront(DeletionSpy{ item1_counter });
            list.PushFront(DeletionSpy{ item2_counter });
            assert(item0_counter == 1);
            assert(item1_counter == 1);
            assert(item2_counter == 1);
        }
        assert(item0_counter == 0);
        assert(item1_counter == 0);
        assert(item2_counter == 0);
    }

    // Вспомогательный класс, бросающий исключение после создания N-копии
    struct ThrowOnCopy {
        ThrowOnCopy() = default;
        explicit ThrowOnCopy(int& copy_counter) noexcept
            : countdown_ptr(&copy_counter) {
        }
        ThrowOnCopy(const ThrowOnCopy& other)
            : countdown_ptr(other.countdown_ptr)  //
        {
            if (countdown_ptr) {
                if (*countdown_ptr == 0) {
                    throw std::bad_alloc();
                }
                else {
                    --(*countdown_ptr);
                }
            }
        }
        // Присваивание элементов этого типа не требуется
        ThrowOnCopy& operator=(const ThrowOnCopy& rhs) = delete;
        // Адрес счётчика обратного отсчёта. Если не равен nullptr, то уменьшается при каждом копировании.
        // Как только обнулится, конструктор копирования выбросит исключение
        int* countdown_ptr = nullptr;
    };

    {
        bool exception_was_thrown = false;
        // Последовательно уменьшаем счётчик копирований до нуля, пока не будет выброшено исключение
        for (int max_copy_counter = 5; max_copy_counter >= 0; --max_copy_counter) {
            // Создаём непустой список
            SingleLinkedList<ThrowOnCopy> list;
            list.PushFront(ThrowOnCopy{});
            try {
                int copy_counter = max_copy_counter;
                list.PushFront(ThrowOnCopy(copy_counter));
                // Если метод не выбросил исключение, список должен перейти в новое состояние
                assert(list.GetSize() == 2);
            }
            catch (const std::bad_alloc&) {
                exception_was_thrown = true;
                // После выбрасывания исключения состояние списка должно остаться прежним
                assert(list.GetSize() == 1);
                break;
            }
        }
        assert(exception_was_thrown);
    }
}

// Эта функция тестирует работу SingleLinkedList, класс Increment
void TestSLL2() {
    // Итерирование по пустому списку
    {
        SingleLinkedList<int> list;
        // Константная ссылка для доступа к константным версиям begin()/end()
        const auto& const_list = list;

        // Итераторы begin и end у пустого диапазона равны друг другу
        assert(list.begin() == list.end());
        assert(const_list.begin() == const_list.end());
        assert(list.cbegin() == list.cend());
        assert(list.cbegin() == const_list.begin());
        assert(list.cend() == const_list.end());
    }

    // Итерирование по непустому списку
    {
        SingleLinkedList<int> list;
        const auto& const_list = list;

        list.PushFront(1);
        assert(list.GetSize() == 1u);
        assert(!list.IsEmpty());

        assert(const_list.begin() != const_list.end());
        assert(const_list.cbegin() != const_list.cend());
        assert(list.begin() != list.end());

        assert(const_list.begin() == const_list.cbegin());

        assert(*list.cbegin() == 1);
        *list.begin() = -1;
        assert(*list.cbegin() == -1);

        const auto old_begin = list.cbegin();
        list.PushFront(2);
        assert(list.GetSize() == 2);

        const auto new_begin = list.cbegin();
        assert(new_begin != old_begin);
        // Проверка прединкремента
        {
            auto new_begin_copy(new_begin);
            assert((++(new_begin_copy)) == old_begin);
        }
        // Проверка постинкремента
        {
            auto new_begin_copy(new_begin);
            assert(((new_begin_copy)++) == new_begin);
            assert(new_begin_copy == old_begin);
        }
        // Итератор, указывающий на позицию после последнего элемента, равен итератору end()
        {
            auto old_begin_copy(old_begin);
            assert((++old_begin_copy) == list.end());
        }
    }
    // Преобразование итераторов
    {
        SingleLinkedList<int> list;
        list.PushFront(1);
        // Конструирование ConstIterator из Iterator
        SingleLinkedList<int>::ConstIterator const_it(list.begin());
        assert(const_it == list.cbegin());
        assert(*const_it == *list.cbegin());

        SingleLinkedList<int>::ConstIterator const_it1;
        // Присваивание ConstIterator'у значения Iterator
        const_it1 = list.begin();
        assert(const_it1 == const_it);
    }
    // Проверка оператора ->
    {
        using namespace std;
        SingleLinkedList<std::string> string_list;

        string_list.PushFront("one"s);
        assert(string_list.cbegin()->length() == 3u);
        string_list.begin()->push_back('!');
        assert(*string_list.begin() == "one!"s);
    }
}

// Эта функция проверяет работу класса SingleLinkedList, класс Iterator, перегрузка операторов
void TestSLL3() {
    // Проверка списков на равенство и неравенство
    {
        SingleLinkedList<int> list_1;
        list_1.PushFront(1);
        list_1.PushFront(2);

        SingleLinkedList<int> list_2;
        list_2.PushFront(1);
        list_2.PushFront(2);
        list_2.PushFront(3);

        SingleLinkedList<int> list_1_copy;
        list_1_copy.PushFront(1);
        list_1_copy.PushFront(2);

        SingleLinkedList<int> empty_list;
        SingleLinkedList<int> another_empty_list;

        // Список равен самому себе
        assert(list_1 == list_1);
        assert(empty_list == empty_list);

        // Списки с одинаковым содержимым равны, а с разным - не равны
        assert(list_1 == list_1_copy);
        assert(list_1 != list_2);
        assert(list_2 != list_1);
        assert(empty_list == another_empty_list);
    }

    // Обмен содержимого списков
    {
        SingleLinkedList<int> first;
        first.PushFront(1);
        first.PushFront(2);

        SingleLinkedList<int> second;
        second.PushFront(10);
        second.PushFront(11);
        second.PushFront(15);

        const auto old_first_begin = first.begin();
        const auto old_second_begin = second.begin();
        const auto old_first_size = first.GetSize();
        const auto old_second_size = second.GetSize();

        first.swap(second);

        assert(second.begin() == old_first_begin);
        assert(first.begin() == old_second_begin);
        assert(second.GetSize() == old_first_size);
        assert(first.GetSize() == old_second_size);

        // Обмен при помощи функции swap
        {
            using std::swap;

            // В отсутствие пользовательской перегрузки будет вызвана функция std::swap, которая
            // выполнит обмен через создание временной копии
            swap(first, second);

            // Убеждаемся, что используется не std::swap, а пользовательская перегрузка

            // Если бы обмен был выполнен с созданием временной копии,
            // то итератор first.begin() не будет равен ранее сохранённому значению,
            // так как копия будет хранить свои узлы по иным адресам
            assert(first.begin() == old_first_begin);
            assert(second.begin() == old_second_begin);
            assert(first.GetSize() == old_first_size);
            assert(second.GetSize() == old_second_size);
        }
    }

    // Инициализация списка при помощи std::initializer_list
    {
        SingleLinkedList<int> list{ 1, 2, 3, 4, 5 };
        assert(list.GetSize() == 5);
        assert(!list.IsEmpty());
        assert(std::equal(list.begin(), list.end(), std::begin({ 1, 2, 3, 4, 5 })));
    }

    // Лексикографическое сравнение списков
    {
        using IntList = SingleLinkedList<int>;

        assert((IntList{ 1, 2, 3 } < IntList{ 1, 2, 3, 1 }));
        assert((IntList{ 1, 2, 3 } <= IntList{ 1, 2, 3 }));
        assert((IntList{ 1, 2, 4 } > IntList{ 1, 2, 3 }));
        assert((IntList{ 1, 2, 3 } >= IntList{ 1, 2, 3 }));
    }

    // Копирование списков
    {
        const SingleLinkedList<int> empty_list{};
        // Копирование пустого списка
        {
            auto list_copy(empty_list);
            assert(list_copy.IsEmpty());
        }

        SingleLinkedList<int> non_empty_list{ 1, 2, 3, 4 };
        // Копирование непустого списка
        {
            auto list_copy(non_empty_list);

            assert(non_empty_list.begin() != list_copy.begin());
            assert(list_copy == non_empty_list);
        }
    }

    // Присваивание списков
    {
        const SingleLinkedList<int> source_list{ 1, 2, 3, 4 };

        SingleLinkedList<int> receiver{ 5, 4, 3, 2, 1 };
        receiver = source_list;
        assert(receiver.begin() != source_list.begin());
        assert(receiver == source_list);
    }

    // Вспомогательный класс, бросающий исключение после создания N-копии
    struct ThrowOnCopy {
        ThrowOnCopy() = default;
        explicit ThrowOnCopy(int& copy_counter) noexcept
            : countdown_ptr(&copy_counter) {
        }
        ThrowOnCopy(const ThrowOnCopy& other)
            : countdown_ptr(other.countdown_ptr)  //
        {
            if (countdown_ptr) {
                if (*countdown_ptr == 0) {
                    throw std::bad_alloc();
                }
                else {
                    --(*countdown_ptr);
                }
            }
        }
        // Присваивание элементов этого типа не требуется
        ThrowOnCopy& operator=(const ThrowOnCopy& rhs) = delete;
        // Адрес счётчика обратного отсчёта. Если не равен nullptr, то уменьшается при каждом копировании.
        // Как только обнулится, конструктор копирования выбросит исключение
        int* countdown_ptr = nullptr;
    };

    // Безопасное присваивание списков
    {
        SingleLinkedList<ThrowOnCopy> src_list;
        src_list.PushFront(ThrowOnCopy{});
        src_list.PushFront(ThrowOnCopy{});
        auto thrower = src_list.begin();
        src_list.PushFront(ThrowOnCopy{});

        int copy_counter = 0;  // при первом же копировании будет выброшего исключение
        thrower->countdown_ptr = &copy_counter;

        SingleLinkedList<ThrowOnCopy> dst_list;
        dst_list.PushFront(ThrowOnCopy{});
        int dst_counter = 10;
        dst_list.begin()->countdown_ptr = &dst_counter;
        dst_list.PushFront(ThrowOnCopy{});

        try {
            dst_list = src_list;
            // Ожидается исключение при присваивании
            assert(false);
        }
        catch (const std::bad_alloc&) {
            // Проверяем, что состояние списка-приёмника не изменилось
            // при выбрасывании исключений
            assert(dst_list.GetSize() == 2);
            auto it = dst_list.begin();
            assert(it != dst_list.end());
            assert(it->countdown_ptr == nullptr);
            ++it;
            assert(it != dst_list.end());
            assert(it->countdown_ptr == &dst_counter);
            assert(dst_counter == 10);
        }
        catch (...) {
            // Других типов исключений не ожидается
            assert(false);
        }
    }
}

// Эта функция проверяет работу класса SingleLinkedList, методы InsertAfter, EraseAfter
void TestSLL4() {
    struct DeletionSpy {
        ~DeletionSpy() {
            if (deletion_counter_ptr) {
                ++(*deletion_counter_ptr);
            }
        }
        int* deletion_counter_ptr = nullptr;
    };

    // Проверка PopFront
    {
        SingleLinkedList<int> numbers{ 3, 14, 15, 92, 6 };
        numbers.PopFront();
        assert((numbers == SingleLinkedList<int>{14, 15, 92, 6}));

        SingleLinkedList<DeletionSpy> list;
        list.PushFront(DeletionSpy{});
        int deletion_counter = 0;
        list.begin()->deletion_counter_ptr = &deletion_counter;
        assert(deletion_counter == 0);
        list.PopFront();
        assert(deletion_counter == 1);
    }

    // Доступ к позиции, предшествующей begin
    {
        SingleLinkedList<int> empty_list;
        const auto& const_empty_list = empty_list;
        assert(empty_list.before_begin() == empty_list.cbefore_begin());
        assert(++empty_list.before_begin() == empty_list.begin());
        assert(++empty_list.cbefore_begin() == const_empty_list.begin());

        SingleLinkedList<int> numbers{ 1, 2, 3, 4 };
        const auto& const_numbers = numbers;
        assert(numbers.before_begin() == numbers.cbefore_begin());
        assert(++numbers.before_begin() == numbers.begin());
        assert(++numbers.cbefore_begin() == const_numbers.begin());
    }

    // Вставка элемента после указанной позиции
    {  // Вставка в пустой список
        {
            SingleLinkedList<int> lst;
            const auto inserted_item_pos = lst.InsertAfter(lst.before_begin(), 123);
            assert((lst == SingleLinkedList<int>{123}));
            assert(inserted_item_pos == lst.begin());
            assert(*inserted_item_pos == 123);
        }

        // Вставка в непустой список
        {
            SingleLinkedList<int> lst{ 1, 2, 3 };
            auto inserted_item_pos = lst.InsertAfter(lst.before_begin(), 123);

            assert(inserted_item_pos == lst.begin());
            assert(inserted_item_pos != lst.end());
            assert(*inserted_item_pos == 123);
            assert((lst == SingleLinkedList<int>{123, 1, 2, 3}));

            inserted_item_pos = lst.InsertAfter(lst.begin(), 555);
            assert(++SingleLinkedList<int>::Iterator(lst.begin()) == inserted_item_pos);
            assert(*inserted_item_pos == 555);
            assert((lst == SingleLinkedList<int>{123, 555, 1, 2, 3}));
        };
    }

    // Вспомогательный класс, бросающий исключение после создания N-копии
    struct ThrowOnCopy {
        ThrowOnCopy() = default;
        explicit ThrowOnCopy(int& copy_counter) noexcept
            : countdown_ptr(&copy_counter) {
        }
        ThrowOnCopy(const ThrowOnCopy& other)
            : countdown_ptr(other.countdown_ptr)  //
        {
            if (countdown_ptr) {
                if (*countdown_ptr == 0) {
                    throw std::bad_alloc();
                }
                else {
                    --(*countdown_ptr);
                }
            }
        }
        // Присваивание элементов этого типа не требуется
        ThrowOnCopy& operator=(const ThrowOnCopy& rhs) = delete;
        // Адрес счётчика обратного отсчёта. Если не равен nullptr, то уменьшается при каждом копировании.
        // Как только обнулится, конструктор копирования выбросит исключение
        int* countdown_ptr = nullptr;
    };

    // Проверка обеспечения строгой гарантии безопасности исключений
    {
        bool exception_was_thrown = false;
        for (int max_copy_counter = 10; max_copy_counter >= 0; --max_copy_counter) {
            SingleLinkedList<ThrowOnCopy> list{ ThrowOnCopy{}, ThrowOnCopy{}, ThrowOnCopy{} };
            try {
                int copy_counter = max_copy_counter;
                list.InsertAfter(list.cbegin(), ThrowOnCopy(copy_counter));
                assert(list.GetSize() == 4u);
            }
            catch (const std::bad_alloc&) {
                exception_was_thrown = true;
                assert(list.GetSize() == 3u);
                break;
            }
        }
        assert(exception_was_thrown);
    }

    // Удаление элементов после указанной позиции
    {
        {
            SingleLinkedList<int> lst{ 1, 2, 3, 4 };
            const auto& const_lst = lst;
            const auto item_after_erased = lst.EraseAfter(const_lst.cbefore_begin());
            assert((lst == SingleLinkedList<int>{2, 3, 4}));
            assert(item_after_erased == lst.begin());
        }
        {
            SingleLinkedList<int> lst{ 1, 2, 3, 4 };
            const auto item_after_erased = lst.EraseAfter(lst.cbegin());
            assert((lst == SingleLinkedList<int>{1, 3, 4}));
            assert(item_after_erased == (++lst.begin()));
        }
        {
            SingleLinkedList<int> lst{ 1, 2, 3, 4 };
            const auto item_after_erased = lst.EraseAfter(++(++lst.cbegin()));
            assert((lst == SingleLinkedList<int>{1, 2, 3}));
            assert(item_after_erased == lst.end());
        }
        {
            SingleLinkedList<DeletionSpy> list{ DeletionSpy{}, DeletionSpy{}, DeletionSpy{} };
            auto after_begin = ++list.begin();
            int deletion_counter = 0;
            after_begin->deletion_counter_ptr = &deletion_counter;
            assert(deletion_counter == 0u);
            list.EraseAfter(list.cbegin());
            assert(deletion_counter == 1u);
        }
    }
}

// Эта функция main тестирует шаблон класса ArrayPtr
void TestArroy() {
    vector<int> v = { 1, 2 };
    ReverseArray(v.data(), v.size());
    assert(v == (vector<int>{2, 1}));

    ArrayPtr<int> numbers(10);
    const auto& const_numbers = numbers;

    numbers[2] = 42;
    assert(const_numbers[2] == 42);
    assert(&const_numbers[2] == &numbers[2]);

    assert(numbers.Get() == &numbers[0]);

    ArrayPtr<int> numbers_2(5);
    numbers_2[2] = 43;

    numbers.swap(numbers_2);

    assert(numbers_2[2] == 42);
    assert(numbers[2] == 43);
}

int main() {
    RUN_TEST(TestCats);
    RUN_TEST(TestScoped1);
    RUN_TEST(TestScoped2);
    RUN_TEST(TestOctopus1);
    RUN_TEST(TestOctopus2);
    RUN_TEST(PtrVector1);
    RUN_TEST(PtrVector2);
    RUN_TEST(TestSLL1);
    RUN_TEST(TestSLL2);
    RUN_TEST(TestSLL3);
    RUN_TEST(TestSLL4);
}