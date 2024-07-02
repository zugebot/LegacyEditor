/*
 * Modified by Jerrin Shirks, 6/20/2024
 * made with ChatGPT :sunglasses:
 *
 * Old Details:
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
    explicit SFOManager(std::string theFilePath)
        : myFilePath(std::move(theFilePath)), myFile(nullptr), myEntries(nullptr) {
        loadFile();
    }

    ~SFOManager() {
        cleanExit();
    }

    class Attribute {
    public:
        std::string myKey;
        std::string myValue;

        [[maybe_unused]] [[nodiscard]] std::string toString() const {
            return myKey + ": " + myValue;
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

    [[maybe_unused]] void addParam(const std::string& theType, const std::string& theKey, const std::string& value);

    [[maybe_unused]] void deleteParam(const std::string& theKey);

    [[maybe_unused]] void editParam(const std::string& theKey, const std::string& value);

    // TODO: this code is slow
    [[nodiscard]] std::string getAttribute(const std::string& theKey) const;

    [[maybe_unused]] void saveToFile(const std::string& outputPath);

private:
    std::string myFilePath;
    FILE* myFile;
    struct index_table_entry* myEntries;
    struct table {
        unsigned int size;
        char* content;
    } myKeyTable{}, myDataTable{};

    struct header {
        uint32_t myMagic;
        uint32_t myVersion;
        uint32_t myKeyTableOffset;
        uint32_t myDataTableOffset;
        uint32_t myEntriesCount;

        header() : myMagic(0), myVersion(0), myKeyTableOffset(0), myDataTableOffset(0), myEntriesCount(0) {
            (void)(myMagic);
            (void)(myVersion);
        }
    } myHeader;

    void loadFile();

    long int getPS4PkgOffset();

    void loadHeader();

    void loadEntries();

    void loadKeyTable();

    void loadDataTable();

    int getIndex(const std::string& theKey);

    static int getReservedStringLen(const std::string& theKey);

    void expandDataTable(int theOffset, int theAdditionalSize);

    static void padTable(struct table* theTable);

    void cleanExit();
};


