/*
 * Modified by Jerrin Shirks, 6/20/2024
 * made with ChatGPT :sunglasses:
 *
 * Reads a file to print or modify its SFO parameters.
 * Supported file types:
 *   - PS4 param.sfo (print and modify)
 *   - PS4 disc param.sfo (print only)
 *   - PS4 PKG (print only)
 * Made with info from https://www.psdevwiki.com/ps4/Param.sfo.
 * Get updates and Windows binaries at https://github.com/hippie68/sfo.
 */

#pragma once

#include <string>
#include <vector>
#include <cstdint>


class SFOManager {
public:
    explicit SFOManager(std::string filePath)
        : filePath(std::move(filePath)), file(nullptr), entries(nullptr) {
        loadFile();
    }

    ~SFOManager() {
        cleanExit();
    }

    class Attribute {
    public:
        std::string key;
        std::string value;

        [[nodiscard]] std::string toString() const {
            return key + ": " + value;
        }

    };

    [[nodiscard]] std::vector<Attribute> getAttributes() const;

    struct index_table_entry {
        uint16_t key_offset;
        uint16_t param_fmt;
        uint32_t param_len;
        uint32_t param_max_len;
        uint32_t data_offset;
    };

    void addParam(const std::string& type, const std::string& key, const std::string& value);

    void deleteParam(const std::string& key);

    void editParam(const std::string& key, const std::string& value);


    // TODO: this code is slow
    [[nodiscard]] std::string getAttribute(const std::string& theKey) const;

    void saveToFile(const std::string& outputPath);

private:
    std::string filePath;
    FILE* file;
    struct index_table_entry* entries;
    struct table {
        unsigned int size;
        char* content;
    } key_table{}, data_table{};

    struct header {
        uint32_t magic;
        uint32_t version;
        uint32_t key_table_offset;
        uint32_t data_table_offset;
        uint32_t entries_count;

        header() : magic(0), version(0), key_table_offset(0), data_table_offset(0), entries_count(0) {
            (void)(magic);
            (void)(version);
        }
    } header;

    void loadFile();

    long int getPS4PkgOffset();

    void loadHeader();

    void loadEntries();

    void loadKeyTable();

    void loadDataTable();

    int getIndex(const std::string& key);

    static int getReservedStringLen(const std::string& key);

    void expandDataTable(int offset, int additional_size);

    static void padTable(struct table* table);

    void cleanExit();
};


