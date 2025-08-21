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


inline const char* to_string(eNBT type) {
    switch (type) {
        case eNBT::NONE:        return "NONE";
        case eNBT::UINT8:       return "UINT8";
        case eNBT::INT16:       return "INT16";
        case eNBT::INT32:       return "INT32";
        case eNBT::INT64:       return "INT64";
        case eNBT::FLOAT:       return "FLOAT";
        case eNBT::DOUBLE:      return "DOUBLE";
        case eNBT::BYTE_ARRAY:  return "BYTE_ARRAY";
        case eNBT::STRING:      return "STRING";
        case eNBT::LIST:        return "LIST";
        case eNBT::COMPOUND:    return "COMPOUND";
        case eNBT::INT_ARRAY:   return "INT_ARRAY";
        case eNBT::LONG_ARRAY:  return "LONG_ARRAY";
        case eNBT::PRIMITIVE:   return "PRIMITIVE";
        default:                return "UNKNOWN";
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
    using       iterator = std::vector<NBTBase>::iterator;
    using const_iterator = std::vector<NBTBase>::const_iterator;

    NBTList();
    explicit NBTList(eNBT expectedType);
    NBTList(eNBT expectedType, std::initializer_list<NBTBase> list);

    ND size_t      size() const;
    ND bool        empty() const;
    MU void        clear();
    NBTBase&       operator[](size_t index);
    const NBTBase& operator[](size_t index) const;
    void push_back(NBTBase val);

    void reserve(size_t n);

       iterator       begin()       { return m_elements.begin(); }
       iterator       end()         { return m_elements.end(); }
    ND const_iterator begin() const { return m_elements.begin(); }
    ND const_iterator end()   const { return m_elements.end(); }

    // additional
    MU ND eNBT subType() const { return m_subType; }
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

    ND bool   empty() const noexcept { return _size == 0; }
    ND size_type size() const noexcept { return _size;  }

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

    template <typename T>
    MU ND T getOr(std::string_view key, T def) {
        return this->value<T>(std::string(key)).value_or(def);
    }

    template <typename T>
    ND T getOr(std::string_view key, T def) const {
        return value<T>(std::string(key)).value_or(def);
    }

    void insert(const std::string& k, const NBTBase& v);

    NBTBase& operator[](const std::string& k);

    const NBTBase& operator()(const std::string& k) const noexcept;

    ND NBTCompound copy() const;

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

    static NBTBase&  ensureCompound(NBTBase& self) {
        if (self.m_type != eNBT::COMPOUND)
            self = {eNBT::COMPOUND,   NBTCompound() };
        return self;
    }

public:
    NBTBase() = default;
    NBTBase(eNBT type, NBTValue val) : m_type(type), m_value(std::move(val)) {}

    NBTBase(const NBTBase& other)
        : m_type(other.m_type),
          m_value(other.m_value)
    {}

    NBTBase(NBTBase&& other) noexcept
        : m_type(other.m_type),       // steal
          m_value(std::move(other.m_value))
    {
        other.m_type  = eNBT::NONE;   // reset source
        other.m_value = {};           // monostate
    }

    NBTBase& operator=(const NBTBase& other) {
        if (this != &other) {
            m_type  = other.m_type;
            m_value = other.m_value;
        }
        return *this;
    }

    NBTBase& operator=(NBTBase&& other) noexcept {
        if (this == &other)
            return *this;

        eNBT      newType  = other.m_type;
        NBTValue  newValue = std::move(other.m_value);

        other.m_type  = eNBT::NONE;
        other.m_value.emplace<std::monostate>();

        m_type  = newType;
        m_value = std::move(newValue);

        return *this;
    }

    ND NBTBase copy() const;
    MU void merge(const NBTBase& other);

    ND static NBTBase read(DataReader& reader);
    ND static NBTBase readInternal(DataReader& reader, eNBT type);

    NBTBase readFile(const std::string& path);

    void write(DataWriter& writer, bool skipEndTag = false) const;
    void writeFile(const std::string& path) const;

    ND eNBT getType() const { return m_type; }

    template<typename T>
    ND bool is() const { return std::holds_alternative<T>(m_value); }

    template<typename T>
    T& get() { return std::get<T>(m_value); }

    template<typename T>
    ND const T& get() const { return std::get<T>(m_value); }

    template <typename T>
    MU ND T getOr(std::string_view key, T def) {
        return this->value<T>(std::string(key)).value_or(def);
    }

    template <typename T>
    ND T getOr(std::string_view key, T def) const {
        return value<T>(std::string(key)).value_or(def);
    }

    template <typename T>
    MU ND T getOr(std::string_view k1, std::string_view k2, T def) {
        if (auto v = this->value<T>(std::string(k1))) return *v;
        if (auto v = this->value<T>(std::string(k2))) return *v;
        return def;
    }

    template <typename T>
    ND T getOr(std::string_view k1, std::string_view k2, T def) const {
        if (auto v = value<T>(std::string(k1))) return *v;
        if (auto v = value<T>(std::string(k2))) return *v;
        return def;
    }

    template <typename T>
    std::optional<T> value(const std::string& key) const;

    void print(int depth = 0, const std::string& theKey = "") const;
    ND std::string to_string_shallow() const;

    MU ND bool hasKey(const std::string& key) const;
    MU ND bool hasKey(const std::string& key, eNBT nbtType);
    MU ND bool hasKeys(std::initializer_list<std::string> keys) const;
    MU ND std::vector<std::string> getKeySet() const;

    MU void removeTag(const std::string& key);

    std::optional<NBTBase> extract(const std::string& key) {
        if (!is<NBTCompound>()) return std::nullopt;
        return std::get<NBTCompound>(m_value).extract(key);
    }

    NBTList& ensureList(const std::string& key, eNBT subType) {
        NBTBase& tag = (*this)[key];
        if (!tag.is<NBTList>() ||
            (tag.get<NBTList>().subType() != subType)) {
            tag = {eNBT::LIST, NBTList(subType)};
        }
        return tag.get<NBTList>();
    }

    NBTBase& operator[](const std::string& key) {
        if (this->m_type != eNBT::COMPOUND)
            *this = {eNBT::COMPOUND,   NBTCompound() };
        auto& selfCmp = std::get<NBTCompound>(m_value);
        return selfCmp[key];
    }

    NBTBase &operator[](const std::string &key, NBTBase def) {
        if (m_type != eNBT::COMPOUND)
            *this = {eNBT::COMPOUND,   NBTCompound() };
        auto &cmp = std::get<NBTCompound>(m_value);

        if (!cmp.contains(key))
            cmp.insert(key, def);

        return cmp[key];
    }

    const NBTBase& operator()(const std::string& key) const noexcept {
        static const NBTBase g_nullTag;
        if (this->m_type != eNBT::COMPOUND) return g_nullTag;
        if (auto p = std::get<NBTCompound>(m_value).find(key)) return *p;
        return g_nullTag;
    }

    ND bool equals(const NBTBase& other) const;
    bool operator==(const NBTBase& other) const {
        return equals(other);
    }

    bool operator!=(const NBTBase& other) const {
        return !(*this == other);
    }

    explicit operator bool() const noexcept {
        return m_type != eNBT::NONE;
    }


    MU ND const NBTBase* getTag(const std::string& key) const;

private:
    void writeInternal(DataWriter& writer, bool skipEndTag, int depth) const;

};

// ----------------------------------------
// Factory Helpers
// ----------------------------------------

template<typename T> requires (std::is_enum_v<T> || std::is_integral_v<T>)
inline NBTBase makeByte(T v)                  { return {eNBT::UINT8,       static_cast<uint8_t>(v) }; }
template<typename T> requires (std::is_enum_v<T> || std::is_integral_v<T>)
inline NBTBase makeShort(T v)                 { return {eNBT::INT16,       static_cast<int16_t>(v) }; }
template<typename T> requires (std::is_enum_v<T> || std::is_integral_v<T>)
inline NBTBase makeInt(T v)                   { return { eNBT::INT32,      static_cast<int32_t>(v)}; }
template<typename T> requires (std::is_enum_v<T> || std::is_integral_v<T>)
inline NBTBase makeLong(T v)                  { return { eNBT::INT64,      static_cast<int64_t>(v) }; }
inline NBTBase makeFloat(float v)             { return { eNBT::FLOAT,      v }; }
inline NBTBase makeDouble(double v)           { return { eNBT::DOUBLE,     v }; }
inline NBTBase makeString(const char* s)      { return { eNBT::STRING,     std::string(s) }; }
inline NBTBase makeString(std::string_view v) { return { eNBT::STRING,     std::string(v) }; }
inline NBTBase makeString(std::string v)      { return { eNBT::STRING,     std::move(v) }; }
inline NBTBase makeByteArray(NBTByteArray v)  { return { eNBT::BYTE_ARRAY, std::move(v) }; }
inline NBTBase makeIntArray(NBTIntArray v)    { return { eNBT::INT_ARRAY,  std::move(v) }; }
inline NBTBase makeLongArray(NBTLongArray v)  { return { eNBT::LONG_ARRAY, std::move(v) }; }
inline NBTBase makeList(eNBT subType)         { return { eNBT::LIST,       NBTList(subType) }; }
inline NBTBase makeList(NBTList v)            { return { eNBT::LIST,       std::move(v) }; }
inline NBTBase makeCompound()                 { return { eNBT::COMPOUND,   NBTCompound() }; }
inline NBTBase makeCompound(NBTCompound v)    { return { eNBT::COMPOUND,   std::move(v) }; }


inline NBTBase makeList(eNBT subType, std::initializer_list<NBTBase> list) {
    return {eNBT::LIST, NBTList(subType, list) };
}

inline NBTBase makeCompound(std::initializer_list<std::pair<std::string, NBTBase>> list) {
    NBTCompound compound;
    for (auto&& [key, val] : list)
        compound[key] = val.copy();
    return {eNBT::COMPOUND, std::move(compound) };
}
