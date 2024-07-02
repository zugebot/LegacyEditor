#include "sfo.hpp"

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>


std::vector<SFOManager::Attribute> SFOManager::getAttributes() const {
    std::vector<Attribute> attributes;
    for (uint32_t i = 0; i < myHeader.myEntriesCount; i++) {
        Attribute attr;
        attr.myKey = &myKeyTable.content[myEntries[i].key_offset];
        switch (myEntries[i].param_fmt) {
            case 516:
            case 1024:
                attr.myValue = &myDataTable.content[myEntries[i].data_offset];
                break;
            case 1028: {
                auto* integer = (uint32_t*)&myDataTable.content[myEntries[i].data_offset];
                attr.myValue = std::to_string(*integer);
                break;
            }
        }
        attributes.push_back(attr);
    }
    return attributes;
}


[[maybe_unused]] void SFOManager::addParam(const std::string& theType, const std::string& theKey, const std::string& value) {
    struct index_table_entry new_entry = {};
    uint32_t new_index = 0;

    // Get new entry's .param_len and .param_max_len
    if (theType == "str") {
        new_entry.param_fmt = 516;
        new_entry.param_max_len = getReservedStringLen(theKey);
        new_entry.param_len = value.length() + 1;
        if (new_entry.param_max_len < new_entry.param_len) {
            new_entry.param_max_len = new_entry.param_len;
            // 4-byte alignment
            while (new_entry.param_max_len % 4) {
                new_entry.param_max_len++;
            }
        }
    } else { // "int"
        new_entry.param_fmt = 1028;
        new_entry.param_len = 4;
        new_entry.param_max_len = 4;
    }

    // Get new entry's index and offsets
    for (uint32_t i = 0; i < myHeader.myEntriesCount; i++) {
        int result = theKey.compare(&myKeyTable.content[myEntries[i].key_offset]);
        if (result == 0) { // Parameter already exists
            throw std::runtime_error("Could not add \"" + theKey + "\": parameter already exists.");
        } else if (result < 0) {
            new_index = i;
            new_entry.key_offset = myEntries[i].key_offset;
            new_entry.data_offset = myEntries[i].data_offset;
            break;
        } else if (i == myHeader.myEntriesCount - 1) {
            new_index = i + 1;
            new_entry.key_offset = myEntries[i].key_offset +
                                   strlen(&myKeyTable.content[myEntries[i].key_offset]) + 1;
            new_entry.data_offset = myEntries[i].data_offset + myEntries[i].param_max_len;
            break;
        }
    }

    // Make room for the new index table entry by moving the old ones
    myHeader.myEntriesCount++;
    myEntries = static_cast<index_table_entry*>(
            realloc(myEntries, sizeof(struct index_table_entry) * myHeader.myEntriesCount));
    for (int i = (int) myHeader.myEntriesCount - 1; i > static_cast<int>(new_index); i--) {
        myEntries[i] = myEntries[i - 1];
        myEntries[i].key_offset += theKey.length() + 1;
        myEntries[i].data_offset += new_entry.param_max_len;
    }

    // Insert new index table entry
    std::memcpy(&myEntries[new_index], &new_entry, sizeof(struct index_table_entry));

    // Resize key table
    myKeyTable.size += theKey.length() + 1;
    myKeyTable.content = static_cast<char*>(realloc(myKeyTable.content, myKeyTable.size));
    // Move higher indexed keys to make room for new key
    for (int i = (int) myKeyTable.size - 1; i > static_cast<int>(new_entry.key_offset + theKey.length()); i--) {
        myKeyTable.content[i] = myKeyTable.content[i - theKey.length() - 1];
    }
    // Insert new key
    std::memcpy(&myKeyTable.content[new_entry.key_offset], theKey.c_str(), theKey.length() + 1);
    padTable(&myKeyTable);

    // Resize data table
    expandDataTable((int)new_entry.data_offset, (int)new_entry.param_max_len);

    // Insert new data
    if (theType == "str") {
        memset(&myDataTable.content[myEntries[new_index].data_offset],
               0, new_entry.param_len); // Overwrite whole space with zeros first
        std::memcpy(&myDataTable.content[myEntries[new_index].data_offset],
                    value.c_str(), value.length() + 1); // Then copy new value
    } else if (theType == "int") {
        uint32_t new_value = strtoul(value.c_str(), nullptr, 0);
        std::memcpy(&myDataTable.content[myEntries[new_index].data_offset], &new_value, 4);
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
    myKeyTable.content = static_cast<char*>(realloc(myKeyTable.content, myKeyTable.size));
    padTable(&myKeyTable);

    // Delete parameter from data table
    for (int i = (int) myEntries[index].data_offset; i < static_cast<int>(myDataTable.size - myEntries[index].param_max_len); i++) {
        myDataTable.content[i] = myDataTable.content[i + myEntries[index].param_max_len];
    }

    // Resize data table
    myDataTable.size -= myEntries[index].param_max_len;
    if (myDataTable.size) {
        myDataTable.content = static_cast<char*>(realloc(myDataTable.content, myDataTable.size));
    } else {
        free(myDataTable.content);
        myDataTable.content = nullptr;
    }

    // Delete parameter from index table
    int param_max_len = (int) myEntries[index].param_max_len;
    for (uint32_t i = index; i < myHeader.myEntriesCount - 1; i++) {
        myEntries[i] = myEntries[i + 1];
        myEntries[i].key_offset -= theKey.length() + 1;
        myEntries[i].data_offset -= param_max_len;
    }

    // Resize index table
    myHeader.myEntriesCount--;
    if (myHeader.myEntriesCount) {
        myEntries = static_cast<index_table_entry*>(
                realloc(myEntries, sizeof(struct index_table_entry) * myHeader.myEntriesCount));
    } else {
        free(myEntries);
        myEntries = nullptr;
    }
}


[[maybe_unused]] void SFOManager::editParam(const std::string& theKey, const std::string& theValue) {
    int index = getIndex(theKey);
    if (index < 0) { // Parameter not found
        throw std::runtime_error("Could not edit \"" + theKey + "\": parameter not found.");
    }

    switch (myEntries[index].param_fmt) {
        case 516: // String
        case 1024: { // Special mode string
            myEntries[index].param_len = theValue.length() + 1;
            // Enlarge data table if new string is longer than allowed
            int diff = (int)(myEntries[index].param_len - myEntries[index].param_max_len);
            if (diff > 0) {
                int offset = (int)(myEntries[index].data_offset + myEntries[index].param_max_len);
                myEntries[index].param_max_len = myEntries[index].param_len;

                // 4-byte alignment
                while (myEntries[index].param_max_len % 4) {
                    myEntries[index].param_max_len++;
                    diff++;
                }

                expandDataTable(offset, diff);

                // Adjust follow-up index table entries' data offsets
                for (uint32_t i = index + 1; i < myHeader.myEntriesCount; i++) {
                    myEntries[i].data_offset += diff;
                }
            }
            // Overwrite old data with zeros
            memset(&myDataTable.content[myEntries[index].data_offset], 0, myEntries[index].param_max_len);
            // Save new string to data table
            snprintf(&myDataTable.content[myEntries[index].data_offset], myEntries[index].param_max_len, "%s", theValue.c_str());
            break;
        }
        case 1028: { // Integer
            uint32_t integer = strtoul(theValue.c_str(), nullptr, 0);
            std::memcpy(&myDataTable.content[myEntries[index].data_offset], &integer, 4);
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


[[maybe_unused]] void SFOManager::saveToFile(const std::string& outputPath) {
    FILE* outFile = fopen(outputPath.c_str(), "wb");
    if (outFile == nullptr) {
        throw std::runtime_error("Could not open file \"" + outputPath + "\" in write mode.");
    }

    // Adjust header's table offsets before saving
    myHeader.myKeyTableOffset = sizeof(struct header) + sizeof(struct index_table_entry) * myHeader.myEntriesCount;
    myHeader.myDataTableOffset = myHeader.myKeyTableOffset + myKeyTable.size;

    if (fwrite(&myHeader, sizeof(struct header), 1, outFile) != 1) {
        throw std::runtime_error("Could not write header to file \"" + outputPath + "\".");
    }
    if (myHeader.myEntriesCount && fwrite(myEntries, sizeof(struct index_table_entry) * myHeader.myEntriesCount, 1, outFile) != 1) {
        throw std::runtime_error("Could not write index table to file \"" + outputPath + "\".");
    }
    if (myKeyTable.size && fwrite(myKeyTable.content, myKeyTable.size, 1, outFile) != 1) {
        throw std::runtime_error("Could not write key table to file \"" + outputPath + "\".");
    }
    if (myDataTable.size && fwrite(myDataTable.content, myDataTable.size, 1, outFile) != 1) {
        throw std::runtime_error("Could not write data table to file \"" + outputPath + "\".");
    }

    fclose(outFile);
}


static uint32_t bswap_32(uint32_t val) {
    val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0x00FF00FF);
    return (val << 16) | (val >> 16);
}


void SFOManager::loadFile() {
    myFile = fopen(myFilePath.c_str(), "rb");
    if (!myFile) {
        throw std::runtime_error("Could not open file \"" + myFilePath + "\".");
    }

    uint32_t magic;
    fread(&magic, 4, 1, myFile);
    if (magic == 1414415231) { // PS4 PKG file
        fseek(myFile, getPS4PkgOffset(), SEEK_SET);
    } else if (magic == 1128612691) { // Disc param.sfo
        fseek(myFile, 0x800, SEEK_SET);
    } else if (magic == 1179865088) { // Param.sfo file
        rewind(myFile);
    } else {
        throw std::runtime_error("Param.sfo magic number not found.");
    }

    loadHeader();
    loadEntries();
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

    struct pkg_table_entry {
        uint32_t id;
        uint32_t filename_offset;
        uint32_t flags1;
        uint32_t flags2;
        uint32_t offset;
        uint32_t size;
        uint64_t padding;
    };

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
    if (fread(&myHeader, sizeof(struct header), 1, myFile) != 1) {
        throw std::runtime_error("Could not read header.");
    }
}


void SFOManager::loadEntries() {
    unsigned int size = sizeof(struct index_table_entry) * myHeader.myEntriesCount;
    myEntries = static_cast<index_table_entry*>(malloc(size));
    if (myEntries == nullptr) {
        throw std::runtime_error("Could not allocate memory for index table.");
    }
    if (size && fread(myEntries, size, 1, myFile) != 1) {
        throw std::runtime_error("Could not read index table entries.");
    }
}


void SFOManager::loadKeyTable() {
    myKeyTable.size = myHeader.myDataTableOffset - myHeader.myKeyTableOffset;
    myKeyTable.content = static_cast<char*>(malloc(myKeyTable.size));
    if (myKeyTable.content == nullptr) {
        throw std::runtime_error("Could not allocate memory for key table.");
    }
    if (myKeyTable.size && fread(myKeyTable.content, myKeyTable.size, 1, myFile) != 1) {
        throw std::runtime_error("Could not read key table.");
    }
}


void SFOManager::loadDataTable() {
    if (myHeader.myEntriesCount) {
        myDataTable.size = myEntries[myHeader.myEntriesCount - 1].data_offset + myEntries[myHeader.myEntriesCount - 1].param_max_len;
    } else {
        myDataTable.size = 0; // For newly created, empty param.sfo files
    }
    myDataTable.content = static_cast<char*>(malloc(myDataTable.size));
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


int SFOManager::getReservedStringLen(const std::string& theKey) {
    int len = 0;
    if (theKey == "CATEGORY" || theKey == "FORMAT") {
        len = 4;
    } else if (theKey == "APP_VER" || theKey == "CONTENT_VER" || theKey == "VERSION") {
        len = 8;
    } else if (theKey == "INSTALL_DIR_SAVEDATA" || theKey == "TITLE_ID") {
        len = 12;
    } else if (theKey == "SERVICE_ID_ADDCONT_ADD_1" ||
               theKey == "SERVICE_ID_ADDCONT_ADD_2" ||
               theKey == "SERVICE_ID_ADDCONT_ADD_3" ||
               theKey == "SERVICE_ID_ADDCONT_ADD_4" ||
               theKey == "SERVICE_ID_ADDCONT_ADD_5" ||
               theKey == "SERVICE_ID_ADDCONT_ADD_6" ||
               theKey == "SERVICE_ID_ADDCONT_ADD_7") {
        len = 20;
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
               theKey == "TITLE_29") {
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
    }
    return len;
}


void SFOManager::expandDataTable(int theOffset, int theAdditionalSize) {
    myDataTable.size += theAdditionalSize;
    myDataTable.content = static_cast<char*>(realloc(myDataTable.content, myDataTable.size));
    // Move higher indexed data to make room for new data
    for (int i = (int) myDataTable.size - 1; i >= theOffset + theAdditionalSize; i--) {
        myDataTable.content[i] = myDataTable.content[i - theAdditionalSize];
    }
    // Set new memory to zero
    memset(&myDataTable.content[theOffset], 0, theAdditionalSize);
}


void SFOManager::padTable(struct table* theTable) {
    // Remove all trailing zeros
    while (theTable->size > 0 && theTable->content[theTable->size - 1] == '\0') {
        theTable->size--;
    }
    if (theTable->size) theTable->size++;// Re-add 1 zero if there are strings left

    theTable->content = static_cast<char*>(realloc(theTable->content, theTable->size));
    // Pad table with zeros
    while (theTable->size % 4) {
        theTable->size++;
        theTable->content = static_cast<char*>(realloc(theTable->content, theTable->size));
        theTable->content[theTable->size - 1] = '\0';
    }
}


void SFOManager::cleanExit() {
    if (myEntries) free(myEntries);
    if (myKeyTable.content) free(myKeyTable.content);
    if (myDataTable.content) free(myDataTable.content);
    if (myFile) fclose(myFile);
}