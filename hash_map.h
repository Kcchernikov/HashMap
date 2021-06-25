#include<algorithm>
#include<cstddef>
#include<initializer_list>
#include<memory>
#include<list>
// This is a hash table class with open addressing based on linear probing.
// Deleted items are marked dead and cleared when the table capacity is reallocated.
// Mathematical expectation of running time is O(1). Hash table is implemented scaling up (https://en.wikipedia.org/wiki/Open_addressing).

// A "forward" iterator (Fiterator) class for Hash Table formulaic of pair (key, value).
template<class KeyType, class ValueType>
class Fiterator: public std::iterator<std::forward_iterator_tag, std::pair<KeyType, ValueType>> {
  public:
    // Default constructor.
    Fiterator(): ptr(nullptr), char_ptr(0) {}
    // Constructor by pointer to a pair, pointer to char, capacity and number_in_table.
    Fiterator(std::pair<KeyType, ValueType>* _ptr, char* _char_ptr, size_t cap, size_t _number_in_table) {
        ptr = _ptr;
        char_ptr = _char_ptr;
        capacity = cap;
        number_in_table = _number_in_table;
    }
    // Operator ++, amortized time is O(1).
    Fiterator& operator++() {
        if (number_in_table != capacity) {
            ++ptr;
            ++char_ptr;
            ++number_in_table;
            // We are looking for a table cell occupied by an element.
            while (number_in_table < capacity && (*char_ptr) != 1) {
                ++ptr;
                ++char_ptr;
                ++number_in_table;
            }
        }
        return *this;
    }

    Fiterator operator++(int) {
        Fiterator temp = *this;
        ++*this;
        return temp;
    }
    // Operator --, amortized time is O(1).
    Fiterator& operator--() {
        if (number_in_table != 0) {
            --ptr;
            --char_ptr;
            --number_in_table;
            // We are looking for a table cell occupied by an element.
            while (number_in_table > 0 && (*char_ptr) != 1) {
                --ptr;
                --char_ptr;
                --number_in_table;
            }
        }
        return *this;
    }

    Fiterator operator--(int) {
        Fiterator temp = *this;
        --*this;
        return temp;
    }
    // Operator *, returns a reference to the object.
    std::pair<KeyType, ValueType>& operator*() {
        return *(ptr);
    }

    const std::pair<KeyType, ValueType>& operator*() const {
        return *(ptr);
    }

    std::pair<KeyType, ValueType>* operator->() {
        return (ptr);
    }

    const std::pair<KeyType, ValueType>* operator->() const {
        return (ptr);
    }

    bool operator == (const Fiterator& other) const {
        return number_in_table == other.number_in_table;
    }

    bool operator != (const Fiterator& other) const {
        return !(number_in_table == other.number_in_table);
    }
  private:
    std::pair<KeyType, ValueType>* ptr;
    char* char_ptr;
    size_t capacity = 0, number_in_table = 0;
};
// A const "forward" iterator (Fiterator) class for Hash Table formulaic of pair (key, value).
template<class KeyType, class ValueType>
class ConstFiterator: public std::iterator<std::forward_iterator_tag, std::pair<KeyType, ValueType>> {
  public:
    // Default constructor.
    ConstFiterator(): ptr(nullptr), char_ptr(0) {}
    // Constructor by pointer to a pair, pointer to char, capacity and number_in_table.
    ConstFiterator(std::pair<KeyType, ValueType>* _ptr, char* _char_ptr, size_t cap, size_t _number_in_table) {
        ptr = _ptr;
        char_ptr = _char_ptr;
        capacity = cap;
        number_in_table = _number_in_table;
    }
    // Operator ++, amortized time is O(1).
    ConstFiterator& operator++() {
        if (number_in_table != capacity) {
            ++ptr;
            ++char_ptr;
            ++number_in_table;
            // We are looking for a table cell occupied by an element.
            while (number_in_table < capacity && (*char_ptr) != 1) {
                ++ptr;
                ++char_ptr;
                ++number_in_table;
            }
        }
        return *this;
    }

    ConstFiterator operator++(int) {
        ConstFiterator temp = *this;
        ++*this;
        return temp;
    }
    // Operator --, amortized time is O(1).
    ConstFiterator& operator--() {
        if (number_in_table != 0) {
            --ptr;
            --char_ptr;
            --number_in_table;
            // We are looking for a table cell occupied by an element.
            while (number_in_table > 0 && (*char_ptr) != 1) {
                --ptr;
                --char_ptr;
                --number_in_table;
            }
        }
        return *this;
    }

    ConstFiterator operator--(int) {
        ConstFiterator temp = *this;
        --*this;
        return temp;
    }

    const std::pair<KeyType, ValueType>& operator*() const {
        return *(ptr);
    }
    // Operator *, returns a reference to the object.
    const std::pair<KeyType, ValueType>* operator->() const {
        return (ptr);
    }

    bool operator == (const ConstFiterator& other) const {
  //      if (ptr == nullptr && other.ptr == nullptr) return true;
        return number_in_table == other.number_in_table;
    }

    bool operator != (const ConstFiterator& other) const {
        return !(number_in_table == other.number_in_table);
    }
  private:
    std::pair<KeyType, ValueType>* ptr;
    char* char_ptr;
    size_t capacity = 0, number_in_table = 0;
};

// Hash table with open addressing based on linear probing.
// Deleted items are marked dead and cleared when the table capacity is reallocated.
// Mathematical expectation of running time is O(1). Hash table is implemented scaling up (https://en.wikipedia.org/wiki/Open_addressing).

template<class KeyType, class ValueType, class Hash = std::hash<KeyType> >
class HashMap {
  public:
    constexpr static int SIZELIMIT = 4, INCREASE = 8, MINSIZE = 8, ALIVE = 1, DEAD = 2, EMPTY = 0;
    typedef Fiterator<const KeyType, ValueType> iterator;
    typedef ConstFiterator<const KeyType, ValueType> const_iterator;
    // Default constructor.
    HashMap(const Hash& _hasher_ = Hash()): capacity_(MINSIZE), fullness_(0), dead_elem_(0), hasher_(_hasher_) {
         data_ = static_cast<std::pair<const KeyType, ValueType>*>(operator new(sizeof(std::pair<const KeyType, ValueType>) * capacity_));
         occupied_ = new char[capacity_];
         init_occupied_();
    }
    // Standard insert with open addressing, scanning from memory to the right for dead and occupied cells.
    // Increasing the size by 10 times when the table is overloaded.
    void insert(const std::pair<KeyType, ValueType>& elem) {
        size_t id = find_position_(elem.first);
        if (occupied_[id] == ALIVE && data_[id].first == elem.first) {
            return;
        }
        fullness_++;
        new (data_ + id) std::pair<const KeyType, ValueType>(elem);
        occupied_[id] = ALIVE;
        adapt_size_();
    }
    // Deleting an item by marking it dead.
    void erase(const KeyType& key) {
        size_t id = find_position_(key);
        if (occupied_[id] != ALIVE || !(data_[id].first == key)) return;
        occupied_[id] = DEAD;
        fullness_--;
        dead_elem_++;
    }
    // Constructor from a pair of iterators.
    template<typename Iterator>
    HashMap(const Iterator& start, const Iterator& finish, const Hash& _hasher_ = Hash()): fullness_(0), dead_elem_(0), hasher_(_hasher_) {
        Iterator it = start;
        size_t sz = 0;
        while (it != finish) {
            ++it;
            ++sz;
        }
        capacity_ = std::max(static_cast<size_t>(MINSIZE), INCREASE*sz);
        data_ = static_cast<std::pair<const KeyType, ValueType>*>(operator new(sizeof(std::pair<const KeyType, ValueType>) * capacity_));
        occupied_ = new char[capacity_];
        init_occupied_();
        it = start;
        while (it != finish) {
            insert((*it));
            ++it;
        }
    }
    // Constructor from a list of elements.
    HashMap(const std::initializer_list<std::pair<KeyType, ValueType>>& list, const Hash& _hasher_ = Hash()): fullness_(0), dead_elem_(0), hasher_(_hasher_) {
        capacity_ = std::max(static_cast<size_t>(MINSIZE), list.size() * INCREASE);
        data_ = static_cast<std::pair<const KeyType, ValueType>*>(operator new(sizeof(std::pair<const KeyType, ValueType>) * capacity_));
        occupied_ = new char[capacity_];
        init_occupied_();
        auto it = list.begin();
        while (it != list.end()) {
            insert(*(it));
            ++it;
        }
    }
    // Copy Constructor.
    HashMap(const HashMap& a): fullness_(0), dead_elem_(0), hasher_(a.hasher_) {
        capacity_ = std::max(static_cast<size_t>(MINSIZE), a.size() * INCREASE);
        data_ = static_cast<std::pair<const KeyType, ValueType>*>(operator new(sizeof(std::pair<const KeyType, ValueType>) * capacity_));
        occupied_ = new char[capacity_];
        init_occupied_();
        auto it = a.begin();
        while (it != a.end()) {
            insert(*(it));
            ++it;
        }
    }
    // Returns count of elements in map.
    size_t size() const {
        return fullness_;
    }

    bool empty() const {
        if (fullness_ == 0) return true;
        return false;
    }
    // Returns the map hasher.
    Hash hash_function() const {
        return hasher_;
    }
    // All simple actions with iterators work in O (1) amortized.
    iterator begin() {
        size_t start = capacity_;
        size_t id = 0;
        for (; id < capacity_; ++id) {
            if (occupied_[id] == ALIVE) {
                start = id;
                break;
            }
        }
        iterator it(data_ + start, occupied_ + start, capacity_, start);
        return it;
    }

    const_iterator begin() const {
        size_t start = capacity_;
        size_t id = 0;
        for (; id < capacity_; ++id) {
            if (occupied_[id] == ALIVE) {
                start = id;
                break;
            }
        }
        const_iterator it(data_ + start, occupied_ + start, capacity_, start);
        return it;
    }

    iterator end() {
        iterator it(data_ + capacity_, occupied_ + capacity_, capacity_, capacity_);
        return it;
    }

    const_iterator end() const {
        const_iterator it(data_ + capacity_, occupied_ + capacity_, capacity_, capacity_);
        return it;
    }
    // Returns an iterator on the found element found by linear scanning to the right.
    const_iterator find(const KeyType& key) const {
        size_t seed = hasher_(key);
        size_t id = seed % capacity_;
        while (occupied_[id] == ALIVE && !(data_[id].first == key)) {
            ++id;
            if (id == capacity_) {
                id = 0;
            }
        }
        const_iterator temp = end();
        if (occupied_[id] != 1) return temp;
        const_iterator it(data_ + id, occupied_ + id, capacity_, id);
        return it;
    }

    iterator find(KeyType key) {
        size_t seed = hasher_(key);
        size_t id = seed % capacity_;
        while (occupied_[id] == ALIVE && !(data_[id].first == key)) {
            ++id;
            if (id == capacity_) {
                id = 0;
            }
        }
        iterator temp = end();
        if (occupied_[id] != 1) return temp;
        iterator it(data_ + id, occupied_ + id, capacity_, id);
        return it;
    }
    // Operator [], amortized running for O (1).
    ValueType& operator[](const KeyType& key) {
        if (find(key) == end()) {
            insert({key, ValueType()});
        }
        size_t id = find_position_(key);
        return data_[id].second;
    }
    // The standard at method that can throw exceptions and amortized running for O (1).
    const ValueType& at(const KeyType& key) const {
        if (find(key) == end()) {
            throw std::out_of_range("out");
        }
        size_t id = find_position_(key);
        return data_[id].second;
    }
    // Clears memory, call all destructors.
    void clear() {
        for (size_t id = 0; id < capacity_; ++id) {
            occupied_[id] = DEAD;
            dead_elem_++;
        }
        fullness_ = 0;
        change_capacity_(MINSIZE);
    }

    HashMap& operator = (const HashMap& a) {
        std::list<std::pair<const KeyType, ValueType>> tmp;
        hasher_ = a.hasher_;
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
        deallocate_();
        if (occupied_ != nullptr) {
            delete[](occupied_);
        }
    }

   private:
    size_t capacity_, fullness_, dead_elem_;
    std::pair<const KeyType, ValueType> *data_ = nullptr;
    char *occupied_ = nullptr;
    Hash hasher_;

    typedef std::pair<const KeyType, ValueType> PtrClass;
    // Clean memory, call destructor for each element, clean nodes list.
    void deallocate_() {
        for (size_t i = 0; i < capacity_; ++i) {
            if (occupied_[i] != 0) {
                (data_ + i) -> ~PtrClass();
            }
        }
        operator delete (data_);
    }
    // Realocate memory with new capacity.
    void change_capacity_(size_t new_capacity_) {
        std::list<std::pair<const KeyType, ValueType>> elements(begin(), end());
        deallocate_();
        delete[](occupied_);
        capacity_ = new_capacity_;
        data_ = static_cast<std::pair<const KeyType, ValueType>*>(operator new(sizeof(std::pair<const KeyType, ValueType>) * capacity_));
        occupied_ = new char[capacity_];
        init_occupied_();
        fullness_ = 0;
        dead_elem_ = 0;
        for (auto v : elements) {
            insert(v);
        }
    }
    // Initializes the oсcupied with zeros.
    void init_occupied_() {
        for (size_t i = 0; i < capacity_; ++i) {
            occupied_[i] = EMPTY;
        }
    }
    // Linear search for a free cell with a step of 1.
    size_t find_position_(const KeyType& key) const {
        size_t seed = hasher_(key);
        size_t id = seed % capacity_;
        while (occupied_[id] != EMPTY && (occupied_[id] == DEAD || !(data_[id].first == key))) {
            ++id;
            if (id == capacity_) {
                id = 0;
            }
        }
        return id;
    }
    // Checks scalability condition.
    void adapt_size_() {
        if ((fullness_ + dead_elem_) * SIZELIMIT > capacity_) {
            change_capacity_(std::max(static_cast<size_t>(MINSIZE), fullness_ * INCREASE));
        }
    }
};
