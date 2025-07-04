#include "nbt.hpp"


NBTList::NBTList() : m_subType(eNBT::COMPOUND) {}


NBTList::NBTList(eNBT expectedType) : m_subType(expectedType) {}


NBTList::NBTList(eNBT expectedType, std::initializer_list<NBTBase> list) {
    m_subType = expectedType;
    for (const auto& item : list) {
        if (item.getType() != expectedType) {
            std::cerr << "Warning: Skipping mismatched NBT type in list (expected "
                      << NBTTypeToName(expectedType) << ", got "
                      << NBTTypeToName(item.getType()) << ")\n";
            continue;
        }
        push_back(item);
    }
}


NBTBase &NBTList::operator[](size_t index) { return m_elements[index]; }


const NBTBase &NBTList::operator[](size_t index) const { return m_elements[index]; }


size_t NBTList::size() const { return m_elements.size(); }


bool NBTList::empty() const { return m_elements.empty(); }


void NBTList::push_back(NBTBase val) {
    if (m_subType == eNBT::NONE) {
        m_subType = val.getType();
    } else if (val.getType() != m_subType) {
        std::cerr << "Warning: Appending mismatched NBT type (expected "
                  << NBTTypeToName(m_subType) << ", got "
                  << NBTTypeToName(val.getType()) << ")\n";
    }
    m_elements.push_back(std::move(val));
}


void NBTList::reserve(size_t n) { m_elements.reserve(n); }


void NBTList::clear() {
    m_elements.clear();
}



MU void NBTBase::read(DataReader& reader) {
    m_type = eNBT::COMPOUND;
    readInternal(reader);
}


MU void NBTBase::readFile(const std::string &path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("Failed to open NBT file");

    std::vector<char> buffer((std::istreambuf_iterator<char>(in)), {});
    DataReader reader(reinterpret_cast<u8*>(buffer.data()), buffer.size());

    read(reader);
}


void NBTBase::write(DataWriter& writer) const {
    writeInternal(writer);
}


MU void NBTBase::writeFile(const std::string &path) const {
    DataWriter writer;
    write(writer);

    std::ofstream out(path, std::ios::binary);
    if (!out) throw std::runtime_error("Failed to open file for writing");
    out.write(reinterpret_cast<const char*>(writer.data()), writer.size());
}


void NBTBase::readInternal(DataReader & reader) {
    switch (m_type) {
        case eNBT::UINT8: m_value = static_cast<u8>(reader.read<u8>()); break;
        case eNBT::INT16: m_value = static_cast<i16>(reader.read<u16>()); break;
        case eNBT::INT32: m_value = static_cast<i32>(reader.read<u32>()); break;
        case eNBT::INT64: m_value = static_cast<i64>(reader.read<u64>()); break;
        case eNBT::FLOAT: m_value = static_cast<float>(reader.read<float>()); break;
        case eNBT::DOUBLE: m_value = static_cast<double>(reader.read<double>()); break;

        case eNBT::BYTE_ARRAY: {
            i32 size = reader.read<i32>();
            c_u8* start = reader.ptr();
            reader.skip(size);
            m_value = NBTByteArray(start, start + size);
            break;
        }
        case eNBT::STRING: {
            c_u32 length = reader.read<u16>();
            m_value = reader.readString(length);
            break;
        }
        case eNBT::LIST: {
            auto subType = static_cast<eNBT>(reader.read<u8>());
            auto size = (i32) reader.read<u32>();
            NBTList list(subType);
            for (int i = 0; i < size; ++i) {
                NBTBase element(subType, {});
                element.readInternal(reader);
                list.push_back(std::move(element));
            }
            m_value = std::move(list);
            break;
        }
        case eNBT::COMPOUND: {
            NBTCompound compound;
            while (true) {
                if (reader.eof()) break;
                auto subType = static_cast<eNBT>(reader.read<u8>());
                if (subType == eNBT::NONE) break;
                c_u32 length = reader.read<u16>();
                std::string key = reader.readString(length);
                NBTBase subTag(subType, {});
                subTag.readInternal(reader);
                compound[key] = std::move(subTag);
            }
            m_value = std::move(compound);
            break;
        }
        case eNBT::INT_ARRAY: {
            i32 size = reader.read<i32>();
            NBTIntArray arr(size);
            for (int i = 0; i < size; ++i) arr[i] = reader.read<i32>();
            m_value = std::move(arr);
            break;
        }
        case eNBT::LONG_ARRAY: {
            i32 size = reader.read<i32>();
            NBTLongArray arr(size);
            for (int i = 0; i < size; ++i) arr[i] = reader.read<i64>();
            m_value = std::move(arr);
            break;
        }
        default: m_value = std::monostate{};
    }
}



void NBTBase::writeInternal(DataWriter & writer) const {
    switch (m_type) {
        case eNBT::UINT8: writer.write<u8>(get<u8>()); break;
        case eNBT::INT16: writer.write<u16>(get<i16>()); break;
        case eNBT::INT32: writer.write<u32>(get<i32>()); break;
        case eNBT::INT64: writer.write<u64>(get<i64>()); break;
        case eNBT::FLOAT: writer.write<float>(get<float>()); break;
        case eNBT::DOUBLE: writer.write<double>(get<double>()); break;
        case eNBT::BYTE_ARRAY: {
            const auto& arr = get<NBTByteArray>();
            writer.write<u32>(static_cast<i32>(arr.size()));
            writer.writeBytes(arr.data(), arr.size());
            break;
        }
        case eNBT::STRING: {
            writer.writeStringLengthPrefixed(get<std::string>());
            break;
        }
        case eNBT::LIST: {
            const auto& list = get<NBTList>();
            writer.write<u8>(static_cast<u8>(list.empty() ? eNBT::NONE : list.subType()));
            writer.write<u32>(static_cast<i32>(list.size()));
            for (const auto& item : list) item.writeInternal(writer);
            break;
        }
        case eNBT::COMPOUND: {
            const auto& compound = get<NBTCompound>();
            for (const auto& [key, val] : compound) {
                writer.write<u8>(static_cast<u8>(val.m_type));
                writer.writeStringLengthPrefixed(key);
                val.writeInternal(writer);
            }
            writer.write<u8>(static_cast<u8>(eNBT::NONE));
            break;
        }
        case eNBT::INT_ARRAY: {
            const auto& arr = get<NBTIntArray>();
            writer.write<u32>(static_cast<i32>(arr.size()));
            for (i32 v : arr) writer.write<u32>(v);
            break;
        }
        case eNBT::LONG_ARRAY: {
            const auto& arr = get<NBTLongArray>();
            writer.write<u32>(static_cast<i32>(arr.size()));
            for (i64 v : arr) writer.write<u64>(v);
            break;
        }
        default: break;
    }
}





void NBTBase::printHelper(int depth, const std::string &theKey) const {
    auto indent = [](int d) { return std::string(d * 2, ' '); };

    if (!theKey.empty())
        std::cout << indent(depth) << theKey << ": ";

    std::visit([&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, std::monostate>) {
            std::cout << "null\n";
        } else if constexpr (std::is_same_v<T, std::string>) {
            std::cout << "\"" << arg << "\"\n";
        } else if constexpr (std::is_same_v<T, u8>) {
            std::cout << static_cast<int>(arg) << "b\n";
        } else if constexpr (std::is_same_v<T, i16>) {
            std::cout << arg << "s\n";
        } else if constexpr (std::is_same_v<T, i32>) {
            std::cout << arg << "\n";
        } else if constexpr (std::is_same_v<T, i64>) {
            std::cout << arg << "L\n";
        } else if constexpr (std::is_same_v<T, float>) {
            std::cout << arg << "f\n";
        } else if constexpr (std::is_same_v<T, double>) {
            std::cout << arg << "d\n";
        } else if constexpr (std::is_same_v<T, NBTByteArray>) {
            std::cout << "u8[" << arg.size() << "]\n";
        } else if constexpr (std::is_same_v<T, NBTIntArray>) {
            std::cout << "i32[" << arg.size() << "]\n";
        } else if constexpr (std::is_same_v<T, NBTLongArray>) {
            std::cout << "i64[" << arg.size() << "]\n";
        } else if constexpr (std::is_same_v<T, NBTList>) {
            std::cout << "TAG_List[" << arg.size() << "] {\n";
            for (const auto& elem : arg) {
                elem.printHelper(depth + 1);
            }
            std::cout << indent(depth) << "}\n";
        } else if constexpr (std::is_same_v<T, NBTCompound>) {
            std::cout << "TAG_Compound[" << arg.size() << "] {\n";
            for (const auto& [key, val] : arg) {
                val.printHelper(depth + 1, key);
            }
            std::cout << indent(depth) << "}\n";
        } else {
            std::cout << "<unknown type>\n";
        }
    }, m_value);
}


bool NBTBase::equals(const NBTBase &other) const {
    if (m_type != other.m_type) return false;

    return std::visit([&](const auto& a) -> bool {
        using T = std::decay_t<decltype(a)>;

        if (!std::holds_alternative<T>(other.m_value)) return false;
        const auto& b = std::get<T>(other.m_value);

        if constexpr (std::is_same_v<T, std::monostate>) {
            return true;
        } else if constexpr (std::is_arithmetic_v<T> ||
                             std::is_same_v<T, std::string> ||
                             std::is_same_v<T, NBTByteArray> ||
                             std::is_same_v<T, NBTIntArray> ||
                             std::is_same_v<T, NBTLongArray>) {
            return a == b;
        } else if constexpr (std::is_same_v<T, NBTList>) {
            if (a.size() != b.size()) return false;
            for (size_t i = 0; i < a.size(); ++i)
                if (!a[i].equals(b[i])) return false;
            return true;
        } else if constexpr (std::is_same_v<T, NBTCompound>) {
            if (a.size() != b.size()) return false;
            for (const auto& [key, val] : a) {
                auto it = b.find(key);
                if (it == nullptr || !val.equals(*it)) return false;
            }
            return true;
        } else {
            return false;
        }
    }, m_value);
}


NBTBase NBTBase::copy() const {
    switch (m_type) {
        case eNBT::UINT8:     return makeByte(get<u8>());
        case eNBT::INT16:    return makeShort(get<i16>());
        case eNBT::INT32:    return makeInt(get<i32>());
        case eNBT::INT64:    return makeLong(get<i64>());
        case eNBT::FLOAT:    return makeFloat(get<float>());
        case eNBT::DOUBLE:   return makeDouble(get<double>());
        case eNBT::STRING:   return makeString(get<std::string>());
        case eNBT::BYTE_ARRAY: return makeByteArray(get<NBTByteArray>());
        case eNBT::INT_ARRAY:  return makeIntArray(get<NBTIntArray>());
        case eNBT::LONG_ARRAY: return makeLongArray(get<NBTLongArray>());

        case eNBT::LIST: {
            NBTList copyList(get<NBTList>().subType());
            for (const auto& item : get<NBTList>()) {
                copyList.push_back(item.copy());
            }
            return makeList(std::move(copyList));
        }

        case eNBT::COMPOUND: {
            NBTCompound copyCompound;
            for (const auto& [k, v] : get<NBTCompound>()) {
                copyCompound[k] = v.copy();
            }
            return makeCompound(std::move(copyCompound));
        }

        case eNBT::NONE:
        default:
            return {};
    }
}


MU std::vector<std::string> NBTBase::getKeySet() const {
    if (!is<NBTCompound>()) return {};
    const auto& compound = get<NBTCompound>();
    std::vector<std::string> keys;
    keys.reserve(compound.size());
    for (const auto& [key, _] : compound) {
        keys.push_back(key);
    }
    return keys;
}


MU bool NBTBase::hasKey(const std::string &key) const {
    return is<NBTCompound>() && get<NBTCompound>().contains(key);
}


bool NBTBase::hasKey(const std::string& key, const eNBT nbtType) {
    if (hasKey(key)) {
        if (m_type == nbtType) { return true; }
        if (m_type != eNBT::PRIMITIVE) { return false; }
        return nbtType == eNBT::UINT8 ||
               nbtType == eNBT::INT16 ||
               nbtType == eNBT::INT32 ||
               nbtType == eNBT::INT64 ||
               nbtType == eNBT::FLOAT ||
               nbtType == eNBT::DOUBLE;
    }
    return false;
}


MU ND bool NBTBase::hasKeys(const std::initializer_list<std::string> keys) const {
    if (!is<NBTCompound>()) return false;
    const auto& compound = get<NBTCompound>();
    for (auto& key : keys) {
        if (!compound.contains(key)) {
            return false;
        }
    }
    return true;
}


MU const NBTBase *NBTBase::getTag(const std::string &key) const {
    return is<NBTCompound>() ? get<NBTCompound>().find(key) : nullptr;
}


MU NBTBase *NBTBase::getTag(const std::string &key) {
    return is<NBTCompound>() ? get<NBTCompound>().find(key) : nullptr;
}


MU void NBTBase::setTag(const std::string &key, NBTBase tag) {
    if (!is<NBTCompound>()) {
        m_type  = eNBT::COMPOUND;
        m_value = NBTCompound{};
    }
    get<NBTCompound>()[key] = std::move(tag);
}


MU void NBTBase::removeTag(const std::string &key) {
    if (!is<NBTCompound>()) { return; }
    get<NBTCompound>().erase(key);
}


void NBTBase::merge(const NBTBase &other) {
    /* Only compounds can be merged.
       If *this* is not a compound yet we just deep-copy the RHS
       (behaves like Minecraft’s original NBT merge). */
    if (!is<NBTCompound>()) {
        if (other.is<NBTCompound>())
            *this = other.copy(); // whole-compound copy
        return;
    }
    if (!other.is<NBTCompound>())
        return; // nothing to merge

    auto&       selfCmp = get<NBTCompound>();
    const auto& rhsCmp  = other.get<NBTCompound>();

    /* Walk every entry of the RHS compound. */
    for (const auto& [key, rhsVal] : rhsCmp) {
        auto* selfPtr = selfCmp.find(key);

        /* Case 1: key exists in both & both values are compounds → recurse. */
        if (selfPtr && selfPtr->is<NBTCompound>() && rhsVal.is<NBTCompound>()) {
            selfPtr->merge(rhsVal); // recursive merge
            continue;
        }

        /* Case 2: otherwise just overwrite / insert with a deep copy. */
        selfCmp.insert(key, rhsVal.copy());
    }
}

NBTCompound::iterator::iterator(NBTCompound *m, NBTCompound::size_type i) : _map(m), _idx(i) {}


std::pair<const std::string &, NBTBase &> NBTCompound::iterator::operator*() const {
    return {_map->_keys[_idx], _map->_values[_idx]};
}


void NBTCompound::clear() {
    _keys.clear();
    _values.clear();
    for (auto& b : _buckets) b.clear();
    _size = 0;
}


NBTCompound::iterator NBTCompound::erase(const std::string& k) {
    const size_type bi      = _bucket_for(k);
    auto&           bucket  = _buckets[bi];

    for (size_type pos = 0; pos < bucket.size(); ++pos) {
        const size_type idx = bucket[pos];
        if (_keys[idx] != k) continue;

        iterator nextIt{ this, (idx + 1 < _values.size()) ? idx : _values.size() - 1 };

        bucket[pos] = bucket.back();
        bucket.pop_back();

        const size_type lastIdx = _values.size() - 1;
        if (idx != lastIdx) {
            _keys[idx]   = std::move(_keys[lastIdx]);
            _values[idx] = std::move(_values[lastIdx]);

            auto& lastBucket = _buckets[_bucket_for(_keys[idx])];
            for (auto& slot : lastBucket)
                if (slot == lastIdx) { slot = idx; break; }

            nextIt._idx = idx;
        }

        _keys.pop_back();
        _values.pop_back();
        --_size;

        return nextIt;
    }
    return end();
}


NBTBase &NBTCompound::operator[](const std::string &k) {
    if (auto p = find(k)) return *p;
    insert(k, NBTBase{});
    return *_values.rbegin();
}


void NBTCompound::insert(const std::string &k, const NBTBase &v) {
    auto bi = _bucket_for(k);
    for (auto idx : _buckets[bi]) {
        if (_keys[idx] == k) {
            _values[idx] = v;
            return;
        }
    }

    size_type newIndex = _values.size();
    _keys.push_back(k);
    _values.push_back(v);
    _buckets[bi].push_back(newIndex);
    ++_size;
    _maybe_rehash();
}


NBTCompound::iterator NBTCompound::begin() { return {this, 0}; }


NBTCompound::iterator NBTCompound::end() { return {this, _values.size()}; }


NBTCompound::NBTCompound(NBTCompound::size_type initialBuckets)
        : _buckets(initialBuckets) {}


void NBTCompound::_maybe_rehash() {
    if (static_cast<float>(_size) >
        static_cast<float>(_buckets.size()) *
                _max_load)
        _rehash(_buckets.size() * 2);
}


NBTCompound::const_iterator NBTCompound::begin() const { return {this, 0}; }


NBTCompound::const_iterator NBTCompound::end() const { return {this, _values.size()}; }


NBTBase *NBTCompound::find(const std::string &k) {
    return const_cast<NBTBase*>(
            static_cast<const NBTCompound*>(this)->find(k));
}


const NBTBase *NBTCompound::find(const std::string &k) const {
    const size_type bi = _bucket_for(k);
    for (auto idx : _buckets[bi])
        if (_keys[idx] == k) return &_values[idx];
    return nullptr;
}


NBTBase &NBTCompound::at(const std::string &k) {
    if (auto p = find(k)) return *p;
    throw std::out_of_range("NBTCompound::at – key not found");
}


const NBTBase &NBTCompound::at(const std::string &k) const {
    return const_cast<NBTCompound*>(this)->at(k);
}


std::pair<const std::string &, const NBTBase &> NBTCompound::const_iterator::operator*() const {
    return {_map->_keys[_idx], _map->_values[_idx]};
}


std::optional<NBTBase> NBTCompound::extract(const std::string& k) {
    const size_type bi = _bucket_for(k);
    auto& bucket       = _buckets[bi];

    for (size_type pos = 0; pos < bucket.size(); ++pos) {
        const size_type idx = bucket[pos];
        if (_keys[idx] != k) continue;

        std::optional<NBTBase> result{ std::move(_values[idx]) };

        bucket[pos] = bucket.back();
        bucket.pop_back();

        const size_type lastIdx = _values.size() - 1;
        if (idx != lastIdx) {
            _keys[idx]   = std::move(_keys[lastIdx]);
            _values[idx] = std::move(_values[lastIdx]);

            auto& lastB = _buckets[_bucket_for(_keys[idx])];
            for (auto& slot : lastB)
                if (slot == lastIdx) { slot = idx; break; }
        }

        _keys.pop_back();
        _values.pop_back();
        --_size;
        return result;
    }
    return std::nullopt;
}


std::optional<NBTBase> NBTBase::extractTag(const std::string& k) {
    return is<NBTCompound>() ? get<NBTCompound>().extract(k)
                             : std::nullopt;
}


template<typename T>
std::optional<T> NBTCompound::value(const std::string& key) const {
    if (const NBTBase* t = find(key); t && t->is<T>())
        return t->get<T>();
    return std::nullopt;
}


template std::optional<std::monostate>NBTCompound::value<std::monostate>(const std::string&) const;
template std::optional<u8>            NBTCompound::value<u8>(const std::string&) const;
template std::optional<i16>           NBTCompound::value<i16>(const std::string&) const;
template std::optional<i32>           NBTCompound::value<i32>(const std::string&) const;
template std::optional<i64>           NBTCompound::value<i64>(const std::string&) const;
template std::optional<float>         NBTCompound::value<float>(const std::string&) const;
template std::optional<double>        NBTCompound::value<double>(const std::string&) const;
template std::optional<NBTByteArray>  NBTCompound::value<NBTByteArray>(const std::string&) const;
template std::optional<std::string>   NBTCompound::value<std::string>(const std::string&) const;
template std::optional<NBTList>       NBTCompound::value<NBTList>(const std::string&) const;
template std::optional<NBTCompound>   NBTCompound::value<NBTCompound>(const std::string&) const;
template std::optional<NBTIntArray>   NBTCompound::value<NBTIntArray>(const std::string&) const;
template std::optional<NBTLongArray>  NBTCompound::value<NBTLongArray>(const std::string&) const;


template<typename T>
std::optional<T> NBTBase::value(const std::string& key) const {
    if (const NBTBase* t = get<NBTCompound>().find(key); t && t->is<T>())
        return t->get<T>();
    return std::nullopt;
}


template std::optional<std::monostate>NBTBase::value<std::monostate>(const std::string&) const;
template std::optional<u8>            NBTBase::value<u8>(const std::string&) const;
template std::optional<i16>           NBTBase::value<i16>(const std::string&) const;
template std::optional<i32>           NBTBase::value<i32>(const std::string&) const;
template std::optional<i64>           NBTBase::value<i64>(const std::string&) const;
template std::optional<float>         NBTBase::value<float>(const std::string&) const;
template std::optional<double>        NBTBase::value<double>(const std::string&) const;
template std::optional<NBTByteArray>  NBTBase::value<NBTByteArray>(const std::string&) const;
template std::optional<std::string>   NBTBase::value<std::string>(const std::string&) const;
template std::optional<NBTList>       NBTBase::value<NBTList>(const std::string&) const;
template std::optional<NBTCompound>   NBTBase::value<NBTCompound>(const std::string&) const;
template std::optional<NBTIntArray>   NBTBase::value<NBTIntArray>(const std::string&) const;
template std::optional<NBTLongArray>  NBTBase::value<NBTLongArray>(const std::string&) const;