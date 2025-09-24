#pragma once

#include "common/data/ghc/fs_std.hpp"
#include "include/lce/processor.hpp"

#include "code/SaveFile/stateSettings.hpp"
#include "headerUnion.hpp"

#include "code/SaveFile/fileListing.hpp"
#include "common/data/buffer.hpp"
#include "common/error_status.hpp"
#include "lce/enums.hpp"
#include "sfo/sfo.hpp"


namespace editor {
    
    static int detectConsole(const fs::path& inFilePath, StateSettings& stateSettings) {
        static constexpr u32 CON_MAGIC = 0x434F4E20;
        static constexpr u32 ZLIB_MAGIC = 0x789C;


        auto buf = DataReader::readFile(inFilePath);
        DataReader reader(buf.span());

        reader.setEndian(Endian::Big);

        reader.seek(0);
        u32 int0_big = reader.read<u32>();
        u32 int1_big = reader.read<u32>();
        u32 int2_big = reader.read<u32>();

        reader.seek(0);
        u32 short0_big = reader.read<u16>();
        u32 short1_big = reader.read<u16>();
        u32 short2_big = reader.read<u16>();
        u32 short3_big = reader.read<u16>();
        u32 short4_big = reader.read<u16>();

        reader.setEndian(Endian::Little);

        reader.seek(0);
        u32 int0_little = reader.read<u32>();
        u32 int1_little = reader.read<u32>();
        u32 int2_little = reader.read<u32>();

        reader.seek(0);
        u32 short0_little = reader.read<u16>();
        u32 short1_little = reader.read<u16>();
        u32 short2_little = reader.read<u16>();
        u32 short3_little = reader.read<u16>();
        u32 short4_little = reader.read<u16>();


        if (int0_big == 0) {

            if (short4_big == ZLIB_MAGIC) {


                if (int0_big < int0_little) { // file is most likely big endian
                    //  4. (mcs   ) wiiu
                    // 14. (normal) wiiu
                    stateSettings.setConsole(lce::CONSOLE::WIIU);
                    // stateSettings.setMCS(true OR false);


                } else {
                    // The 3 MCS files are functionally the same.
                    //  2. (mcs   ) ps4
                    //  3. (mcs   ) switch
                    //  6. (mcs   ) xbox1

                    //  9. (normal) ps4
                    // 11. (normal) shadps4
                    // 12. (normal) switch
                    // 16. (normal) xbox1
                    // 15. (normal) windurango

                    stateSettings.setNewGen(true);



                    const fs::path dir = inFilePath.parent_path();


                    std::error_code ec;
                    bool hasGamedata000 = false;
                    for (const auto& de : fs::directory_iterator(dir, ec)) {
                        if (!de.is_regular_file(ec)) continue;
                        const std::string fName = de.path().filename().string();
                        if (fName.starts_with("GAMEDATA_000")) {
                            hasGamedata000 = true;
                            break;
                        }
                    }
                    if (hasGamedata000) {
                        if (fs::exists(dir / "wd_displayname.txt")) {
                            stateSettings.setConsole(lce::CONSOLE::WINDURANGO);
                            return SUCCESS;
                        } else {
                            stateSettings.setConsole(lce::CONSOLE::XBOX1);
                            return SUCCESS;
                        }
                    }

                    // TODO: this code does not try to guess if it is a new gen MCS file or not
                    if (fs::path sfoPath = inFilePath.parent_path() / "sce_sys" / "param.sfo"; fs::exists(sfoPath)) {

                        SFOManager sfo(sfoPath.string());
                        if (sfo.getAttribute("PARAMS").has_value()) {
                            stateSettings.setConsole(lce::CONSOLE::PS4);
                            return SUCCESS;
                        } else {
                            stateSettings.setConsole(lce::CONSOLE::SHADPS4);
                            return SUCCESS;
                        }

                    } else if (fs::path temp = inFilePath;
                               fs::exists(temp.replace_extension(".sub"))) {
                        stateSettings.setConsole(lce::CONSOLE::SWITCH);
                        return SUCCESS;

                    } else {
                        stateSettings.setMCS(true);
                        stateSettings.setConsole(lce::CONSOLE::NEWGENMCS);
                    }
                }



            } else if (int1_big == int2_big) {
                // 1. (mcs   ) ps3
                stateSettings.setConsole(lce::CONSOLE::PS3);
                stateSettings.setMCS(true);

            } else if (std::abs((i32)int1_little - (i32)int2_little) < 65536) {
                //  5. (mcs   ) vita
                // 13. (normal) vita
                stateSettings.setConsole(lce::CONSOLE::VITA);
                // stateSettings.setMCS(true OR false);

            } else {
                // 7. (mcs   ) xbox360 (.dat)
                stateSettings.setConsole(lce::CONSOLE::XBOX360);
                stateSettings.setXbox360Bin(false);
                stateSettings.setMCS(true);
            }


        } else if (int0_big == CON_MAGIC) {
            // 17. (normal) xbox360 (.bin)

            stateSettings.setConsole(lce::CONSOLE::XBOX360);
            stateSettings.setXbox360Bin(true);
            stateSettings.setMCS(false);

        } else if (int1_big == 0) {
            // 18. (normal) xbox360 (.dat)
            stateSettings.setConsole(lce::CONSOLE::XBOX360);
            stateSettings.setXbox360Bin(false);

        // it is most likely a raw fileListing
        } else {
            // 10. (normal) rpcs3
            //  8. (normal) ps3
            // Basically, all raw fileListings and/or non-minecraft worlds.

            stateSettings.setCompressed(false);

            // PS3 is natively just a raw fileListing
            fs::path sfoPath = inFilePath.parent_path() / "param.sfo";
            if (fs::exists(sfoPath)) {
                stateSettings.setConsole(lce::CONSOLE::PS3);

                SFOManager mainSFO(sfoPath.string());
                if (mainSFO.getAttribute("RPCS3_BLIST").has_value()) {
                    stateSettings.setConsole(lce::CONSOLE::RPCS3);
                }

            // It is some other console as a raw fileListing,
            // I originally had it as "int1_big % 136 == 0" for decompressed pre-Xbox360-TU4 saves.
            // TODO: write code that determines save from chunk contents
            } else {

                reader.setEndian(Endian::Big);
                bool isValidBig = editor::FileListing::isValid(reader);

                reader.setEndian(Endian::Little);
                bool isValidLittle = editor::FileListing::isValid(reader);

                // from here, we can try to determine it off:
                // 1. of the player file names
                // 2. ...
                if (isValidBig) {
                    reader.setEndian(Endian::Big);
                } else if (isValidLittle) {
                    reader.setEndian(Endian::Little);
                } else {
                    return INVALID_SAVE;
                }

                ListingHeader header(reader);
                c_auto items = FileListing::createListItems(reader, header);

                if (isValidBig && header.latestVersion <= 1) {
                    stateSettings.setConsole(lce::CONSOLE::XBOX360);
                    stateSettings.setXbox360Bin(false);
                } else {
                    // ... Lot's of code TODO here, for now just stub it.
                    return INVALID_SAVE;
                }

            }

        }
    
        return SUCCESS;
    }
}




/*
        FILE* f_in = fopen(inFilePath.string().c_str(), "rb");
        if (f_in == nullptr) {
            return printf_err(FILE_ERROR, ERROR_4, inFilePath.string().c_str());
        }

        fseek(f_in, 0, SEEK_END);
        c_u64 input_size = ftell(f_in);
        fseek(f_in, 0, SEEK_SET);
        if (input_size < 12) {
            return printf_err(FILE_ERROR, ERROR_5);
        }
        HeaderUnion headerUnion{};
        fread(&headerUnion, 1, 12, f_in);
        fclose(f_in);

bool isNewGen = false;

Buffer data;
if (headerUnion.getInt1() <= 2) {
    if (headerUnion.getShort5() == ZLIB_MAGIC) {
        if (headerUnion.getInt2Swap() >= headerUnion.getDestSize()) {
            stateSettings.setConsole(lce::CONSOLE::WIIU);
        } else {
            isNewGen = true;
        }
    } else {
        // TODO: change this to write custom checker for FILE_COUNT * 144 == diff. with
        // TODO: with custom vitaRLE decompress checker
        c_u32 indexFromSF = headerUnion.getInt2Swap() - headerUnion.getInt3Swap();
        if (indexFromSF > 0 && indexFromSF < 65536) {
            stateSettings.setConsole(lce::CONSOLE::VITA);
        } else { // compressed ps3
                 // TODO: I don't believe in the existence of "compressed PS3", so I am repurposing it
            // stateSettings.setConsole(lce::CONSOLE::PS3);
            stateSettings.setConsole(lce::CONSOLE::XBOX360);
            stateSettings.setXbox360Bin(false);
            stateSettings.setMCS(true);
        }
    }
} else if (headerUnion.getInt2() <= 2) {
    /// if (int2 == 0) it is an xbox savefile unless it's a massive
    /// file, but there won't be 2 files in a savegame file for PS3
    stateSettings.setConsole(lce::CONSOLE::XBOX360);
    stateSettings.setXbox360Bin(false);
    // TODO: don't use arbitrary guess for a value
} else if (headerUnion.getInt1() == CON_MAGIC) {
    stateSettings.setConsole(lce::CONSOLE::XBOX360);
    stateSettings.setXbox360Bin(true);
} else if (headerUnion.getInt2() % 136 == 0) {
    // This is here as a gag, but it will work!
    stateSettings.setConsole(lce::CONSOLE::XBOX360);
    stateSettings.setXbox360Bin(false);
    stateSettings.setCompressed(false);
} else if (headerUnion.getInt2() < 1000) { // uncompressed PS3 / RPCS3
    if (fs::path sfoPath = inFilePath.parent_path() / "param.sfo";
        fs::exists(sfoPath)) {
        SFOManager mainSFO(sfoPath.string());
        auto testAttr = mainSFO.getAttribute("RPCS3_BLIST");
        if (testAttr) {
            stateSettings.setConsole(lce::CONSOLE::RPCS3);
        } else {
            stateSettings.setConsole(lce::CONSOLE::PS3);
            stateSettings.setCompressed(false);
        }
    } else {
        // fallback, because RPCS3 saves are easier to use than PS3, could be config?
        stateSettings.setConsole(lce::CONSOLE::RPCS3);
    }

} else {
    return INVALID_SAVE; // printf_err(INVALID_SAVE, ERROR_3);
}

// Lastly, check for other files existence to differentiate
// TODO: this does not detect lce::CONSOLE::XBOX1
if (isNewGen) {
    if (fs::path sfoPath = inFilePath.parent_path() / "sce_sys" / "param.sfo"; fs::exists(sfoPath)) {

        SFOManager sfo(sfoPath.string());
        if (!sfo.getAttribute("PARAMS").has_value()) {
            stateSettings.setConsole(lce::CONSOLE::SHADPS4);
        } else {
            stateSettings.setConsole(lce::CONSOLE::PS4);
        }

    } else if (fs::path temp = inFilePath;
               fs::exists(temp.replace_extension(".sub"))) {
        stateSettings.setConsole(lce::CONSOLE::SWITCH);
    }

    if (fs::exists(stateSettings.filePath().parent_path() / "wd_displayname.txt")) {
        stateSettings.setConsole(lce::CONSOLE::WINDURANGO);
    }
}
*/