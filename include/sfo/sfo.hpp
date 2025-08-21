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

#include <cstdint>
#include <iomanip>
#include <string>
#include <vector>
#include <variant>



enum class eSFO_FMT : uint16_t {
    /// utf8 Special Mode, NOT NULL terminated
    UTF8_SPECIAL  = 4,
    /// utf8 char string, NULL terminated
    UTF8_NORMAL = 516,
    /// integer 32 bits unsigned
    INT        = 1028,
};


enum class eSFO_MAGIC : uint32_t {
    /// none
    NONE = 0,
    /// PS4 PKG file
    PS4_PKG = 1414415231,
    /// Disc param.sfo
    PS3_DISC = 1128612691,
    /// Param.sfo file
    PS3_HDD = 1179865088
};


extern std::string hexDumpToString(std::vector<std::uint8_t> const& data);


class SFOManager {
public:
    explicit SFOManager(std::string theFilePath);

    SFOManager();

    ~SFOManager() {
        cleanExit();
    }

    using AttributeValue_t = std::variant<std::string, int, std::vector<uint8_t>>;

    struct Attribute {
        eSFO_FMT         myFmt{};
        std::string      myKey;
        AttributeValue_t myValue;

        [[nodiscard]] std::string toString() const {
            using enum eSFO_FMT;
            switch (myFmt) {
                case UTF8_NORMAL:
                    return myKey + ": " + std::get<std::string>(myValue);
                case INT:
                    return myKey + ": " + std::to_string(std::get<int>(myValue));
                case UTF8_SPECIAL:
                    return myKey + ":\n" + hexDumpToString(std::get<std::vector<uint8_t>>(myValue));
            }
            return {};
        }
    };

    [[nodiscard]] std::vector<Attribute> getAttributes() const;

    struct index_table_entry {
        uint16_t key_offset;
        eSFO_FMT param_fmt;
        uint32_t param_len;
        uint32_t param_max_len;
        uint32_t data_offset;
    };

    static constexpr uint32_t INDEX_TABLE_ENTRY_SIZE = 16;

    [[maybe_unused]] void addParam(const eSFO_FMT& theType, const std::string& theKey, const std::string& value);

    [[maybe_unused]] void deleteParam(const std::string& theKey);

    [[maybe_unused]] void editParam(const std::string& theKey, const std::string& value);

    [[nodiscard]] std::optional<Attribute> getAttribute(const std::string& theKey) const;

    [[nodiscard]] std::optional<std::string> getStringAttribute(const std::string& theKey) const;

    [[nodiscard]] std::optional<int> getIntAttribute(const std::string& theKey) const;

    [[nodiscard]] std::optional<std::vector<uint8_t>> getRawAttribute(const std::string& theKey) const;

    [[maybe_unused]] void setMagic(eSFO_MAGIC magic);

    [[maybe_unused]] void saveToFile(const std::string& outputPath);

    void setLexicographic(bool enabled) { m_lexicographic = enabled; }

private:
    std::string m_filePath;
    FILE* myFile;
    bool m_lexicographic = true;
    struct index_table_entry* myEntries;
    struct table {
        unsigned int size;
        char* content;
    } myKeyTable{}, myDataTable{};

    struct header {
        eSFO_MAGIC myMagic;
        uint32_t myVersion;
        uint32_t myKeyTableOffset;
        uint32_t myDataTableOffset;
        uint32_t myEntriesCount;

        header() : myMagic(eSFO_MAGIC::NONE), myVersion(0), myKeyTableOffset(0), myDataTableOffset(0), myEntriesCount(0) {
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


