# Entity component system with arena allocator
Simple Entity component system written in modern C++.
#### Reasoning: why not just use default c++ pointers?
Imagine we have this:
```c++
struct CustomComponent1 : Component {
    //
};
struct CustomComponent2 : Component {
    //
};

struct Entity{
    vector<Component*> components;
    void add_component(Component * component){
        components.push_back(component);
    }
};

int main(){
    Entity entity;
    entity.add_component(new CustomComponent1);
    entity.add_component(new CustomComponent2);
}
```
Problem with this we can have so many components, all different size, and straight up allocation is bad in many way, because you end up jumping around in memory.
![img.png](https://www.david-colson.com/assets/images/ecsArticle/Figure1.png)

What we are trying to achieve is something like this:
![img_1.png](https://www.david-colson.com/assets/images/ecsArticle/Figure2.png)

##### Implementation:
```c++
struct Arena {
    unordered_map<type_index, tuple<size_t, char *>> pull;
    const size_t MAX_SIZE = 100;
    template<typename T>
    T *new_instance();  
    // everything that got spawned by arena should be default initialized.
    // it's inconvenient, but it's a tradeoff with c++ language and reasonable safety
    // at least I'm not aware of workaround
    
    template<typename T>
    optional<vector<T>> iterate();
    
    template<typename T>
    bool destroy_all_instances_of_type(); // returns false if there is no such type 
    
    ~Arena();
    // note: it doesn't call destroy_all_instances_of_type and therefore no destructor is called
    // you can add it manually for every existing component, like:
    // destroy_all_instances_of_type<CustomComponent1>(); destroy_all_instances_of_type<CustomComponent2>(); etc...
} arena;
```

```c++
struct Entity {
    const string tag{};

    unordered_map<type_index, Component *> hot_components{}; // for frequently accessed components. It's long to add components and iterate over them, but fast to seek
    vector<tuple<type_index, Component *>> cold_components{};

    template<typename T>
    T *get_component();

    template<typename T>
    T *add_component(bool hot);

    template<typename T>
    bool remove_component(); // returns false if there is no such component 

    void cleanup(); // filter out empty cold components
};
```
##### Example usage:
```c++
int main() {
    auto entity = arena.new_instance<Entity>();
    auto c1 = entity->add_component<CustomComponent1>(true);
    auto c2 = entity->add_component<CustomComponent2>(false);
    c1->x = 1;
    c1->update();
    c2->update();
    assert(entity->remove_component<CustomComponent2>());
    assert(!entity->remove_component<CustomComponent2>()); // check double deletion
    entity->cleanup();
    assert(arena.destroy_all_instances_of_type<Entity>());
    assert(arena.destroy_all_instances_of_type<CustomComponent1>());
    assert(arena.destroy_all_instances_of_type<CustomComponent2>());
    return 0;
}
```
