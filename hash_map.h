#include<algorithm>
#include<cstddef>
#include<initializer_list>
#include<memory>
#include<list>

// This is a hash table class with open addressing based on linear probing.
// Deleted items are marked dead and cleared when the table capacity is reallocated.
// Mathematical expectation of running time is O(1). Hash table is implemented scaling up (https://en.wikipedia.org/wiki/Open_addressing).

// A "bidirectional" iterator (Biterator) class for Hash Table formulaic of pair (key, value).
template<class KeyType, class ValueType>
class Biterator: public std::iterator<std::forward_iterator_tag, std::pair<KeyType, ValueType>> {
  public:
    // Default constructor.
    Biterator(): ptr(nullptr), it(nullptr) {}
    // Сonstructor by pointer to a pair and iterator. 
    Biterator(std::pair<KeyType, ValueType>* _ptr, std::list<size_t>::iterator _it) {
        ptr = _ptr;
        it = _it;
    }
    // Operator ++. All the operators below work for O(1).
    Biterator& operator++() {
        ++it;
        return *this;
    }
    
    Biterator operator ++(int) {
        Biterator temp = *this;
        ++*this;
        return temp;
    }

    Biterator& operator--() {
        --it;
        return *this;
    }
    
    Biterator operator --(int) {
        Biterator temp = *this;
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

    bool operator==(const Biterator& other) const {
        return it == other.it;
    }
    
    bool operator!=(const Biterator& other) const {
        return it != other.it;        
    }
    
  private:
    std::pair<KeyType, ValueType>* ptr;
    std::list<size_t>::iterator it;
};
// A const "bidirectional" iterator (Biterator) class for Hash Table formulaic of pair (key, value).
template<class KeyType, class ValueType>
class ConstBiterator: public std::iterator<std::forward_iterator_tag, std::pair<KeyType, ValueType>> {
  public:
    // Default constuctor.
    ConstBiterator(): ptr(nullptr), it(nullptr) {}
    // Сonstructor by pointer to a pair and iterator. 
    ConstBiterator(std::pair<KeyType, ValueType>* _ptr, const std::list<size_t>::const_iterator _it) {
        ptr = _ptr;
        it = _it;
    }
    // Operator ++. All the operators below work for O(1).
    ConstBiterator& operator++() {
        ++it;
        return *this;
    }
    
    ConstBiterator operator ++(int) {
        ConstBiterator temp = *this;
        ++*this;
        return temp;
    }

    ConstBiterator& operator--() {
        --it;
        return *this;
    }
    
    ConstBiterator operator --(int) {
        ConstBiterator temp = *this;
        --*this;
        return temp;
    }

    std::pair<KeyType, ValueType>& operator*() const {
        return *(ptr + (*it));
    }
    
    const std::pair<KeyType, ValueType>* operator->() const {
        return (ptr + (*it));
    }

    bool operator==(const ConstBiterator& other) const {
        return it == other.it;
    }

    bool operator!=(const ConstBiterator& other) const {
        return it != other.it;
    }
    
  private:
    std::pair<KeyType, ValueType>* ptr;
    std::list<size_t>::const_iterator it;
};
// Hash table with open addressing based on linear probing.
// Deleted items are marked dead and cleared when the table capacity is reallocated.
// Mathematical expectation of running time is O(1). Hash table is implemented scaling up (https://en.wikipedia.org/wiki/Open_addressing).

template<class KeyType, class ValueType, class Hash = std::hash<KeyType> >
class HashMap {
  public:
    constexpr static int SIZELIMIT = 4, INCREASE = 8, MINSIZE = 8, ALIVE = 1, DEAD = 2, EMPTY = 0;
    typedef Biterator<const KeyType, ValueType> iterator;
    typedef ConstBiterator<const KeyType, ValueType> const_iterator;    
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
        nodes_.push_back(id);
        iters_[id] = --nodes_.end();
        adapt_size_();   
    }
    // Deleting an item by marking it dead.
    void erase(const KeyType& key) {
        size_t id = find_position_(key);
        if (occupied_[id] != ALIVE || !(data_[id].first == key)) return;
        occupied_[id] = DEAD;
        nodes_.erase(iters_[id]);
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
    // All simple actions with iterators work in O (1). 
    iterator begin() {
        iterator it(data_, nodes_.begin());
        return it;
    }
        
    const_iterator begin() const {
        const_iterator it(data_, nodes_.begin());
        return it;
    }
    
    iterator end() {
        iterator it(data_, nodes_.end());
        return it;
    }
    
    const_iterator end() const {
        const_iterator it(data_, nodes_.end());        
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
        const_iterator it(data_, iters_[id]);      
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
        iterator it(data_, iters_[id]);      
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
        for (size_t id : nodes_) {
            occupied_[id] = DEAD;
            iters_[id] = nodes_.end();
            dead_elem_++;
        }
        fullness_ = 0;
        nodes_.clear();
        change_capacity_(MINSIZE);
    }
    
    HashMap& operator = (const HashMap& a) {
        std::list<std::pair<const KeyType, ValueType>> tmp;
        hasher_ = a.hasher_;
        nodes_ = a.nodes_;
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
    std::vector<std::list<size_t>::iterator> iters_;
    std::list<size_t> nodes_;
    Hash hasher_;
    
    typedef std::pair<const KeyType, ValueType> PtrClass;
    // Clean memory, call destructor for each element, clean nodes list.
    void deallocate_() {
        for (size_t i = 0; i < capacity_; ++i) {
            if (occupied_[i] != 0) {
                (data_ + i) -> ~PtrClass();
            }
        }
        nodes_.clear();
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
        iters_.resize(capacity_, nodes_.end());            
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
