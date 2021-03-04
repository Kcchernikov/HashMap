#include<algorithm>
#include <cstddef>
#include<iostream>
#include<initializer_list>
#include<memory>
#include<list>

template<class KeyType, class ValueType>
class Literator: public std::iterator<std::forward_iterator_tag, std::pair<KeyType, ValueType>> {
    std::pair<KeyType, ValueType>* ptr;
    std::list<size_t>::iterator it;

public:
    Literator(): ptr(nullptr), it(nullptr) {}

    Literator(std::pair<KeyType, ValueType>* _ptr, std::list<size_t>::iterator _it) {
        ptr = _ptr;
        it = _it;
    }

    Literator& operator++() {
        ++it;
        return *this;
    }
    
    Literator operator ++(int) {
        Literator temp = *this;
        ++*this;
        return temp;
    }

    Literator& operator--() {
        --it;
        return *this;
    }
    
    Literator operator --(int) {
        Literator temp = *this;
        --*this;
        return temp;
    }

    std::pair<KeyType, ValueType>& operator*() {
        return *(ptr + (*it));
    }
    
    const std::pair<KeyType, ValueType>& operator*() const {
        return *(ptr + (*it));
    }
    
    std::pair<KeyType, ValueType>* operator->() {
        return (ptr + (*it));
    }
    
    const std::pair<KeyType, ValueType>* operator->() const {
        return (ptr + (*it));
    }

    bool operator==(const Literator& other) const {
        return it == other.it;
    }
    
    bool operator!=(const Literator& other) const {
        return it != other.it;        
    }
};

template<class KeyType, class ValueType>
class ConstLiterator: public std::iterator<std::forward_iterator_tag, std::pair<KeyType, ValueType>> {
    std::pair<KeyType, ValueType>* ptr;
    std::list<size_t>::const_iterator it;

public:
    ConstLiterator(): ptr(nullptr), it(nullptr) {}

    ConstLiterator(std::pair<KeyType, ValueType>* _ptr, const std::list<size_t>::const_iterator _it) {
        ptr = _ptr;
        it = _it;
    }

    ConstLiterator& operator++() {
        ++it;
        return *this;
    }
    
    ConstLiterator operator ++(int) {
        ConstLiterator temp = *this;
        ++*this;
        return temp;
    }

    ConstLiterator& operator--() {
        --it;
        return *this;
    }
    
    ConstLiterator operator --(int) {
        ConstLiterator temp = *this;
        --*this;
        return temp;
    }

    std::pair<KeyType, ValueType>& operator*() const {
        return *(ptr + (*it));
    }
    
    const std::pair<KeyType, ValueType>* operator->() const {
        return (ptr + (*it));
    }

    bool operator==(const ConstLiterator& other) const {
        return it == other.it;
    }

    bool operator!=(const ConstLiterator& other) const {
        return it != other.it;
    }
};

template<class KeyType, class ValueType, class Hash = std::hash<KeyType> >
class HashMap {
    size_t capacity, fullness, dead_elem;
    std::pair<const KeyType, ValueType> *data = nullptr;
    char *occup = nullptr;
    std::vector<std::list<size_t>::iterator> iters;
    std::list<size_t> nodes;
    Hash hasher;
    
    typedef std::pair<const KeyType, ValueType> PtrClass;
    
    void deallocate() {
        for (size_t i = 0; i < capacity; ++i) {
            if (occup[i] != 0) {
                (data + i) -> ~PtrClass();
            }
        }
        nodes.clear();
        operator delete (data);
    }
    
    void change_capacity(size_t new_capacity) {
        std::list<std::pair<const KeyType, ValueType>> elements(begin(), end());
        deallocate();
        delete[](occup);
        capacity = new_capacity;
        data = static_cast<std::pair<const KeyType, ValueType>*>(operator new(sizeof(std::pair<const KeyType, ValueType>) * capacity));
        occup = new char[capacity];                                                         
        init_occup();
        fullness = 0;
        dead_elem = 0;
        for (auto v : elements) {
            insert(v);
        }
    }
    
    void init_occup() {
        iters.resize(capacity, nodes.end());            
        for (size_t i = 0; i < capacity; ++i) {
            occup[i] = 0;
        }
    }
    
public:
    typedef Literator<const KeyType, ValueType> iterator;
    typedef ConstLiterator<const KeyType, ValueType> const_iterator;    

    HashMap(const Hash& _hasher = Hash()): capacity(8), fullness(0), dead_elem(0), hasher(_hasher) {
         data = static_cast<std::pair<const KeyType, ValueType>*>(operator new(sizeof(std::pair<const KeyType, ValueType>) * capacity));
         occup = new char[capacity];         
         init_occup();
    }
    
    void insert(const std::pair<KeyType, ValueType>& elem) {
        size_t seed = hasher(elem.first);
        size_t id = seed % capacity;
        while (occup[id] != 0 && (occup[id] == 2 || !(data[id].first == elem.first))) {
            ++id;
            if (id == capacity) {
                id = 0;
            }                    
        }
        if (occup[id] == 1 && data[id].first == elem.first) {
            return;
        }
        fullness++;
        new (data + id) std::pair<const KeyType, ValueType>(elem);
        occup[id] = 1;
        nodes.push_back(id);
        iters[id] = --nodes.end();
        if ((fullness + dead_elem) * 10 > capacity) {
            change_capacity(std::max(static_cast<size_t>(8), fullness * 100));
        }
    }
    
    void erase(const KeyType& key) {
        size_t seed = hasher(key);
        size_t id = seed % capacity;
        while (occup[id] != 0 && (occup[id] == 2 || !(data[id].first == key))) {
            ++id;
            if (id == capacity) {
                id = 0;
            }                    
        }
        if (occup[id] != 1 || !(data[id].first == key)) return;
        occup[id] = 2;
        nodes.erase(iters[id]);
        fullness--;
        dead_elem++;
    }
    
    template<typename Iterator>
    HashMap(const Iterator& beg, const Iterator& en, const Hash& _hasher = Hash()): fullness(0), dead_elem(0), hasher(_hasher) {
        Iterator begin1 = beg;
        size_t sz = 0;
        while (begin1 != en) {
            ++begin1;
            ++sz;
        }
        capacity = std::max(static_cast<size_t>(8), 8*sz);
        data = static_cast<std::pair<const KeyType, ValueType>*>(operator new(sizeof(std::pair<const KeyType, ValueType>) * capacity));
        occup = new char[capacity];
        init_occup();                  
        auto beg2 = beg;
        while (beg2 != en) {
            insert((*beg2));
            ++beg2;
        }
    }
    
    HashMap(const std::initializer_list<std::pair<KeyType, ValueType>>& list, const Hash& _hasher = Hash()): fullness(0), dead_elem(0), hasher(_hasher) {
        capacity = std::max(static_cast<size_t>(8), list.size() * 8);
        data = static_cast<std::pair<const KeyType, ValueType>*>(operator new(sizeof(std::pair<const KeyType, ValueType>) * capacity));
        occup = new char[capacity];
        init_occup();                         
        auto it = list.begin();
        while (it != list.end()) {
            insert(*(it));
            ++it;
        }
    }
    
    HashMap(const HashMap& a): fullness(0), dead_elem(0), hasher(a.hasher) {
        capacity = std::max(static_cast<size_t>(8), a.size() * 8);
        data = static_cast<std::pair<const KeyType, ValueType>*>(operator new(sizeof(std::pair<const KeyType, ValueType>) * capacity));
        occup = new char[capacity];
        init_occup();                         
        auto it = a.begin();
        while (it != a.end()) {
            insert(*(it));
            ++it;
        }
    }
    
    size_t size() const {
        return fullness;
    }
    
    bool empty() const {
        if (fullness == 0) return true;
        return false;
    }
    
    Hash hash_function() const {
        return hasher;
    }   
    
    iterator begin() {
        iterator it(data, nodes.begin());
        return it;
    }
        
    const_iterator begin() const {
        const_iterator it(data, nodes.begin());
        return it;
    }
    
    iterator end() {
        iterator it(data, nodes.end());
        return it;
    }
    
    const_iterator end() const {
        const_iterator it(data, nodes.end());        
        return it;
    }
    
    const_iterator find(const KeyType& key) const {
        size_t seed = hasher(key);
        size_t id = seed % capacity;
        while (occup[id] == 1 && !(data[id].first == key)) {
            ++id;
            if (id == capacity) {
                id = 0;
            }                    
        }
        const_iterator temp = end();
        if (occup[id] != 1) return temp;
        const_iterator it(data, iters[id]);      
        return it;
    }
    
    iterator find(KeyType key) {
        size_t seed = hasher(key);
        size_t id = seed % capacity;
        while (occup[id] == 1 && !(data[id].first == key)) {
            ++id;
            if (id == capacity) {
                id = 0;
            }                    
        }   
        iterator temp = end();
        if (occup[id] != 1) return temp;
        iterator it(data, iters[id]);      
        return it;
    }
    
    ValueType& operator[](const KeyType& key) {
        if (find(key) == end()) {
            insert({key, ValueType()});
        }
        size_t seed = hasher(key);
        size_t id = seed % capacity;
        while (occup[id] != 0 && (occup[id] == 2 || !(data[id].first == key))) {
            ++id;
            if (id == capacity) {
                id = 0;
            }                    
        }
        return data[id].second;
    }
    
    const ValueType& at(const KeyType& key) const {
        if (find(key) == end()) {
            throw std::out_of_range("out");                                                                                                                
        }
        size_t seed = hasher(key);
        size_t id = seed % capacity;
        while (occup[id] != 0 && (occup[id] == 2 || !(data[id].first == key))) {
            ++id;
            if (id == capacity) {
                id = 0;
            }                    
        }
        return data[id].second;
    }
    
    void clear() {
        for (size_t id : nodes) {
            occup[id] = 2;
            iters[id] = nodes.end();
            dead_elem++;
        }
        fullness = 0;
        nodes.clear();
        change_capacity(8);
    }
    
    HashMap& operator = (const HashMap& a) {
        std::list<std::pair<const KeyType, ValueType>> tmp;
        hasher = a.hasher;
        nodes = a.nodes;
        for (auto it = a.begin(); it != a.end(); ++it) {
            tmp.push_back(*it);      
        }
        clear();
        for (auto it = tmp.begin(); it != tmp.end(); ++it) {
            insert(*it);
        }
        return *this;
    }
    
    ~HashMap() {
        deallocate();
        if (occup != nullptr) {
            delete[](occup);
        }
    }
};
