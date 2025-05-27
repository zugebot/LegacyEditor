#pragma once

#include "common/DataReader.hpp"
#include "common/DataWriter.hpp"


// ----------------------------------------
// NBT Type Enum
// ----------------------------------------

enum class eNBT : uint8_t {
    NONE = 0, ///< officially "END"
    UINT8 = 1,
    INT16 = 2,
    INT32 = 3,
    INT64 = 4,
    FLOAT = 5,
    DOUBLE = 6,
    BYTE_ARRAY = 7,
    STRING = 8,
    LIST = 9,
    COMPOUND = 10,
    INT_ARRAY = 11,
    LONG_ARRAY = 12,
    PRIMITIVE = 99
};

static std::string NBTTypeToName(eNBT theType) {
    switch (theType) {
        case eNBT::NONE: return "NBT_NONE";
        case eNBT::UINT8: return "NBT_UINT8";
        case eNBT::INT16: return "NBT_INT16";
        case eNBT::INT32: return "NBT_INT32";
        case eNBT::INT64: return "NBT_INT64";
        case eNBT::FLOAT: return "NBT_FLOAT";
        case eNBT::DOUBLE: return "NBT_DOUBLE";
        case eNBT::BYTE_ARRAY: return "TAG_BYTE_ARRAY";
        case eNBT::STRING: return "TAG_STRING";
        case eNBT::LIST: return "TAG_LIST";
        case eNBT::COMPOUND: return "TAG_COMPOUND";
        case eNBT::INT_ARRAY: return "TAG_INT_ARRAY";
        case eNBT::LONG_ARRAY: return "TAG_LONG_ARRAY";
        case eNBT::PRIMITIVE: return "TAG_PRIMITIVE";
        default: return "UNKNOWN";
    }
}

// ----------------------------------------
// Type Aliases
// ----------------------------------------

class NBTBase;

class NBTList {
    eNBT m_subType = eNBT::NONE;
    std::vector<NBTBase> m_elements;
public:

    NBTList();
    explicit NBTList(eNBT expectedType);
    NBTList(eNBT expectedType, std::initializer_list<NBTBase> list);
    
    MU ND eNBT subType() const { return m_subType; }
    
    NBTBase& operator[](size_t index);
    const NBTBase& operator[](size_t index) const;
    
    ND size_t size() const;
    ND bool empty() const;
    MU void clear();

    void push_back(NBTBase val);

    void reserve(size_t n);

    auto begin() { return m_elements.begin(); }
    auto end() { return m_elements.end(); }
    ND auto begin() const { return m_elements.begin(); }
    ND auto end() const { return m_elements.end(); }

    std::vector<NBTBase>& data() { return m_elements; }
    ND const std::vector<NBTBase>& data() const { return m_elements; }
};



class NBTCompound {
public:
    using size_type = std::size_t;

private:
    std::vector<std::string>            _keys;
    std::vector<NBTBase>                _values;
    std::vector<std::vector<size_type>> _buckets;
    size_type                           _size      = 0;
    float                               _max_load  = 1.0f;

    [[nodiscard]] size_type _bucket_for(const std::string& k) const noexcept {
        return std::hash<std::string>{}(k) % _buckets.size();
    }

    void _rehash(size_type new_bucket_count) {
        std::vector<std::vector<size_type>> newBuckets(new_bucket_count);
        for (size_type idx = 0; idx < _keys.size(); ++idx) {
            auto bi = std::hash<std::string>{}(_keys[idx]) % new_bucket_count;
            newBuckets[bi].push_back(idx);
        }
        _buckets.swap(newBuckets);
    }

    void _maybe_rehash();

public:
    explicit NBTCompound(size_type initialBuckets = 8);

    [[nodiscard]] bool   empty() const noexcept { return _size == 0; }
    [[nodiscard]] size_type size() const noexcept { return _size;  }

    template <typename T>
    std::optional<T> value(const std::string& key) const;
    NBTBase*       find(const std::string& k);
    ND const NBTBase* find(const std::string& k) const;
    ND std::optional<NBTBase> extract(const std::string& k);

    // template
    // NBTBase*       findOrDefault(const std::string& k);
    ND bool contains(const std::string& k) const { return find(k) != nullptr; }

    NBTBase&       at(const std::string& k);
    ND const NBTBase& at(const std::string& k) const;

    void insert(const std::string& k, const NBTBase& v);

    NBTBase& operator[](const std::string& k);


    void clear();

    struct iterator {
        NBTCompound* _map;
        size_type    _idx;

        iterator(NBTCompound* m, size_type i);
        bool operator!=(const iterator& o) const { return _idx != o._idx; }
        void operator++()   { ++_idx; }
        std::pair<const std::string&, NBTBase&> operator*() const;
    };

    iterator erase(const std::string& k);
    iterator begin();
    iterator end();

    struct const_iterator {
        const NBTCompound* _map;
        size_type          _idx;
        const_iterator(const NBTCompound* m, size_type i) : _map(m), _idx(i) {}
        bool operator!=(const const_iterator& o) const { return _idx != o._idx; }
        void operator++()   { ++_idx; }
        std::pair<const std::string&, const NBTBase&> operator*() const;
    };
    ND const_iterator begin() const;
    ND const_iterator end()   const;
};


using NBTByteArray  = std::vector<uint8_t>;
using NBTIntArray   = std::vector<int32_t>;
using NBTLongArray  = std::vector<int64_t>;

using NBTValue = std::variant<
        std::monostate,
        uint8_t,
        int16_t,
        int32_t,
        int64_t,
        float,
        double,
        NBTByteArray,
        std::string,
        NBTList,
        NBTCompound,
        NBTIntArray,
        NBTLongArray
        >;

// ----------------------------------------
// NBTBase Class
// ----------------------------------------

class NBTBase {
    eNBT m_type = eNBT::NONE;
    NBTValue m_value;

public:
    NBTBase() = default;
    NBTBase(eNBT type, NBTValue val) : m_type(type), m_value(std::move(val)) {}

    ND NBTBase copy() const;
    ND bool equals(const NBTBase& other) const;


    void read(DataReader& reader);
    void readFile(const std::string& path);

    void write(DataWriter& writer) const;
    void writeFile(const std::string& path) const;

    ND eNBT getType() const { return m_type; }

    template<typename T>
    ND bool is() const { return std::holds_alternative<T>(m_value); }

    template<typename T>
    T& get() { return std::get<T>(m_value); }

    template<typename T>
    const T& get() const { return std::get<T>(m_value); }

    template <typename T>
    std::optional<T> value(const std::string& key) const;

    void print() const { printHelper(0, ""); }

    MU ND bool hasKey(const std::string& key) const;
    MU ND bool hasKey(const std::string& key, eNBT nbtType);
    MU ND bool hasKeys(std::initializer_list<std::string> keys) const;
    MU ND std::vector<std::string> getKeySet() const;

    MU ND const NBTBase* getTag(const std::string& key) const;
    MU NBTBase* getTag(const std::string& key);
    std::optional<NBTBase> extractTag(const std::string& k);

    MU void setTag(const std::string& key, NBTBase tag);
    MU void removeTag(const std::string& key);

    MU void merge(const NBTBase& other);

private:
    void readInternal(DataReader& reader);
    void writeInternal(DataWriter& writer) const;

    void printHelper(int depth = 0, const std::string& theKey = "") const;
};

// ----------------------------------------
// Factory Helpers
// ----------------------------------------

inline NBTBase makeByte(uint8_t v)                     { return {eNBT::UINT8, v }; }
inline NBTBase makeShort(int16_t v)                   { return {eNBT::INT16, v }; }
inline NBTBase makeInt(int32_t v)                     { return {eNBT::INT32, v }; }
inline NBTBase makeLong(int64_t v)                    { return {eNBT::INT64, v }; }
inline NBTBase makeFloat(float v)                     { return {eNBT::FLOAT, v }; }
inline NBTBase makeDouble(double v)                   { return {eNBT::DOUBLE, v }; }
inline NBTBase makeString(std::string v)              { return {eNBT::STRING, std::move(v) }; }
inline NBTBase makeByteArray(NBTByteArray v)          { return {eNBT::BYTE_ARRAY, std::move(v) }; }
inline NBTBase makeIntArray(NBTIntArray v)            { return {eNBT::INT_ARRAY, std::move(v) }; }
inline NBTBase makeLongArray(NBTLongArray v)          { return {eNBT::LONG_ARRAY, std::move(v) }; }
inline NBTBase makeList(NBTList v)                    { return {eNBT::LIST, std::move(v) }; }
inline NBTBase makeCompound(NBTCompound v)            { return {eNBT::COMPOUND, std::move(v) }; }


inline NBTBase makeList(eNBT expectedType) {
    NBTList result(expectedType, {});
    return {eNBT::LIST, std::move(result) };
}


inline NBTBase makeList(eNBT expectedType, std::initializer_list<NBTBase> list) {
    NBTList result(expectedType, list);
    return {eNBT::LIST, std::move(result) };
}

MU inline NBTBase makeCompound(std::initializer_list<std::pair<std::string, NBTBase>> list) {
    NBTCompound compound;
    for (auto&& [key, val] : list)
        compound[key] = val;
    return {eNBT::COMPOUND, std::move(compound) };
}

MU inline NBTBase makeCompound() {
    NBTCompound compound;
    return {eNBT::COMPOUND, std::move(compound) };
}
