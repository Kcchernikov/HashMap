#include<algorithm>
#include <cstddef>
#include<initializer_list>
#include<memory>
#include<list>

// A "bidirectional" iterator class for Hash Table formulaic of pair (key, value).
template<class KeyType, class ValueType>
class Literator: public std::iterator<std::forward_iterator_tag, std::pair<KeyType, ValueType>> {
  private:
    std::pair<KeyType, ValueType>* ptr;
    std::list<size_t>::iterator it;

  public:
    // Default constructor.
    Literator(): ptr(nullptr), it(nullptr) {}
    // Сonstructor by pointer to a pair and iterator. 
    Literator(std::pair<KeyType, ValueType>* _ptr, std::list<size_t>::iterator _it) {
        ptr = _ptr;
        it = _it;
    }
    // Operator ++. All the operators below work for O(1).
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
    //Operator *, returns a reference to the object.
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
// A const "bidirectional" iterator class for Hash Table formulaic of pair (key, value).
template<class KeyType, class ValueType>
class ConstLiterator: public std::iterator<std::forward_iterator_tag, std::pair<KeyType, ValueType>> {
  private:
    std::pair<KeyType, ValueType>* ptr;
    std::list<size_t>::const_iterator it;

  public:
    // Default constuctor.
    ConstLiterator(): ptr(nullptr), it(nullptr) {}
    // Сonstructor by pointer to a pair and iterator. 
    ConstLiterator(std::pair<KeyType, ValueType>* _ptr, const std::list<size_t>::const_iterator _it) {
        ptr = _ptr;
        it = _it;
    }
    // Operator ++. All the operators below work for O(1).
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
// Hash table with open addressing. Deleted items are marked dead and cleared when the table capacity is reallocated.
template<class KeyType, class ValueType, class Hash = std::hash<KeyType> >
class HashMap {
  private:
    size_t capacity, fullness, dead_elem;
    std::pair<const KeyType, ValueType> *data = nullptr;
    char *occupied = nullptr;
    std::vector<std::list<size_t>::iterator> iters;
    std::list<size_t> nodes;
    Hash hasher;
    
    typedef std::pair<const KeyType, ValueType> PtrClass;
    // Clean memory, call destructor for each element, clean nodes list/
    void deallocate() {
        for (size_t i = 0; i < capacity; ++i) {
            if (occupied[i] != 0) {
                (data + i) -> ~PtrClass();
            }
        }
        nodes.clear();
        operator delete (data);
    }
    // Realocate memory with new capacity/
    void change_capacity(size_t new_capacity) {
        std::list<std::pair<const KeyType, ValueType>> elements(begin(), end());
        deallocate();
        delete[](occupied);
        capacity = new_capacity;
        data = static_cast<std::pair<const KeyType, ValueType>*>(operator new(sizeof(std::pair<const KeyType, ValueType>) * capacity));
        occupied = new char[capacity];                                                         
        init_occupied();
        fullness = 0;
        dead_elem = 0;
        for (auto v : elements) {
            insert(v);
        }
    }
    // Initializes the oсcupied with zeros.
    void init_occupied() {
        iters.resize(capacity, nodes.end());            
        for (size_t i = 0; i < capacity; ++i) {
            occupied[i] = 0;
        }
    }
    
  public:
    typedef Literator<const KeyType, ValueType> iterator;
    typedef ConstLiterator<const KeyType, ValueType> const_iterator;    
    // Default constructor.
    HashMap(const Hash& _hasher = Hash()): capacity(8), fullness(0), dead_elem(0), hasher(_hasher) {
         data = static_cast<std::pair<const KeyType, ValueType>*>(operator new(sizeof(std::pair<const KeyType, ValueType>) * capacity));
         occupied = new char[capacity];         
         init_occupied();
    }
    // Standard insert with open addressing, scanning from memory to the right for dead and occupied cells. Increasing the size by 10 times when the table is overloaded.
    void insert(const std::pair<KeyType, ValueType>& elem) {
        size_t seed = hasher(elem.first);
        size_t id = seed % capacity;
        while (occupied[id] != 0 && (occupied[id] == 2 || !(data[id].first == elem.first))) {
            ++id;
            if (id == capacity) {
                id = 0;
            }                    
        }
        if (occupied[id] == 1 && data[id].first == elem.first) {
            return;
        }
        fullness++;
        new (data + id) std::pair<const KeyType, ValueType>(elem);
        occupied[id] = 1;
        nodes.push_back(id);
        iters[id] = --nodes.end();
        if ((fullness + dead_elem) * 10 > capacity) {
            change_capacity(std::max(static_cast<size_t>(8), fullness * 100));
        }
    }
    // Deleting an item by marking it dead.
    void erase(const KeyType& key) {
        size_t seed = hasher(key);
        size_t id = seed % capacity;
        while (occupied[id] != 0 && (occupied[id] == 2 || !(data[id].first == key))) {
            ++id;
            if (id == capacity) {
                id = 0;
            }                    
        }
        if (occupied[id] != 1 || !(data[id].first == key)) return;
        occupied[id] = 2;
        nodes.erase(iters[id]);
        fullness--;
        dead_elem++;
    }
    // Constructor from a pair of iterators.
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
        occupied = new char[capacity];
        init_occupied();                  
        auto beg2 = beg;
        while (beg2 != en) {
            insert((*beg2));
            ++beg2;
        }
    }
    // Constructor from a list of elements.
    HashMap(const std::initializer_list<std::pair<KeyType, ValueType>>& list, const Hash& _hasher = Hash()): fullness(0), dead_elem(0), hasher(_hasher) {
        capacity = std::max(static_cast<size_t>(8), list.size() * 8);
        data = static_cast<std::pair<const KeyType, ValueType>*>(operator new(sizeof(std::pair<const KeyType, ValueType>) * capacity));
        occupied = new char[capacity];
        init_occupied();                         
        auto it = list.begin();
        while (it != list.end()) {
            insert(*(it));
            ++it;
        }
    }
    // Copy Constructor.
    HashMap(const HashMap& a): fullness(0), dead_elem(0), hasher(a.hasher) {
        capacity = std::max(static_cast<size_t>(8), a.size() * 8);
        data = static_cast<std::pair<const KeyType, ValueType>*>(operator new(sizeof(std::pair<const KeyType, ValueType>) * capacity));
        occupied = new char[capacity];
        init_occupied();                         
        auto it = a.begin();
        while (it != a.end()) {
            insert(*(it));
            ++it;
        }
    }
    // Returns count of elements in map.
    size_t size() const {
        return fullness;
    }
    
    bool empty() const {
        if (fullness == 0) return true;
        return false;
    }
    // Returns the map hasher.
    Hash hash_function() const {
        return hasher;
    }   
    // All simple actions with iterators work in O (1). 
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
    // Returns an iterator on the found element found by linear scanning to the right.    
    const_iterator find(const KeyType& key) const {
        size_t seed = hasher(key);
        size_t id = seed % capacity;
        while (occupied[id] == 1 && !(data[id].first == key)) {
            ++id;
            if (id == capacity) {
                id = 0;
            }                    
        }
        const_iterator temp = end();
        if (occupied[id] != 1) return temp;
        const_iterator it(data, iters[id]);      
        return it;
    }
    
    iterator find(KeyType key) {
        size_t seed = hasher(key);
        size_t id = seed % capacity;
        while (occupied[id] == 1 && !(data[id].first == key)) {
            ++id;
            if (id == capacity) {
                id = 0;
            }                    
        }   
        iterator temp = end();
        if (occupied[id] != 1) return temp;
        iterator it(data, iters[id]);      
        return it;
    }
    // Operator [], amortized running for O (1).
    ValueType& operator[](const KeyType& key) {
        if (find(key) == end()) {
            insert({key, ValueType()});
        }
        size_t seed = hasher(key);
        size_t id = seed % capacity;
        while (occupied[id] != 0 && (occupied[id] == 2 || !(data[id].first == key))) {
            ++id;
            if (id == capacity) {
                id = 0;
            }                    
        }
        return data[id].second;
    }
    // The standard at method that can throw exceptions and amortized running for O (1).
    const ValueType& at(const KeyType& key) const {
        if (find(key) == end()) {
            throw std::out_of_range("out");                                                                                                                
        }
        size_t seed = hasher(key);
        size_t id = seed % capacity;
        while (occupied[id] != 0 && (occupied[id] == 2 || !(data[id].first == key))) {
            ++id;
            if (id == capacity) {
                id = 0;
            }                    
        }
        return data[id].second;
    }
    // Clears memory, call all destructors.
    void clear() {
        for (size_t id : nodes) {
            occupied[id] = 2;
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
    //Destructor.
    ~HashMap() {
        deallocate();
        if (occupied != nullptr) {
            delete[](occupied);
        }
    }
};
