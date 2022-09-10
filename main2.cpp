#include <cstdio>
#include <new>

struct Base{
    virtual void base_method() = 0;
};

struct Derived : Base{
    int x;
    void own_method(){
        printf("own %d", x);
    }
    void base_method() override{
        printf("base %d", x);
    }
};

int main() {
    auto * char_ptr = new char[500];
    void * void_ptr = char_ptr;
    Derived * derived_ptr = new(void_ptr) Derived;
    derived_ptr->x = 15;
    derived_ptr->own_method();
    derived_ptr->base_method(); //SEGMENTATION ERROR IF UNCOMMENT
    return 0;
}