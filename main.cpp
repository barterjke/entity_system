#include "iostream"
#include <cstdio>
#include <typeindex>
#include <algorithm>
#include "vector"
#include "unordered_map"
#include "optional"
#include "tuple"
#include "string"

using std::vector, std::unordered_map, std::type_index, std::tuple, std::string, std::optional;

struct Entity;

struct Component {
    Entity *parent = nullptr;

    virtual void update() = 0;

//    virtual ~Component() = 0;
};

struct CustomComponent1 : Component {
    int x = 0;                          // should be default initialized

    void update() override {
        printf("custom1 updated %d\n", x);
    }

    ~CustomComponent1() {
        printf("custom1 destroyed %d\n", x);
    }
};

struct CustomComponent2 : Component {
    float x = 0;

    void printf_self() const {
        printf("custom2 updated %f\n", x);
    }

    void update() override {
        printf_self();
    }

    ~CustomComponent2() {
        printf("custom2 destroyed %d\n", x);
    }
};


struct Arena {
    unordered_map<type_index, tuple<size_t, char *>> pull;
    const size_t MAX_SIZE = 100;

    template<typename T>
    T *new_instance() {
        // everything that got spawned by arena should be default initialized.
        // it's inconvenient, but it's a tradeoff with c++ language and reasonable safety
        // at least I'm not aware of workaround
        auto t = type_index(typeid(T));
        if (pull.contains(t)) {
            auto &[size, ptr] = pull.at(t);
            void *void_ptr = &ptr[size * sizeof(T)];
            auto instance = new(void_ptr) T;
            size++;
            return instance;
        } else {
            pull[t] = {0, new char[sizeof(T) * MAX_SIZE]};
            return new_instance<T>();
        }
    }

    template<typename T>
    optional<vector<T>> iterate() {
        auto t = type_index(typeid(T));
        if (pull.contains(t)) {
            vector<T *> result;
            auto &[size, ptr] = pull.at(t);
            for (auto i = 0; i < size; i++) {
                void *void_ptr = &ptr[i * sizeof(T)];
                auto instance_ptr = dynamic_cast<T *>(void_ptr);
                result.push_back(instance_ptr);
            }
            return result;
        }
        return {};
    }

    template<typename T>
    bool destroy_all_instances_of_type() {
        auto t = type_index(typeid(T));
        if (pull.contains(t)) {
            auto &[size, ptr] = pull.at(t);
            for (auto i = 0; i < size; i++) {
                void *void_ptr = &ptr[i * sizeof(T)];
                auto instance_ptr = (T *) (void_ptr);
                if (instance_ptr != nullptr) {
                    delete instance_ptr;
                }
                return true;
            }
        }
        return false;
    }

    template<typename T>
    bool destroy_instance(T *to_remove) {
        auto t = type_index(typeid(T));
        if (pull.contains(t)) {
            auto &[size, ptr] = pull.at(t);
            for (auto i = 0; i < size; i++) {
                void *void_ptr = &ptr[i * sizeof(T)];
                auto instance_ptr = (T *) void_ptr; //dynamic_cast<CustomComponent1 *>(void_ptr);
                if (instance_ptr != nullptr && instance_ptr == to_remove) {
                    delete instance_ptr;
                    return true;
                }
            }
        }
        return false;
    }


    ~Arena() {
        // note: it doesn't call destroy_all_instances_of_type and therefore no destructor is called
        // you can add it manually for every existing component, like:
        // destroy_all_instances_of_type<CustomComponent1>(); destroy_all_instances_of_type<CustomComponent2>(); etc...
        for (auto &[t, tuple]: pull) {
            auto &[_, ptr] = tuple;
            delete ptr;
        }
    }
} arena;


struct Entity {
    const string tag{};

    unordered_map<type_index, Component *> hot_components{};
    vector<tuple<type_index, Component *>> cold_components{};

    template<typename T>
    T *get_component() {
        auto t = type_index(typeid(T));
        if (hot_components.contains(t)) {
            return dynamic_cast<T *>(hot_components[t]);
        }
        for (auto &[comp_type, comp_ptr]: cold_components) {
            if (comp_type == t) {
                return dynamic_cast<T *>(comp_ptr);
            }
        }
        return nullptr;
    }

    template<typename T>
    T *add_component(bool hot) {
        auto comp_ptr = arena.new_instance<T>();
        auto t = type_index(typeid(T));
        if (hot) {
            hot_components[t] = comp_ptr;
        } else {
            cold_components.push_back({t, comp_ptr});
        }
        dynamic_cast<Component *>(comp_ptr)->parent = this;
        return comp_ptr;
    }

    template<typename T>
    bool remove_component() {
        auto t = type_index(typeid(T));
        if (hot_components.contains(t)) {
            hot_components.erase(t);
            return true;
        }
        size_t i = 0;
        for (auto &[comp_type, comp_ptr]: cold_components) {
            if (comp_type == t) {
                T *component_ptr = dynamic_cast<T *>(comp_ptr);
                cold_components[i] = {type_index(typeid(void *)), nullptr};
                arena.destroy_instance<T>(component_ptr);
                return true;
            }
            i++;
        }
        return false;
    }

    void cleanup() {
        auto void_ptr_ind = type_index(typeid(void *));
        std::copy_if(cold_components.begin(), cold_components.end(), std::back_inserter(cold_components),
                     [&void_ptr_ind](tuple<type_index, Component *> &tuple) {
                         return get<type_index>(tuple) != void_ptr_ind;
                     }
        );
    }
};

int main() {
    auto entity = arena.new_instance<Entity>();
    auto c1 = entity->add_component<CustomComponent1>(true);
    auto c2 = entity->add_component<CustomComponent2>(false);
    c1->x = 1;
    c1->update();
    c2->update();
    entity->remove_component<CustomComponent2>();
    entity->cleanup();
    arena.destroy_instance<CustomComponent1>(c1);
    arena.destroy_all_instances_of_type<CustomComponent2>();
    return 0;
}