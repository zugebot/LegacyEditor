#include "include/pfd/pfd.hpp"
#include <iostream>
#include <array>
#include <cstdlib>
#include <fstream>


// Write a buffer out to disk
static bool writeFile(const std::string& path, const std::vector<uint8_t>& buf) {
    std::ofstream f(path, std::ios::binary);
    if (!f) return false;
    f.write((char*)buf.data(), buf.size());
    return true;
}

// Read a file into a buffer
static std::vector<uint8_t> readFile(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("open " + path);
    return { std::istreambuf_iterator<char>(f), {} };
}

/**
 * Decrypts a single child file in a PS3 save folder.
 * @param saveFolder  path to the folder containing PARAM.PFD
 * @param filename    the child filename (e.g. "PARAM.DAT", "TROPSYS.DAT", etc)
 * @param secureID    16-byte Secure File ID
 * @return true on success (file overwritten with plaintext)
 */
bool decryptStandalone(
        const std::string& saveFolder,
        const std::string& filename,
        const std::array<uint8_t,16>& secureID
) {
    // 1) Load the PFD
    ParamPFD p(saveFolder + "/PARAM.PFD");
    p.secureID = secureID;

    // 2) Decrypt just that one file
    std::vector<uint8_t> plain;
    if (!p.decryptFile(saveFolder, filename, plain)) {
        std::cerr << "decryptFile failed for " << filename << "\n";
        return false;
    }

    // 3) Write the plaintext back out
    return writeFile(saveFolder + "/" + filename, plain);
}

/**
 * Encrypts a single child file in a PS3 save folder.
 * @param saveFolder  path to the folder containing PARAM.PFD
 * @param filename    the child filename (must already exist unencrypted)
 * @param secureID    16-byte Secure File ID
 * @return true on success (file overwritten with ciphertext)
 */
bool encryptStandalone(
        const std::string& saveFolder,
        const std::string& filename,
        const std::array<uint8_t,16>& secureID
) {
    // 1) Load the PFD
    ParamPFD p(saveFolder + "/PARAM.PFD");
    p.secureID = secureID;

    // 2) Read the plaintext file
    auto plain = readFile(saveFolder + "/" + filename);

    // 3) Encrypt it and write back
    if (!p.encryptFile(saveFolder, filename, plain)) {
        std::cerr << "encryptFile failed for " << filename << "\n";
        return false;
    }
    return true;
}


int main() {
    // ─── Hardcoded save folder ────────────────────────────────────────────────
    const std::string saveFolder = 
            "C:/Users/jerrin/Desktop/00000001/NPUB31419--140430222735";

    // ─── Hardcoded Secure File ID (32 hex chars → 16 bytes) ─────────────────
    // EEA937CC5BD4D90D55ED2531FA33BDC4
    const std::array<uint8_t,16> secureID = {
            0xEE, 0xA9, 0x37, 0xCC,
            0x5B, 0xD4, 0xD9, 0x0D,
            0x55, 0xED, 0x25, 0x31,
            0xFA, 0x33, 0xBD, 0xC4
    };

    try {
        // Load PARAM.PFD
        ParamPFD p(saveFolder + "/PARAM.PFD");

        // Inject Secure File ID
        p.secureID = secureID;

        // Rebuild hashes (set true if you also need to re-encrypt child files)
        p.encryptAll(saveFolder);
        bool ok = p.rebuild(saveFolder, true);
        if (!ok) {
            std::cerr << "Failed to rebuild PARAM.PFD\n";
            return EXIT_FAILURE;
        }

        std::cout << "PARAM.PFD successfully rebuilt in:\n  "
                  << saveFolder << "\n";
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
