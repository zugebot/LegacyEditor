#include "sfo.hpp"


#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>


std::vector<SFOManager::Attribute> SFOManager::getAttributes() const {
    std::vector<Attribute> attributes;
    for (int i = 0; i < header.entries_count; i++) {
        Attribute attr;
        attr.key = &key_table.content[entries[i].key_offset];
        switch (entries[i].param_fmt) {
            case 516:
            case 1024:
                attr.value = &data_table.content[entries[i].data_offset];
                break;
            case 1028: {
                auto* integer = (uint32_t*)&data_table.content[entries[i].data_offset];
                attr.value = std::to_string(*integer);
                break;
            }
        }
        attributes.push_back(attr);
    }
    return attributes;
}




void SFOManager::addParam(const std::string& type, const std::string& key, const std::string& value) {
    struct index_table_entry new_entry = {0};
    int new_index = 0;

    // Get new entry's .param_len and .param_max_len
    if (type == "str") {
        new_entry.param_fmt = 516;
        new_entry.param_max_len = getReservedStringLen(key);
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
    for (int i = 0; i < header.entries_count; i++) {
        int result = key.compare(&key_table.content[entries[i].key_offset]);
        if (result == 0) { // Parameter already exists
            throw std::runtime_error("Could not add \"" + key + "\": parameter already exists.");
        } else if (result < 0) {
            new_index = i;
            new_entry.key_offset = entries[i].key_offset;
            new_entry.data_offset = entries[i].data_offset;
            break;
        } else if (i == header.entries_count - 1) {
            new_index = i + 1;
            new_entry.key_offset = entries[i].key_offset +
                                   strlen(&key_table.content[entries[i].key_offset]) + 1;
            new_entry.data_offset = entries[i].data_offset + entries[i].param_max_len;
            break;
        }
    }

    // Make room for the new index table entry by moving the old ones
    header.entries_count++;
    entries = static_cast<index_table_entry*>(
            realloc(entries, sizeof(struct index_table_entry) * header.entries_count));
    for (int i = (int)header.entries_count - 1; i > new_index; i--) {
        entries[i] = entries[i - 1];
        entries[i].key_offset += key.length() + 1;
        entries[i].data_offset += new_entry.param_max_len;
    }

    // Insert new index table entry
    memcpy(&entries[new_index], &new_entry, sizeof(struct index_table_entry));

    // Resize key table
    key_table.size += key.length() + 1;
    key_table.content = static_cast<char*>(realloc(key_table.content, key_table.size));
    // Move higher indexed keys to make room for new key
    for (int i = (int)key_table.size - 1; i > new_entry.key_offset + key.length(); i--) {
        key_table.content[i] = key_table.content[i - key.length() - 1];
    }
    // Insert new key
    memcpy(&key_table.content[new_entry.key_offset], key.c_str(), key.length() + 1);
    padTable(&key_table);

    // Resize data table
    expandDataTable((int)new_entry.data_offset, (int)new_entry.param_max_len);

    // Insert new data
    if (type == "str") {
        memset(&data_table.content[entries[new_index].data_offset],
               0, new_entry.param_len); // Overwrite whole space with zeros first
        memcpy(&data_table.content[entries[new_index].data_offset],
               value.c_str(), value.length() + 1); // Then copy new value
    } else if (type == "int") {
        uint32_t new_value = strtoul(value.c_str(), nullptr, 0);
        memcpy(&data_table.content[entries[new_index].data_offset], &new_value, 4);
    }
}


void SFOManager::deleteParam(const std::string& key) {
    int index = getIndex(key);
    if (index < 0) { // Parameter not found
        throw std::runtime_error("Could not delete \"" + key + "\": parameter not found.");
    }

    // Delete parameter from key table
    for (int i = entries[index].key_offset; i < key_table.size - key.length() - 1; i++) {
        key_table.content[i] = key_table.content[i + key.length() + 1];
    }

    // Resize key table
    key_table.size -= key.length() + 1;
    key_table.content = static_cast<char*>(realloc(key_table.content, key_table.size));
    padTable(&key_table);

    // Delete parameter from data table
    for (int i = (int)entries[index].data_offset; i < data_table.size - entries[index].param_max_len; i++) {
        data_table.content[i] = data_table.content[i + entries[index].param_max_len];
    }

    // Resize data table
    data_table.size -= entries[index].param_max_len;
    if (data_table.size) {
        data_table.content = static_cast<char*>(realloc(data_table.content, data_table.size));
    } else {
        free(data_table.content);
        data_table.content = nullptr;
    }

    // Delete parameter from index table
    int param_max_len = (int)entries[index].param_max_len;
    for (int i = index; i < header.entries_count - 1; i++) {
        entries[i] = entries[i + 1];
        entries[i].key_offset -= key.length() + 1;
        entries[i].data_offset -= param_max_len;
    }

    // Resize index table
    header.entries_count--;
    if (header.entries_count) {
        entries = static_cast<index_table_entry*>(
                realloc(entries, sizeof(struct index_table_entry) * header.entries_count));
    } else {
        free(entries);
        entries = nullptr;
    }
}


void SFOManager::editParam(const std::string& key, const std::string& value) {
    int index = getIndex(key);
    if (index < 0) { // Parameter not found
        throw std::runtime_error("Could not edit \"" + key + "\": parameter not found.");
    }

    switch (entries[index].param_fmt) {
        case 516: // String
        case 1024: { // Special mode string
            entries[index].param_len = value.length() + 1;
            // Enlarge data table if new string is longer than allowed
            int diff = (int)(entries[index].param_len - entries[index].param_max_len);
            if (diff > 0) {
                int offset = (int)(entries[index].data_offset + entries[index].param_max_len);
                entries[index].param_max_len = entries[index].param_len;

                // 4-byte alignment
                while (entries[index].param_max_len % 4) {
                    entries[index].param_max_len++;
                    diff++;
                }

                expandDataTable(offset, diff);

                // Adjust follow-up index table entries' data offsets
                for (int i = index + 1; i < header.entries_count; i++) {
                    entries[i].data_offset += diff;
                }
            }
            // Overwrite old data with zeros
            memset(&data_table.content[entries[index].data_offset], 0, entries[index].param_max_len);
            // Save new string to data table
            snprintf(&data_table.content[entries[index].data_offset], entries[index].param_max_len, "%s", value.c_str());
            break;
        }
        case 1028: { // Integer
            uint32_t integer = strtoul(value.c_str(), nullptr, 0);
            memcpy(&data_table.content[entries[index].data_offset], &integer, 4);
            break;
        }
    }
}


std::string SFOManager::getAttribute(const std::string& theKey) const {
    auto attrs = getAttributes();
    for (const auto& attr : attrs) {
        if (attr.key == theKey) {
            return attr.value;
        }
    }
    return "";
}


void SFOManager::saveToFile(const std::string& outputPath) {
    FILE* outFile = fopen(outputPath.c_str(), "wb");
    if (outFile == nullptr) {
        throw std::runtime_error("Could not open file \"" + outputPath + "\" in write mode.");
    }

    // Adjust header's table offsets before saving
    header.key_table_offset = sizeof(struct header) + sizeof(struct index_table_entry) * header.entries_count;
    header.data_table_offset = header.key_table_offset + key_table.size;

    if (fwrite(&header, sizeof(struct header), 1, outFile) != 1) {
        throw std::runtime_error("Could not write header to file \"" + outputPath + "\".");
    }
    if (header.entries_count && fwrite(entries, sizeof(struct index_table_entry) * header.entries_count, 1, outFile) != 1) {
        throw std::runtime_error("Could not write index table to file \"" + outputPath + "\".");
    }
    if (key_table.size && fwrite(key_table.content, key_table.size, 1, outFile) != 1) {
        throw std::runtime_error("Could not write key table to file \"" + outputPath + "\".");
    }
    if (data_table.size && fwrite(data_table.content, data_table.size, 1, outFile) != 1) {
        throw std::runtime_error("Could not write data table to file \"" + outputPath + "\".");
    }

    fclose(outFile);
}


static uint32_t bswap_32(uint32_t val) {
    val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0x00FF00FF);
    return (val << 16) | (val >> 16);
}


void SFOManager::loadFile() {
    file = fopen(filePath.c_str(), "rb");
    if (!file) {
        throw std::runtime_error("Could not open file \"" + filePath + "\".");
    }

    uint32_t magic;
    fread(&magic, 4, 1, file);
    if (magic == 1414415231) { // PS4 PKG file
        fseek(file, getPS4PkgOffset(), SEEK_SET);
    } else if (magic == 1128612691) { // Disc param.sfo
        fseek(file, 0x800, SEEK_SET);
    } else if (magic == 1179865088) { // Param.sfo file
        rewind(file);
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
    fseek(file, 0x00C, SEEK_SET);
    fread(&pkg_file_count, 4, 1, file);
    fseek(file, 0x018, SEEK_SET);
    fread(&pkg_table_offset, 4, 1, file);
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
    } pkg_table_entry[pkg_file_count];
    fseek(file, (long)pkg_table_offset, SEEK_SET);
    fread(pkg_table_entry, sizeof(struct pkg_table_entry), pkg_file_count, file);
    for (int i = 0; i < pkg_file_count; i++) {
        if (pkg_table_entry[i].id == 1048576) {// param.sfo ID
            return (long)bswap_32(pkg_table_entry[i].offset);
        }
    }
    throw std::runtime_error("Could not find a param.sfo file inside the PS4 PKG.");
}


void SFOManager::loadHeader() {
    if (fread(&header, sizeof(struct header), 1, file) != 1) {
        throw std::runtime_error("Could not read header.");
    }
}


void SFOManager::loadEntries() {
    unsigned int size = sizeof(struct index_table_entry) * header.entries_count;
    entries = static_cast<index_table_entry*>(malloc(size));
    if (entries == nullptr) {
        throw std::runtime_error("Could not allocate memory for index table.");
    }
    if (size && fread(entries, size, 1, file) != 1) {
        throw std::runtime_error("Could not read index table entries.");
    }
}


void SFOManager::loadKeyTable() {
    key_table.size = header.data_table_offset - header.key_table_offset;
    key_table.content = static_cast<char*>(malloc(key_table.size));
    if (key_table.content == nullptr) {
        throw std::runtime_error("Could not allocate memory for key table.");
    }
    if (key_table.size && fread(key_table.content, key_table.size, 1, file) != 1) {
        throw std::runtime_error("Could not read key table.");
    }
}


void SFOManager::loadDataTable() {
    if (header.entries_count) {
        data_table.size = entries[header.entries_count - 1].data_offset + entries[header.entries_count - 1].param_max_len;
    } else {
        data_table.size = 0; // For newly created, empty param.sfo files
    }
    data_table.content = static_cast<char*>(malloc(data_table.size));
    if (data_table.content == nullptr) {
        throw std::runtime_error("Could not allocate memory for data table.");
    }
    if (data_table.size && fread(data_table.content, data_table.size, 1, file) != 1) {
        throw std::runtime_error("Could not read data table.");
    }
}


int SFOManager::getIndex(const std::string& key) {
    for (int i = 0; i < header.entries_count; i++) {
        if (key == &key_table.content[entries[i].key_offset]) {
            return i;
        }
    }
    return -1;
}


int SFOManager::getReservedStringLen(const std::string& key) {
    int len = 0;
    if (key == "CATEGORY" || key == "FORMAT") {
        len = 4;
    } else if (key == "APP_VER" || key == "CONTENT_VER" || key == "VERSION") {
        len = 8;
    } else if (key == "INSTALL_DIR_SAVEDATA" || key == "TITLE_ID") {
        len = 12;
    } else if (key == "SERVICE_ID_ADDCONT_ADD_1" ||
               key == "SERVICE_ID_ADDCONT_ADD_2" ||
               key == "SERVICE_ID_ADDCONT_ADD_3" ||
               key == "SERVICE_ID_ADDCONT_ADD_4" ||
               key == "SERVICE_ID_ADDCONT_ADD_5" ||
               key == "SERVICE_ID_ADDCONT_ADD_6" ||
               key == "SERVICE_ID_ADDCONT_ADD_7") {
        len = 20;
    } else if (key == "CONTENT_ID") {
        len = 48;
    } else if (key == "PROVIDER" || key == "TITLE" ||
               key == "PROVIDER_00" || key == "TITLE_00" ||
               key == "PROVIDER_01" || key == "TITLE_01" ||
               key == "PROVIDER_02" || key == "TITLE_02" ||
               key == "PROVIDER_03" || key == "TITLE_03" ||
               key == "PROVIDER_04" || key == "TITLE_04" ||
               key == "PROVIDER_05" || key == "TITLE_05" ||
               key == "PROVIDER_06" || key == "TITLE_06" ||
               key == "PROVIDER_07" || key == "TITLE_07" ||
               key == "PROVIDER_08" || key == "TITLE_08" ||
               key == "PROVIDER_09" || key == "TITLE_09" ||
               key == "PROVIDER_10" || key == "TITLE_10" ||
               key == "PROVIDER_11" || key == "TITLE_11" ||
               key == "PROVIDER_12" || key == "TITLE_12" ||
               key == "PROVIDER_13" || key == "TITLE_13" ||
               key == "PROVIDER_14" || key == "TITLE_14" ||
               key == "PROVIDER_15" || key == "TITLE_15" ||
               key == "PROVIDER_16" || key == "TITLE_16" ||
               key == "PROVIDER_17" || key == "TITLE_17" ||
               key == "PROVIDER_18" || key == "TITLE_18" ||
               key == "PROVIDER_19" || key == "TITLE_19" ||
               key == "PROVIDER_20" || key == "TITLE_20" ||
               key == "TITLE_21" || key == "TITLE_22" ||
               key == "TITLE_23" || key == "TITLE_24" ||
               key == "TITLE_25" || key == "TITLE_26" ||
               key == "TITLE_27" || key == "TITLE_28" ||
               key == "TITLE_29") {
        len = 128;
    } else if (key == "PUBTOOLINFO" || key == "PS3_TITLE_ID_LIST_FOR_BOOT" ||
               key == "SAVE_DATA_TRANSFER_TITLE_ID_LIST_1" ||
               key == "SAVE_DATA_TRANSFER_TITLE_ID_LIST_2" ||
               key == "SAVE_DATA_TRANSFER_TITLE_ID_LIST_3" ||
               key == "SAVE_DATA_TRANSFER_TITLE_ID_LIST_4" ||
               key == "SAVE_DATA_TRANSFER_TITLE_ID_LIST_5" ||
               key == "SAVE_DATA_TRANSFER_TITLE_ID_LIST_6" ||
               key == "SAVE_DATA_TRANSFER_TITLE_ID_LIST_7") {
        len = 512;
    }
    return len;
}


void SFOManager::expandDataTable(int offset, int additional_size) {
    data_table.size += additional_size;
    data_table.content = static_cast<char*>(realloc(data_table.content, data_table.size));
    // Move higher indexed data to make room for new data
    for (int i = (int)data_table.size - 1; i >= offset + additional_size; i--) {
        data_table.content[i] = data_table.content[i - additional_size];
    }
    // Set new memory to zero
    memset(&data_table.content[offset], 0, additional_size);
}


void SFOManager::padTable(struct table* table) {
    // Remove all trailing zeros
    while (table->size > 0 && table->content[table->size - 1] == '\0') {
        table->size--;
    }
    if (table->size) table->size++;// Re-add 1 zero if there are strings left

    table->content = static_cast<char*>(realloc(table->content, table->size));
    // Pad table with zeros
    while (table->size % 4) {
        table->size++;
        table->content = static_cast<char*>(realloc(table->content, table->size));
        table->content[table->size - 1] = '\0';
    }
}


void SFOManager::cleanExit() {
    if (entries) free(entries);
    if (key_table.content) free(key_table.content);
    if (data_table.content) free(data_table.content);
    if (file) fclose(file);
}




