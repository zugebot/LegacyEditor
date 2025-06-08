/*
// -----------------------------------------------------------------------------
// ParamPFD – C++20 implementation closely matching Jappi88's C# Param_PFD class
// -----------------------------------------------------------------------------
// single‑translation‑unit implementation of every method declared in param_pfd.hpp
// -----------------------------------------------------------------------------
#ifdef INCLUDE_OPENSSL

#include "pfd.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <stdexcept>

// ──────────────────────────────────────────────────────────────────────────────
// tiny utility helpers (endianness, file io, alignment, hex)
// ──────────────────────────────────────────────────────────────────────────────
namespace util {
    inline uint64_t bswap64(uint64_t v) { return __builtin_bswap64(v); }
    inline uint32_t bswap32(uint32_t v) { return __builtin_bswap32(v); }

    inline uint64_t be64(const uint8_t* p) {
        uint64_t v = 0;
        for (int i = 0; i < 8; ++i) v = (v << 8) | p[i];
        return v;
    }
    inline void be64put(uint8_t* p, uint64_t v) {
        for (int i = 7; i >= 0; --i) {
            p[i] = v & 0xFF;
            v >>= 8;
        }
    }

    inline size_t align16(size_t n) { return (n + 15) & ~size_t(15); }

    inline std::vector<uint8_t> read_file(const std::string& path) {
        std::ifstream f(path, std::ios::binary);
        if (!f) throw std::runtime_error("open " + path);
        return {std::istreambuf_iterator<char>(f), {}};
    }
    inline void write_file(const std::string& path, const std::vector<uint8_t>& d) {
        std::ofstream f(path, std::ios::binary);
        if (!f) throw std::runtime_error("write " + path);
        f.write(reinterpret_cast<const char*>(d.data()), static_cast<std::streamsize>(d.size()));
    }
} // namespace util

// ──────────────────────────────────────────────────────────────────────────────
// simple AES‑128 wrapper used by the original implementation
// ──────────────────────────────────────────────────────────────────────────────
class AES128 {
    AES_KEY enc_, dec_;

public:
    explicit AES128(const std::array<uint8_t, 16>& k) {
        AES_set_encrypt_key(k.data(), 128, &enc_);
        AES_set_decrypt_key(k.data(), 128, &dec_);
    }
    std::vector<uint8_t> CBCDec(const std::vector<uint8_t>& c, const std::array<uint8_t, 16>& iv) const {
        std::vector<uint8_t> o(c.size());
        auto ivc = iv;
        AES_cbc_encrypt(c.data(), o.data(), c.size(), &dec_, ivc.data(), AES_DECRYPT);
        return o;
    }
    std::vector<uint8_t> CBCEnc(const std::vector<uint8_t>& p, const std::array<uint8_t, 16>& iv) const {
        std::vector<uint8_t> o(p.size());
        auto ivc = iv;
        AES_cbc_encrypt(p.data(), o.data(), p.size(), &enc_, ivc.data(), AES_ENCRYPT);
        return o;
    }
    void ECBEnc(const uint8_t* in, uint8_t* out) const { AES_ecb_encrypt(in, out, &enc_, AES_ENCRYPT); }
    void ECBDec(const uint8_t* in, uint8_t* out) const { AES_ecb_encrypt(in, out, &dec_, AES_DECRYPT); }
};

static constexpr std::array<uint8_t, 16> SYSCON_KEY{
        0xD4, 0x13, 0xB8, 0x96, 0x63, 0xE1, 0xFE, 0x9F, 0x75, 0x14, 0x3D, 0x3B, 0xB4, 0x56, 0x52, 0x74};
static const uint8_t KEYGEN_KEY[20] = {
        0x6B, 0x1A, 0xCE, 0xA2, 0x46, 0xB7, 0x45, 0xFD, 0x8F, 0x93,
        0x76, 0x3B, 0x92, 0x05, 0x94, 0xCD, 0x53, 0x48, 0x3B, 0x82};


static std::array<uint8_t, 20> createKeyFromSecureID(const std::array<uint8_t, 16>& secureID) {
    std::array<uint8_t, 20> key{};
    int j = 0;
    for (int i = 0; i < 20; ++i) {
        switch (i) {
            case 1: key[i] = 11; break;
            case 2: key[i] = 15; break;
            case 5: key[i] = 14; break;
            case 8: key[i] = 10; break;
            default: key[i] = secureID[j++]; break;
        }
    }
    return key;
}


static std::array<uint8_t, 20> hmacSHA1(const uint8_t* k, size_t kl, const uint8_t* d, size_t dl) {
    std::array<uint8_t, 20> o{};
    unsigned l = 0;
    HMAC(EVP_sha1(), k, static_cast<int>(kl), d, dl, o.data(), &l);
    return o;
}

// ──────────────────────────────────────────────────────────────────────────────
// ParamPFD::Entry helpers
// ──────────────────────────────────────────────────────────────────────────────
std::vector<uint8_t> ParamPFD::Entry::raw() const {
    std::vector<uint8_t> b(272);
    util::be64put(b.data(), next);
    std::array<uint8_t, 65> nb{};
    std::memcpy(nb.data(), name.c_str(), name.size());
    std::memcpy(b.data() + 8, nb.data(), 65);
    std::memcpy(b.data() + 73, pad0.data(), 7);
    std::memcpy(b.data() + 80, encKey.data(), 64);
    size_t off = 144;
    for (const auto& h: hashes) {
        std::memcpy(b.data() + off, h.data(), 20);
        off += 20;
    }
    std::memcpy(b.data() + 224, pad1.data(), 40);
    util::be64put(b.data() + 264, util::bswap64(size));
    return b;
}
std::vector<uint8_t> ParamPFD::Entry::hashData() const {
    auto r = raw();
    r.erase(r.begin(), r.begin() + 8);
    return r;
}

// ──────────────────────────────────────────────────────────────────────────────
// Constructors & basic accessors
// ──────────────────────────────────────────────────────────────────────────────
ParamPFD::ParamPFD() = default;
ParamPFD::ParamPFD(const std::string& fp) { load(util::read_file(fp)); }
ParamPFD::ParamPFD(const std::vector<uint8_t>& buf) { load(buf); }
ParamPFD::ParamPFD(std::istream& in) {
    std::vector<uint8_t> v{std::istreambuf_iterator<char>(in), {}};
    load(v);
}
const std::vector<ParamPFD::Entry>& ParamPFD::entries() const { return entries_; }

// ──────────────────────────────────────────────────────────────────────────────
// load() – parse PARAM.PFD structure (mirrors C# Init)
// ──────────────────────────────────────────────────────────────────────────────
void ParamPFD::load(const std::vector<uint8_t>& buf) {
    const uint8_t* p = buf.data();
    if (util::be64(p) != 0x50464442ULL) throw std::runtime_error("PFD magic");
    version = util::be64(p + 8);
    std::memcpy(headerIV.data(), p + 16, 16);

    std::array<uint8_t,16> pfdHeaderKey{};
    std::memcpy(pfdHeaderKey.data(), p + 16, 16);
    AES128 aes(SYSCON_KEY);
    auto decryptedHeader = aes.CBCDec({p+32,p+96}, pfdHeaderKey);

    std::memcpy(bottomHash.data(), decryptedHeader.data(), 20);
    std::memcpy(topHash.data(), decryptedHeader.data() + 20, 20);
    std::memcpy(fileKey.data(), decryptedHeader.data() + 40, 20);
    std::memcpy(pad.data(), decryptedHeader.data() + 60, 4);

    realKey = (version == 4) ? hmacSHA1(KEYGEN_KEY, 20, fileKey.data(), 20) : fileKey;

    xyCap = util::be64(p + 96);
    pfReserved = util::be64(p + 104);
    pfUsed = util::be64(p + 112);
    size_t off = 120;

    xTable.resize(xyCap);
    for (auto& v: xTable) {
        v = util::be64(p + off);
        off += 8;
    }

    entries_.resize(pfUsed);
    for (auto& e: entries_) {
        e.next = util::be64(p + off);
        off += 8;
        char nb[65];
        std::memcpy(nb, p + off, 65);
        e.name.assign(nb, strnlen(nb, 65));
        off += 65;
        std::memcpy(e.pad0.data(), p + off, 7);
        off += 7;
        std::memcpy(e.encKey.data(), p + off, 64);
        off += 64;
        for (auto& h: e.hashes) {
            std::memcpy(h.data(), p + off, 20);
            off += 20;
        }
        std::memcpy(e.pad1.data(), p + off, 40);
        off += 40;
        e.size = util::bswap64(util::be64(p + off));
        off += 8;
    }

    size_t unused = (pfReserved - pfUsed) * 272;
    space.assign(p + off, p + off + unused);
    off += unused;
    yTable.resize(xyCap);
    for (auto& y: yTable) {
        std::memcpy(y.data(), p + off, 20);
        off += 20;
    }
    std::memcpy(pad2.data(), p + off, 44);
}

// ──────────────────────────────────────────────────────────────────────────────
// helper calculations identical to C#
// ──────────────────────────────────────────────────────────────────────────────
uint64_t ParamPFD::xyIndex(const std::string& n) const {
    uint64_t h = 0;
    for (char c: n) { h = ((h << 5) - h) + uint8_t(std::tolower(c)); }
    return h % xyCap;
}
std::array<uint8_t, 20> ParamPFD::entryCID(size_t idx) const {
    uint64_t cur = xTable[xyIndex(entries_[idx].name)];
    HMAC_CTX* ctx = HMAC_CTX_new();
    HMAC_Init_ex(ctx, realKey.data(), 20, EVP_sha1(), nullptr);
    while (cur < pfReserved) {
        const auto& e = entries_[cur];
        auto d = e.hashData();
        HMAC_Update(ctx, d.data(), d.size());
        cur = e.next;
    }
    std::array<uint8_t, 20> o{};
    unsigned l = 0;
    HMAC_Final(ctx, o.data(), &l);
    HMAC_CTX_free(ctx);
    return o;
}
std::array<uint8_t, 20> ParamPFD::defaultHash() const {
    HMAC_CTX* ctx = HMAC_CTX_new();
    HMAC_Init_ex(ctx, realKey.data(), 20, EVP_sha1(), nullptr);
    std::array<uint8_t, 20> o{};
    unsigned l = 0;
    HMAC_Final(ctx, o.data(), &l);
    HMAC_CTX_free(ctx);
    return o;
}
std::array<uint8_t, 64> ParamPFD::decryptEntryKey(const Entry& e) const {
    auto key20 = createKeyFromSecureID(secureID);
    std::array<uint8_t,16> key16{};
    std::memcpy(key16.data(), key20.data(), 16);
    AES128 aes(key16);
    auto dec = aes.CBCDec({e.encKey.begin(),e.encKey.end()}, key16);

    std::array<uint8_t, 64> out{};
    std::copy(dec.begin(), dec.end(), out.begin());
    return out;
}

// ──────────────────────────────────────────────────────────────────────────────
// decrypt / encrypt a single child file (CTR+ECB like original implementation)
// ──────────────────────────────────────────────────────────────────────────────
bool ParamPFD::decryptFile(const std::string& root, const std::string& name, std::vector<uint8_t>& out) {
    auto it = std::find_if(entries_.begin(), entries_.end(), [&](const Entry& s) { return strcasecmp(s.name.c_str(), name.c_str()) == 0; });
    if (it == entries_.end()) return false;
    std::string path = root + "/" + name;
    if (!std::filesystem::exists(path)) return false;
    auto cipher = util::read_file(path);
    auto key64 = decryptEntryKey(*it);
    AES128 aesFile(*reinterpret_cast<std::array<uint8_t, 16>*>(key64.data()));
    size_t blocks = util::align16(cipher.size()) / 16;
    cipher.resize(blocks * 16, 0);
    out.resize(blocks * 16);
    for (size_t i = 0; i < blocks; ++i) {
        uint8_t ctr[16]{};
        std::memcpy(ctr, &i, sizeof(i));
        uint8_t mask[16];
        aesFile.ECBEnc(ctr, mask);
        uint8_t plain[16];
        aesFile.ECBDec(cipher.data() + i * 16, plain);
        for (int j = 0; j < 16; ++j) plain[j] ^= mask[j];
        std::memcpy(out.data() + i * 16, plain, 16);
    }
    out.resize(it->size);
    return true;
}

bool ParamPFD::encryptFile(const std::string& root, const std::string& name, const std::vector<uint8_t>& plain) {
    auto it = std::find_if(entries_.begin(), entries_.end(), [&](const Entry& s) { return strcasecmp(s.name.c_str(), name.c_str()) == 0; });
    if (it == entries_.end()) return false;
    auto key64 = decryptEntryKey(*it);
    AES128 aesFile(*reinterpret_cast<std::array<uint8_t, 16>*>(key64.data()));
    size_t blocks = util::align16(plain.size()) / 16;
    std::vector<uint8_t> cipher(blocks * 16, 0);
    for (size_t i = 0; i < blocks; ++i) {
        uint8_t ctr[16]{};
        std::memcpy(ctr, &i, sizeof(i));
        uint8_t mask[16];
        aesFile.ECBEnc(ctr, mask);
        uint8_t buf[16]{};
        size_t chunk = std::min<size_t>(16, plain.size() - i * 16);
        std::memcpy(buf, &plain[i * 16], chunk);
        for (int j = 0; j < 16; ++j) buf[j] ^= mask[j];
        aesFile.ECBEnc(buf, &cipher[i * 16]);
    }
    util::write_file(root + "/" + name, cipher);
    return true;
}

int ParamPFD::decryptAll(const std::string& root) {
    int n = 0;
    for (auto& e: entries_) {
        if (strcasecmp(e.name.c_str(), "PARAM.SFO") == 0) continue;
        std::vector<uint8_t> out;
        if (decryptFile(root, e.name, out)) {
            util::write_file(root + "/" + e.name, out);
            ++n;
        }
    }
    return n;
}
int ParamPFD::encryptAll(const std::string& root) {
    int n = 0;
    for (auto& e: entries_) {
        if (strcasecmp(e.name.c_str(), "PARAM.SFO") == 0) continue;
        auto plain = util::read_file(root + "/" + e.name);
        if (encryptFile(root, e.name, plain)) ++n;
    }
    return n;
}

// ──────────────────────────────────────────────────────────────────────────────
// validateAll – mirrors C# ValidAllParamHashes when fix==true
// (only subset implemented necessary for basic rebuild workflow)
// ──────────────────────────────────────────────────────────────────────────────
bool ParamPFD::validateAll(const std::string& root, bool fix) {
    // per‑entry hash[0]
    for (size_t i = 0; i < entries_.size(); ++i) {
        auto data = util::read_file(root + "/" + entries_[i].name);
        auto h = hmacSHA1(secureID.data(), 16, data.data(), data.size());
        if (h != entries_[i].hashes[0]) {
            if (!fix) return false;
            entries_[i].hashes[0] = h;
        }
    }
    // y table cid
    for (size_t i = 0; i < entries_.size(); ++i) {
        auto cid = entryCID(i);
        size_t idx = xyIndex(entries_[i].name);
        if (cid != yTable[idx]) {
            if (!fix) return false;
            yTable[idx] = cid;
        }
    }
    // bottom / top hashes
    std::vector<uint8_t> ybuf;
    ybuf.reserve(yTable.size() * 20);
    for (auto& y: yTable) ybuf.insert(ybuf.end(), y.begin(), y.end());
    auto bh = hmacSHA1(realKey.data(), 20, ybuf.data(), ybuf.size());
    if (bh != bottomHash) {
        if (!fix) return false;
        bottomHash = bh;
    }

    std::vector<uint8_t> tbuf(24 + xyCap * 8);
    util::be64put(tbuf.data(), util::bswap64(xyCap));
    util::be64put(tbuf.data() + 8, util::bswap64(pfReserved));
    util::be64put(tbuf.data() + 16, util::bswap64(pfUsed));
    size_t off = 24;
    for (auto v: xTable) {
        util::be64put(tbuf.data() + off, util::bswap64(v));
        off += 8;
    }
    auto th = hmacSHA1(realKey.data(), 20, tbuf.data(), tbuf.size());
    if (th != topHash) {
        if (!fix) return false;
        topHash = th;
    }
    return true;
}

// ──────────────────────────────────────────────────────────────────────────────
bool ParamPFD::rebuild(const std::string& root, bool encrypt) {
    if (encrypt) encryptAll(root);
    if (!validateAll(root, true)) return false;
    util::write_file(root + "/PARAM.PFD", serialize());
    return true;
}

std::vector<uint8_t> ParamPFD::serialize() {
    size_t total = 32 + 64 + 24 + xyCap * 8 + pfReserved * 272 + xyCap * 20 + 44;
    std::vector<uint8_t> buf(total);
    util::be64put(buf.data(), 0x50464442ULL);
    util::be64put(buf.data() + 8, version);
    std::memcpy(buf.data() + 16, headerIV.data(), 16);



    std::array<uint8_t, 64> hdr{};
    std::memcpy(hdr.data(), bottomHash.data(), 20);
    std::memcpy(hdr.data() + 20, topHash.data(), 20);
    std::memcpy(hdr.data() + 40, fileKey.data(), 20);
    std::memcpy(hdr.data() + 60, pad.data(), 4);

    AES128 aes(SYSCON_KEY);
    auto enc = aes.CBCEnc({hdr.begin(), hdr.end()}, headerIV);

    std::memcpy(buf.data() + 32, enc.data(), 64);
    util::be64put(buf.data() + 96, xyCap);
    util::be64put(buf.data() + 104, pfReserved);
    util::be64put(buf.data() + 112, pfUsed);
    size_t off = 120;
    for (auto v: xTable) {
        util::be64put(buf.data() + off, v);
        off += 8;
    }
    for (auto& e: entries_) {
        auto r = e.raw();
        std::memcpy(buf.data() + off, r.data(), r.size());
        off += r.size();
    }
    std::memcpy(buf.data() + off, space.data(), space.size());
    off += space.size();
    for (auto& y: yTable) {
        std::memcpy(buf.data() + off, y.data(), 20);
        off += 20;
    }
    std::memcpy(buf.data() + off, pad2.data(), 44);
    return buf;
}

#endif
 */