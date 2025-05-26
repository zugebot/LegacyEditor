/*
 * Modified by Jerrin Shirks, 6/20/2024
 * Updated by Jerrin Shirks, 8/1/2024
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


enum eSFO_FMT : uint32_t {
    /// utf8 Special Mode, NOT NULL terminated
    UTF8_SPECIAL  = 4,
    /// utf8 char string, NULL terminated
    UTF8_NORMAL = 516,
    /// integer 32 bits unsigned
    INT        = 1028,
};


enum eSFO_MAGIC {
    /// PS4 PKG file
    PS4_PKG = 1414415231,
    /// Disc param.sfo
    PS3_DISC = 1128612691,
    /// Param.sfo file
    PS3_HDD = 1179865088
};


class SFOManager {
public:
    explicit SFOManager(std::string theFilePath);

    SFOManager();


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

    static constexpr uint32_t INDEX_TABLE_ENTRY_SIZE = 16;

    [[maybe_unused]] void addParam(const eSFO_FMT& theType, const std::string& theKey, const std::string& value);

    [[maybe_unused]] void deleteParam(const std::string& theKey);

    [[maybe_unused]] void editParam(const std::string& theKey, const std::string& value);

    [[nodiscard]] std::string getAttribute(const std::string& theKey) const;

    [[maybe_unused]] void setMagic(eSFO_MAGIC magic);

    [[maybe_unused]] void saveToFile(const std::string& outputPath);

private:
    std::string m_filePath;
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

    [[nodiscard]] uint32_t getTableSize() const;

    static uint32_t getReservedStringLen(const std::string& theKey);

    void expandDataTable(uint32_t theOffset, uint32_t theAdditionalSize);

    static void padTable(struct table* theTable);

    void cleanExit();
};


