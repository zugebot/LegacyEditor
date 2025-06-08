#pragma once
/*
#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include <istream>


//------------------------------------------------ crypto forward declarations ----------------
class AES128;

//------------------------------------------------ ParamPFD ---------------------------------------------
class ParamPFD {
public:
    // -------------------------------------------------------------------------
    // Entry: represents one child file record
    // -------------------------------------------------------------------------
    struct Entry {
        uint64_t                                   next;
        std::string                                name;
        std::array<uint8_t, 7>                     pad0;
        std::array<uint8_t, 64>                    encKey;
        std::array<std::array<uint8_t,20>, 4>      hashes;
        std::array<uint8_t, 40>                    pad1;
        uint64_t                                   size;

        // build raw 272‑byte entry blob (same as C# EntryData)
        std::vector<uint8_t> raw() const;
        // hashing payload (same as C# HashData)
        std::vector<uint8_t> hashData() const;
    };

    // -------------------------------------------------------------------------
    // Constructors
    // -------------------------------------------------------------------------
    ParamPFD();
    explicit ParamPFD(const std::string& filepath);
    explicit ParamPFD(const std::vector<uint8_t>& buf);
    explicit ParamPFD(std::istream& input);

    // -------------------------------------------------------------------------
    // Public properties (console/session keys)
    // -------------------------------------------------------------------------
    std::array<uint8_t, 32> consoleID;
    std::array<uint8_t,  8> userID;
    std::array<uint8_t, 16> discHashKey;
    std::array<uint8_t,  8> authID;
    std::array<uint8_t, 16> secureID;

    // read‑only access to the loaded entries
    const std::vector<Entry>& entries() const;

    // -------------------------------------------------------------------------
    // High‑level API
    // -------------------------------------------------------------------------
    bool validateAll   (const std::string& root, bool fix);
    bool decryptFile   (const std::string& root, const std::string& name, std::vector<uint8_t>& out);
    bool encryptFile   (const std::string& root, const std::string& name, const std::vector<uint8_t>& plain);
    int  decryptAll    (const std::string& root);
    int  encryptAll    (const std::string& root);
    bool rebuild       (const std::string& root, bool encrypt);
    std::vector<uint8_t> serialize();

private:
    // -------------------------------------------------------------------------
    // Internal/Private methods
    // -------------------------------------------------------------------------
    void load           (const std::vector<uint8_t>& buf);
    uint64_t xyIndex    (const std::string& n) const;
    std::array<uint8_t,20> entryCID        (size_t idx) const;
    std::array<uint8_t,20> defaultHash     () const;
    std::array<uint8_t,64> decryptEntryKey (const Entry& e) const;

    // -------------------------------------------------------------------------
    // Raw PARAM.PFD state (mirrors original C# fields)
    // -------------------------------------------------------------------------
    uint64_t               version;
    std::array<uint8_t,16> headerIV;
    std::array<uint8_t,20> bottomHash;
    std::array<uint8_t,20> topHash;
    std::array<uint8_t,20> fileKey;
    std::array<uint8_t, 4> pad;
    uint64_t               xyCap;
    uint64_t               pfReserved;
    uint64_t               pfUsed;
    std::vector<uint64_t>  xTable;
    std::vector<Entry>     entries_;
    std::vector<uint8_t>   space;
    std::vector<std::array<uint8_t,20>> yTable;
    std::array<uint8_t,44> pad2;
    std::array<uint8_t,20> realKey;
};
*/