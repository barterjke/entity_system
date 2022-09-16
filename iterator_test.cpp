#include <iostream>

struct Integers
{
    int * m_data;

//    [[nodiscard]] auto begin() const { return Iterator{&m_data[0]}; }
//    [[nodiscard]] auto end() const   { return Iterator{&m_data[3]}; } // 200 is out of bounds
};

template<typename T>
struct Iterator
{
    explicit Iterator(int * ptr) : m_ptr(ptr) {std::cout << typeid(T).name();}

    int& operator*() const { return *m_ptr; }
    int* operator->() { return m_ptr; }

    // Prefix increment
    Iterator& operator++() { m_ptr++; return *this; }

    // Postfix increment
    Iterator operator++(int)  { Iterator tmp = *this; ++(*this); return tmp; }

    friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_ptr == b.m_ptr; };

private:

    int * m_ptr;
};

[[nodiscard]] auto begin(const Integers& integers) { return Iterator<int>{&integers.m_data[0]}; }
[[nodiscard]] auto end(const Integers& integers)    { return Iterator<int>{&integers.m_data[3]}; } // 200 is out of bounds

int main(){
    int data[] = {3,5,10};
    Integers integers{data};
    for (bool i : integers)
        std::cout << i << "\n";
}