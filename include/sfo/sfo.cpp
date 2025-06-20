#include "sfo.hpp"

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>



SFOManager::SFOManager(std::string theFilePath)
    : m_filePath(std::move(theFilePath)), myFile(nullptr), myEntries(nullptr) {
    loadFile();
}


SFOManager::SFOManager()
    : m_filePath(), myFile(nullptr), myEntries(nullptr) {}


struct pkg_table_entry {
    uint32_t id;
    uint32_t filename_offset;
    uint32_t flags1;
    uint32_t flags2;
    uint32_t offset;
    uint32_t size;
    uint64_t padding;
};


std::vector<SFOManager::Attribute> SFOManager::getAttributes() const {
    std::vector<Attribute> attributes;
    for (uint32_t i = 0; i < myHeader.myEntriesCount; i++) {
        Attribute attr;
        attr.myKey = &myKeyTable.content[myEntries[i].key_offset];

        auto entry = myEntries[i];
        switch (entry.param_fmt) {
            // TODO: this needs to be re-thought out for ps4 "FORMAT"
            case eSFO_FMT::UTF8_SPECIAL:
            case eSFO_FMT::UTF8_NORMAL: {
                const char* ptr = &myDataTable.content[entry.data_offset];
                const uint32_t include_null = entry.param_fmt == eSFO_FMT::UTF8_NORMAL;
                attr.myValue = std::string(ptr, entry.param_len - include_null);
                // attr.myValue = "\"" + attr.myValue + "\"";
                break;
            }
            case eSFO_FMT::INT: {
                auto* integer = (uint32_t*)&myDataTable.content[entry.data_offset];
                attr.myValue = std::to_string(*integer);
                break;
            }
        }
        attributes.push_back(attr);
    }
    return attributes;
}


template<class T>
T* cast_realloc(T* obj, uint32_t size) {
    void* ptr = realloc(obj, size);
    return static_cast<T*>(ptr);
}


template<class T>
T* cast_malloc(uint32_t size) {
    void* ptr = malloc(size);
    return static_cast<T*>(ptr);
}


[[maybe_unused]] void SFOManager::addParam(const eSFO_FMT& theType, const std::string& theKey, const std::string& value) {
    index_table_entry newEntry = {};
    uint32_t newIndex = 0;

    // Get new entry's .param_len and .param_max_len
    if (theType == eSFO_FMT::UTF8_NORMAL || theType == eSFO_FMT::UTF8_SPECIAL) {
        newEntry.param_fmt = theType;
        newEntry.param_max_len = getReservedStringLen(theKey);
        newEntry.param_len = value.length() + (theType == eSFO_FMT::UTF8_NORMAL);
        if (newEntry.param_max_len < newEntry.param_len) {
            newEntry.param_max_len = newEntry.param_len;
            // 4-byte alignment
            while (newEntry.param_max_len % 4) {
                newEntry.param_max_len++;
            }
        }
    } else {
        newEntry.param_fmt = eSFO_FMT::INT;
        newEntry.param_len = 4;
        newEntry.param_max_len = 4;
    }

    // Get new entry's index and offsets
    for (uint32_t i = 0; i < myHeader.myEntriesCount; i++) {
        int32_t result = theKey.compare(&myKeyTable.content[myEntries[i].key_offset]);
        if (result == 0) { // Parameter already exists
            throw std::runtime_error("Could not add \"" + theKey + "\": parameter already exists.");
        } else if (result < 0) {
            newIndex = i;
            newEntry.key_offset = myEntries[i].key_offset;
            newEntry.data_offset = myEntries[i].data_offset;
            break;
        } else if (i == myHeader.myEntriesCount - 1) {
            newIndex = i + 1;
            newEntry.key_offset = myEntries[i].key_offset + strlen(&myKeyTable.content[myEntries[i].key_offset]) + 1;
            newEntry.data_offset = myEntries[i].data_offset + myEntries[i].param_max_len;
            break;
        }
    }

    // Make room for the new index table entry by moving the old ones
    myHeader.myEntriesCount++;
    const uint32_t newSize = INDEX_TABLE_ENTRY_SIZE * myHeader.myEntriesCount;
    myEntries = cast_realloc<index_table_entry>(myEntries, newSize);
    for (uint32_t i = myHeader.myEntriesCount - 1; i > newIndex; i--) {
        myEntries[i] = myEntries[i - 1];
        myEntries[i].key_offset += theKey.length() + 1;
        myEntries[i].data_offset += newEntry.param_max_len;
    }

    // Insert new index table entry
    std::memcpy(&myEntries[newIndex], &newEntry, sizeof(index_table_entry));

    // Resize key table
    myKeyTable.size += theKey.length() + 1;
    myKeyTable.content = cast_realloc<char>(myKeyTable.content, myKeyTable.size);
    // Move higher indexed keys to make room for new key
    for (uint32_t i = myKeyTable.size - 1; i > newEntry.key_offset + theKey.length(); i--) {
        myKeyTable.content[i] = myKeyTable.content[i - theKey.length() - 1];
    }
    // Insert new key
    std::memcpy(&myKeyTable.content[newEntry.key_offset], theKey.c_str(), theKey.length() + 1);
    padTable(&myKeyTable);

    // Resize data table
    expandDataTable(newEntry.data_offset, newEntry.param_max_len);

    // Insert new data
    char* ptr = &myDataTable.content[myEntries[newIndex].data_offset];
    if (theType == eSFO_FMT::UTF8_NORMAL) {
        memset(ptr, 0, newEntry.param_len); // Overwrite whole space with zeros first
        std::memcpy(ptr, value.c_str(), value.length() + 1); // Then copy new value

    } if (theType == eSFO_FMT::UTF8_SPECIAL) {
        memset(ptr, 0, newEntry.param_len); // Overwrite whole space with zeros first
        std::memcpy(ptr, value.c_str(), value.length()); // Then copy new value

    } else if (theType == eSFO_FMT::INT) {
        uint32_t new_value = strtoul(value.c_str(), nullptr, 0);
        std::memcpy(ptr, &new_value, 4);
    }
}


[[maybe_unused]] void SFOManager::deleteParam(const std::string& theKey) {
    int index = getIndex(theKey);
    if (index < 0) { // Parameter not found
        throw std::runtime_error("Could not delete \"" + theKey + "\": parameter not found.");
    }

    // Delete parameter from key table
    for (uint32_t i = myEntries[index].key_offset; i < myKeyTable.size - theKey.length() - 1; i++) {
        myKeyTable.content[i] = myKeyTable.content[i + theKey.length() + 1];
    }

    // Resize key table
    myKeyTable.size -= theKey.length() + 1;
    myKeyTable.content = cast_realloc<char>(myKeyTable.content, myKeyTable.size);
    padTable(&myKeyTable);

    // Delete parameter from data table
    for (uint32_t i = myEntries[index].data_offset; i < myDataTable.size - myEntries[index].param_max_len; i++) {
        myDataTable.content[i] = myDataTable.content[i + myEntries[index].param_max_len];
    }

    // Resize data table
    myDataTable.size -= myEntries[index].param_max_len;
    if (myDataTable.size) {
        myDataTable.content = cast_realloc<char>(myDataTable.content, myDataTable.size);
    } else {
        free(myDataTable.content);
        myDataTable.content = nullptr;
    }

    // Delete parameter from index table
    uint32_t param_max_len = myEntries[index].param_max_len;
    for (uint32_t i = index; i < myHeader.myEntriesCount - 1; i++) {
        myEntries[i] = myEntries[i + 1];
        myEntries[i].key_offset -= theKey.length() + 1;
        myEntries[i].data_offset -= param_max_len;
    }

    // Resize index table
    myHeader.myEntriesCount--;
    if (myHeader.myEntriesCount) {
        const uint32_t newSize = getTableSize();
        myEntries = cast_realloc<index_table_entry>(myEntries, newSize);
    } else {
        free(myEntries);
        myEntries = nullptr;
    }
}


[[maybe_unused]] void SFOManager::editParam(const std::string& theKey, const std::string& theValue) {
    int index = getIndex(theKey);
    if (index == -1) { // Parameter not found
        throw std::runtime_error("Could not edit \"" + theKey + "\": parameter not found.");
    }

    index_table_entry& entry = myEntries[index];

    // TODO: this should probably be removed
    if (theKey == "ACCOUNT_ID") {
        memset(&myDataTable.content[entry.data_offset], 0, entry.param_max_len);
        snprintf(&myDataTable.content[entry.data_offset], entry.param_max_len + 1, "%s", theValue.c_str());
        return;
    }


    switch (entry.param_fmt) {
        case eSFO_FMT::UTF8_NORMAL:
        case eSFO_FMT::UTF8_SPECIAL: {
            entry.param_len = theValue.length() + (entry.param_fmt == eSFO_FMT::UTF8_NORMAL);
            // Enlarge data table if new string is longer than allowed
            int diff = (int)(entry.param_len - entry.param_max_len);
            if (diff > 0) {
                uint32_t offset = entry.data_offset + entry.param_max_len;
                entry.param_max_len = entry.param_len;

                // 4-byte alignment
                while (entry.param_max_len % 4) {
                    entry.param_max_len++;
                    diff++;
                }

                expandDataTable(offset, diff);

                // Adjust follow-up index table entries' data offsets
                for (uint32_t i = index + 1; i < myHeader.myEntriesCount; i++) {
                    myEntries[i].data_offset += diff;
                }
            }
            // Overwrite old data with zeros
            memset(&myDataTable.content[entry.data_offset], 0, entry.param_max_len);
            // Save new string to data table
            snprintf(&myDataTable.content[entry.data_offset], entry.param_max_len, "%s", theValue.c_str());
            break;
        }
        case eSFO_FMT::INT: {
            uint32_t integer = strtoul(theValue.c_str(), nullptr, 0);
            std::memcpy(&myDataTable.content[entry.data_offset], &integer, 4);
            break;
        }
    }
}


std::string SFOManager::getAttribute(const std::string& theKey) const {
    auto attrs = getAttributes();
    for (const auto& attr : attrs) {
        if (attr.myKey == theKey) {
            return attr.myValue;
        }
    }
    return "";
}


void SFOManager::setMagic(eSFO_MAGIC magic) {
    myHeader.myMagic = magic;
    myHeader.myVersion = 0x00000101;
}


// TODO: make this return a fail code
[[maybe_unused]] void SFOManager::saveToFile(const std::string& outputPath) {
    FILE* outFile = fopen(outputPath.c_str(), "wb");
    if (outFile == nullptr) {
        throw std::runtime_error("Could not open file \"" + outputPath + "\" in write mode.");
    }

    // Adjust header's table offsets before saving
    uint32_t full_index_table_size = getTableSize();
    myHeader.myKeyTableOffset = sizeof(header) + full_index_table_size;
    myHeader.myDataTableOffset = myHeader.myKeyTableOffset + myKeyTable.size;

    std::string outPathF = "to file \"" + outputPath + "\".";
    if (fwrite(&myHeader, sizeof(header), 1, outFile) != 1) {
        throw std::runtime_error("Could not write header " + outPathF);
    }
    if (myHeader.myEntriesCount && fwrite(myEntries, full_index_table_size, 1, outFile) != 1) {
        throw std::runtime_error("Could not write index table " + outPathF);
    }
    if (myKeyTable.size && fwrite(myKeyTable.content, myKeyTable.size, 1, outFile) != 1) {
        throw std::runtime_error("Could not write key table " + outPathF);
    }
    if (myDataTable.size && fwrite(myDataTable.content, myDataTable.size, 1, outFile) != 1) {
        throw std::runtime_error("Could not write data table " + outPathF);
    }

    fclose(outFile);
}


static uint32_t bswap_32(uint32_t val) {
    val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0x00FF00FF);
    return (val << 16) | (val >> 16);
}


void SFOManager::loadFile() {
    myFile = fopen(m_filePath.c_str(), "rb");
    if (!myFile) {
        throw std::runtime_error("Could not open file \"" + m_filePath + "\".");
    }

    uint32_t magic;
    fread(&magic, 4, 1, myFile);
    if (magic == eSFO_MAGIC::PS4_PKG) {
        fseek(myFile, getPS4PkgOffset(), SEEK_SET);
    } else if (magic == eSFO_MAGIC::PS3_DISC) {
        fseek(myFile, 0x800, SEEK_SET);
    } else if (magic == eSFO_MAGIC::PS3_HDD) {
        rewind(myFile);
    } else {
        throw std::runtime_error("Param.sfo magic number not found.");
    }

    loadHeader();
    loadEntries();
    if (myEntries == nullptr) {
        throw std::runtime_error("failed to read SFO file\n");
    }
    loadKeyTable();
    loadDataTable();
}


long int SFOManager::getPS4PkgOffset() {
    uint32_t pkg_table_offset;
    uint32_t pkg_file_count;

    fseek(myFile, 0x00C, SEEK_SET);
    fread(&pkg_file_count, 4, 1, myFile);
    fseek(myFile, 0x018, SEEK_SET);
    fread(&pkg_table_offset, 4, 1, myFile);

    pkg_file_count = bswap_32(pkg_file_count);
    pkg_table_offset = bswap_32(pkg_table_offset);

    std::vector<pkg_table_entry> pkg_table_entries(pkg_file_count);

    fseek(myFile, (long)pkg_table_offset, SEEK_SET);
    fread(pkg_table_entries.data(), sizeof(pkg_table_entry), pkg_file_count, myFile);

    for (uint32_t i = 0; i < pkg_file_count; i++) {
        if (pkg_table_entries[i].id == 1048576) { // param.sfo ID
            return (long)bswap_32(pkg_table_entries[i].offset);
        }
    }

    throw std::runtime_error("Could not find a param.sfo file inside the PS4 PKG.");
}


void SFOManager::loadHeader() {
    if (fread(&myHeader, sizeof(header), 1, myFile) != 1) {
        throw std::runtime_error("Could not read header.");
    }
}


void SFOManager::loadEntries() {
    uint32_t size = INDEX_TABLE_ENTRY_SIZE * myHeader.myEntriesCount;
    myEntries = cast_malloc<index_table_entry>(size);
    if (myEntries == nullptr) {
        throw std::runtime_error("Could not allocate memory for index table.");
    }
    if (size && fread(myEntries, size, 1, myFile) != 1) {
        throw std::runtime_error("Could not read index table entries.");
    }
}


void SFOManager::loadKeyTable() {
    myKeyTable.size = myHeader.myDataTableOffset - myHeader.myKeyTableOffset;
    myKeyTable.content = cast_malloc<char>(myKeyTable.size);
    if (myKeyTable.content == nullptr) {
        throw std::runtime_error("Could not allocate memory for key table.");
    }
    if (myKeyTable.size && fread(myKeyTable.content, myKeyTable.size, 1, myFile) != 1) {
        throw std::runtime_error("Could not read key table.");
    }
}


void SFOManager::loadDataTable() {
    if (myHeader.myEntriesCount) {
        index_table_entry entry = myEntries[myHeader.myEntriesCount - 1];
        myDataTable.size = entry.data_offset + entry.param_max_len;
    } else {
        myDataTable.size = 0; // For newly created, empty param.sfo files
    }
    myDataTable.content = cast_malloc<char>(myDataTable.size);
    if (myDataTable.content == nullptr) {
        throw std::runtime_error("Could not allocate memory for data table.");
    }
    if (myDataTable.size && fread(myDataTable.content, myDataTable.size, 1, myFile) != 1) {
        throw std::runtime_error("Could not read data table.");
    }
}


int SFOManager::getIndex(const std::string& theKey) {
    for (uint32_t i = 0; i < myHeader.myEntriesCount; i++) {
        if (theKey == &myKeyTable.content[myEntries[i].key_offset]) {
            return static_cast<int>(i);
        }
    }
    return -1;
}


/// returns sizeof(index_table_entry) * myHeader.myEntriesCount
uint32_t SFOManager::getTableSize() const {
    return sizeof(index_table_entry) * myHeader.myEntriesCount;
}


uint32_t SFOManager::getReservedStringLen(const std::string& theKey) {
    int len = 0;
    if (theKey == "CATEGORY" || theKey == "FORMAT") {
        len = 4;
    } else if (theKey == "APP_VER" ||
               theKey == "CONTENT_VER" ||
               theKey == "VERSION" ||
               theKey == "SAVEDATA_LIST_PARAM") {
        len = 8;
    } else if (theKey == "INSTALL_DIR_SAVEDATA" ||
               theKey == "TITLE_ID" ||
               theKey == "PARAMS2") {
        len = 12;
    } else if (theKey == "SERVICE_ID_ADDCONT_ADD_1" ||
               theKey == "SERVICE_ID_ADDCONT_ADD_2" ||
               theKey == "SERVICE_ID_ADDCONT_ADD_3" ||
               theKey == "SERVICE_ID_ADDCONT_ADD_4" ||
               theKey == "SERVICE_ID_ADDCONT_ADD_5" ||
               theKey == "SERVICE_ID_ADDCONT_ADD_6" ||
               theKey == "SERVICE_ID_ADDCONT_ADD_7") {
        len = 20;
    } else if (theKey == "SAVEDATA_DIRECTORY") {
        len = 32;
    } else if (theKey == "RPCS3_BLIST") {
        len = 36;
    } else if (theKey == "CONTENT_ID") {
        len = 48;
    } else if (theKey == "PROVIDER" || theKey == "TITLE" ||
               theKey == "PROVIDER_00" || theKey == "TITLE_00" ||
               theKey == "PROVIDER_01" || theKey == "TITLE_01" ||
               theKey == "PROVIDER_02" || theKey == "TITLE_02" ||
               theKey == "PROVIDER_03" || theKey == "TITLE_03" ||
               theKey == "PROVIDER_04" || theKey == "TITLE_04" ||
               theKey == "PROVIDER_05" || theKey == "TITLE_05" ||
               theKey == "PROVIDER_06" || theKey == "TITLE_06" ||
               theKey == "PROVIDER_07" || theKey == "TITLE_07" ||
               theKey == "PROVIDER_08" || theKey == "TITLE_08" ||
               theKey == "PROVIDER_09" || theKey == "TITLE_09" ||
               theKey == "PROVIDER_10" || theKey == "TITLE_10" ||
               theKey == "PROVIDER_11" || theKey == "TITLE_11" ||
               theKey == "PROVIDER_12" || theKey == "TITLE_12" ||
               theKey == "PROVIDER_13" || theKey == "TITLE_13" ||
               theKey == "PROVIDER_14" || theKey == "TITLE_14" ||
               theKey == "PROVIDER_15" || theKey == "TITLE_15" ||
               theKey == "PROVIDER_16" || theKey == "TITLE_16" ||
               theKey == "PROVIDER_17" || theKey == "TITLE_17" ||
               theKey == "PROVIDER_18" || theKey == "TITLE_18" ||
               theKey == "PROVIDER_19" || theKey == "TITLE_19" ||
               theKey == "PROVIDER_20" || theKey == "TITLE_20" ||
               theKey == "TITLE_21" || theKey == "TITLE_22" ||
               theKey == "TITLE_23" || theKey == "TITLE_24" ||
               theKey == "TITLE_25" || theKey == "TITLE_26" ||
               theKey == "TITLE_27" || theKey == "TITLE_28" ||
               theKey == "TITLE_29" || theKey == "SUB_TITLE") {
        len = 128;
    } else if (theKey == "PUBTOOLINFO" || theKey == "PS3_TITLE_ID_LIST_FOR_BOOT" ||
               theKey == "SAVE_DATA_TRANSFER_TITLE_ID_LIST_1" ||
               theKey == "SAVE_DATA_TRANSFER_TITLE_ID_LIST_2" ||
               theKey == "SAVE_DATA_TRANSFER_TITLE_ID_LIST_3" ||
               theKey == "SAVE_DATA_TRANSFER_TITLE_ID_LIST_4" ||
               theKey == "SAVE_DATA_TRANSFER_TITLE_ID_LIST_5" ||
               theKey == "SAVE_DATA_TRANSFER_TITLE_ID_LIST_6" ||
               theKey == "SAVE_DATA_TRANSFER_TITLE_ID_LIST_7") {
        len = 512;
    } else if (theKey == "PARAMS" || theKey == "DETAIL") {
        len = 1024;
    }
    return len;
}


void SFOManager::expandDataTable(uint32_t theOffset, uint32_t theAdditionalSize) {
    myDataTable.size += theAdditionalSize;
    myDataTable.content = cast_realloc<char>(myDataTable.content, myDataTable.size);
    // Move higher indexed data to make room for new data
    for (uint32_t i = myDataTable.size - 1; i >= theOffset + theAdditionalSize; i--) {
        myDataTable.content[i] = myDataTable.content[i - theAdditionalSize];
    }
    // Set new memory to zero
    memset(&myDataTable.content[theOffset], 0, theAdditionalSize);
}


void SFOManager::padTable(table* theTable) {
    // Remove all trailing zeros
    while (theTable->size > 0 && theTable->content[theTable->size - 1] == '\0') {
        theTable->size--;
    }
    // Re-add 1 zero if there are strings left
    if (theTable->size) theTable->size++;


    char* temp = cast_realloc<char>(theTable->content, theTable->size);
    if (temp == nullptr) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    theTable->content = temp;

    // Pad table with zeros
    while (theTable->size % 4) {
        theTable->size++;
        temp = cast_realloc<char>(theTable->content, theTable->size);
        if (temp == nullptr) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
        theTable->content = temp;
        theTable->content[theTable->size - 1] = '\0';
    }
}


void SFOManager::cleanExit() {
    if (myEntries) free(myEntries);
    if (myKeyTable.content) free(myKeyTable.content);
    if (myDataTable.content) free(myDataTable.content);
    if (myFile) fclose(myFile);
}