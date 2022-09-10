#include "iostream"
#include <cstdio>
#include <typeindex>
#include "vector"
#include "unordered_map"
#include "tuple"

using std::vector, std::unordered_map, std::type_index, std::tuple;

struct Component {
    virtual void update() = 0;
};

struct CustomComponent : Component {
    int x;

    void update() override {
        printf("custom %d\n", x);
    }
};

struct Arena {
    unordered_map<type_index, tuple<size_t, char *>> pull;
    size_t max_size = 100;

    template<typename T>
    T* new_component() {
        auto t = type_index(typeid(T));
        if (pull.contains(t)) {
            auto &[size, ptr] = pull.at(t);
            void * void_ptr = &ptr[size * sizeof(T)];
            auto component = new(void_ptr) T;//(T*)ptr[size * sizeof(T)];
            size++;
            return component;
        } else {
            pull[t] = {0, new char[sizeof(T) * max_size]};
            return new_component<T>();
        }
    }
};


struct Entity {
    vector<Component *> components;
};


int main() {
    Arena arena{};
    auto* c1 = arena.new_component<CustomComponent>();
    auto* c2 = arena.new_component<CustomComponent>();
    c1->x = 15;
    c2->x = 16;
    c1->update();
    c2->update();
    return 0;
}