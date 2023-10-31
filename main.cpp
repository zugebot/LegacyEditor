#include <cassert>
#include <iostream>

#include "LegacyEditor/utils/dataManager.hpp"

#include "LegacyEditor/LCE/ConsoleParser.hpp"
#include "LegacyEditor/LCE/Region.hpp"
#include "LegacyEditor/LCE/fileListing.hpp"
#include "LegacyEditor/utils/time.hpp"


/*
class DataManagerTest {
public:
    static void runTests() {
        testBigEndian();
        testLittleEndian();
        std::cout << "All tests passed!" << std::endl;
    }

private:
    static void testBigEndian() {
        std::string filename = "temp_big_endian.dat";
        DataManager dataManager(1000);
        dataManager.setBigEndian();

        // Write test data
        writeTestData(dataManager);

        // Write to file
        assert(dataManager.writeToFile(filename) == 0);

        // Read from file
        DataManager readManager;
        readManager.setBigEndian();
        assert(readManager.readFromFile(filename) == 0);

        // Verify the data
        verifyTestData(readManager);

        // std::remove(filename.c_str()); // Clean up the temporary file
        std::cout << "Big endian tests passed." << std::endl;
    }

    static void testLittleEndian() {
        std::string filename = "temp_little_endian.dat";
        DataManager dataManager(1000);
        dataManager.setLittleEndian();

        // Write test data
        writeTestData(dataManager);

        // Write to file
        assert(dataManager.writeToFile(filename) == 0);

        // Read from file
        DataManager readManager;
        readManager.setLittleEndian();
        assert(readManager.readFromFile(filename) == 0);

        // Verify the data
        verifyTestData(readManager);

        // std::remove(filename.c_str()); // Clean up the temporary file
        std::cout << "Little endian tests passed." << std::endl;
    }

    static void writeTestData(DataManager& dataManager) {
        dataManager.writeByte(0x12);
        dataManager.writeShort(0x1234);
        dataManager.writeInt(0x12345678);
        dataManager.writeLong(0x123456789ABCDEF0);
        dataManager.writeFloat(123.456f);
        dataManager.writeDouble(123.456789);

        std::string testWString = "Hello World"; // Assuming ASCII characters only
        dataManager.writeWString(testWString, testWString.length());
    }

    static void verifyTestData(DataManager& dataManager) {
        assert(dataManager.readByte() == 0x12);
        assert(dataManager.readShort() == 0x1234);
        assert(dataManager.readInt() == 0x12345678);
        assert(dataManager.readLong() == 0x123456789ABCDEF0);
        assert(dataManager.readFloat() == 123.456f);
        assert(dataManager.readDouble() == 123.456789);

        std::string expectedWString = "Hello World"; // Assuming ASCII characters only
        std::string readWString = dataManager.readWAsString(expectedWString.length());
        assert(readWString == expectedWString);
    }
};

int main() {
    DataManagerTest::runTests();
    return 0;
}
*/






int main() {

    /*
    {
        DataManager managerOut(14);
        managerOut.setBigEndian();
        u16 i1 = 0x1234;
        u32 i2 = 0x12345678;
        u64 i3 = 0x1234567812345678;
        managerOut.writeShort(i1);
        managerOut.writeInt(i2);
        managerOut.writeLong(i3);
        managerOut.writeToFile(dir_path + "data_out.bin");
        printf("%hu\n%i\n%llu\n", i1, i2, i3);
        // managerOut.writeWString("Hello World!", 64, true);
        DataManager managerIn;
        managerIn.setBigEndian();
        managerIn.readFromFile(dir_path + "data_out.bin");
        i1 = managerIn.readShort();
        i2 = managerIn.readInt();
        i3 = managerIn.readLong();
        printf("%hu\n%i\n%llu\n", i1, i2, i3);
        // auto stringOut = managerIn.readWAsString(64, true);
        // std::cout << stringOut << ", " << stringOut.size() << std::endl;
    }
     */
    // /*
    std::string fileInPath = dir_path + "230918230206_in.wii";

    auto start = getMilliseconds();

    ConsoleParser parser;
    int status = parser.loadConsoleFile(fileInPath.c_str());

    FileListing fileListing(parser);

    Region region(CONSOLE::WIIU);
    region.read(fileListing.overworldFilePtrs[0]);
    Chunk* chunk = region.getChunk(171);

    for (int i = 0; i < 1024; i++) {
        if (chunk->sectors) {
            chunk->ensure_decompress(CONSOLE::WIIU);
            chunk->ensure_compressed(CONSOLE::WIIU);
        }
    }

    /*
    printf("hi");
    for (auto* regionFilePtr : fileListing.overworldFilePtrs) {
        region.read(regionFilePtr);
        for (int i = 0; i < 1024; i++) {
            chunk = region.getChunk(i);
            chunk->ensure_decompress(CONSOLE::WIIU);
            chunk->ensure_compressed(CONSOLE::WIIU);
            int x = 0;
            std::cout << x;
        }
        Data data = region.write(CONSOLE::WIIU);
        regionFilePtr->deallocate();
        regionFilePtr->data = data.data;
        regionFilePtr->size = data.size;
        data.using_memory = false;
        printf("Recompressed all chunks in '%s'\n", regionFilePtr->name.c_str());
    }
    for (auto* regionFilePtr : fileListing.netherFilePtrs) {
        region.read(regionFilePtr);
        for (int i = 0; i < 1024; i++) {
            chunk = region.getChunk(i);
            chunk->ensure_decompress(CONSOLE::WIIU);
            chunk->ensure_compressed(CONSOLE::WIIU);
        }
        Data data = region.write(CONSOLE::WIIU);
        regionFilePtr->deallocate();
        regionFilePtr->data = data.data;
        regionFilePtr->size = data.size;
        data.using_memory = false;

        printf("Recompressed all chunks in '%s'\n", regionFilePtr->name.c_str());
    }
    for (auto* regionFilePtr : fileListing.endFilePtrs) {
        region.read(regionFilePtr);
        for (int i = 0; i < 1024; i++) {
            chunk = region.getChunk(i);
            chunk->ensure_decompress(CONSOLE::WIIU);
            chunk->ensure_compressed(CONSOLE::WIIU);
        }
        Data data = region.write(CONSOLE::WIIU);
        regionFilePtr->deallocate();
        regionFilePtr->data = data.data;
        regionFilePtr->size = data.size;
        data.using_memory = false;

        printf("Recompressed all chunks in '%s'\n", regionFilePtr->name.c_str());
    }














     */
    Data dataOut = fileListing.write();
    // */
    /*
    // compare decompressed FileListing
    // DataManager(parser).writeToFile(dir_path + "in_" + "file_listing");
    // DataManager(dataOut).writeToFile(dir_path + "out_" + "file_listing");

    // test to ensure read and write data are the same
    // for (int i = 0; i < 1058310; i++) {
    //     u8 a = parser.data[i];
    //     u8 b = dataOut.data[i];
    //     if (a != b) {
    //         delete[] &a; // sysfaults
    //     }
    // }

    // int reg = 1;
    // File* filePtr = fileListing.overworldFilePtrs[reg];
    // std::cout << filePtr->name << std::endl;

    // Region region(CONSOLE::WIIU);
    // region.read(filePtr);

    // Chunk* chunk = region.getChunk(4, 15);
    // chunk->ensure_decompress(CONSOLE::WIIU);

    // DataManager chunkIn(chunk->data, chunk->size);
    // u16 version = chunkIn.readShort();
    // i32 xPos = (i32)chunkIn.readInt();
    // i32 zPos = (i32)chunkIn.readInt();
    // std::cout << version << " " << xPos << " " << zPos << std::endl;
    // chunkIn.seekStart();
    // chunk->ensure_compressed(CONSOLE::WIIU);

    // int count = -1;
    // for (Chunk& chunk : region.chunks) {
    //     count++;
    //     if (chunk.sectors == 0) continue;
    //     std::cout << count << std::endl;
    //     printf("[%4u, %1u] [%5u][%5u]\n", chunk.location, chunk.sectors, chunk.size, chunk.dec_size);
    // }

    // Data data = region.write(CONSOLE::WIIU);
    // fileListing.overworldFilePtrs[reg]->data = data.data;
    // fileListing.overworldFilePtrs[reg]->size = data.size;

    // DataManager(filePtr).writeToFile(dir_path + "in_" + filePtr->name);
    // DataManager(data).writeToFile(dir_path + "out_" + filePtr->name);
    */
    // /*
    // std::string out_file = dir_path + "230918230206_out.wii";
    // ConsoleParser::saveWiiU(out_file, dataOut);

    auto diff = getMilliseconds() - start;
    std::cout << "time: " << diff << "ms" << std::endl;

    return status;
     // */
}


// worldIn.seekStart();
// worldOut.seekStart();
// printf("\nin: %u | out: %u\n", worldIn.size, worldOut.size);
// std::string file_path_dec_in = dir_path + "test_dec_in.wii_d";
// std::string file_path_dec_out = dir_path + "test_dec_out.wii_d";
// worldIn.saveToFile(file_path_dec_in);
// worldOut.saveToFile(file_path_dec_out);
// std::cout << "waiting..." << std::endl;