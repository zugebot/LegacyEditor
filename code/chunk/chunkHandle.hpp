#pragma once

#include "include/lce/enums.hpp"
#include "common/error_status.hpp"
#include "common/buffer.hpp"

#include "chunkData.hpp"


namespace editor {

    class WriteSettings;


    struct ChunkHeader {
    private:
    public:
        u32 rleSize{0};
        u32 timestamp : 32 {0};
        u32 decSize   : 28 {0};
        u32 isRLE : 1 {1};
        u32 isNewSave       : 1 {1};
        u32 isDirty         : 1 {0};
        u32 isWritten       : 1 {0};

        MU u32  getTimestamp() const     { return timestamp; }
        void    setTimestamp(u32 v)      { timestamp = v;    }

        MU u32  getDecSize() const       { return decSize;   }
        void    setDecSize(u32 v)        { decSize = v;      }

        MU u32  getRLESize() const       { return rleSize;   }
        void    setRLESize(u32 v)        { rleSize = v;      }

        MU ND bool rle() const           { return isRLE; }
        void    setRle(bool v)           { isRLE = v;    }

        MU ND bool newSave() const       { return isNewSave;       }
        void    setNewSave(bool v)       { isNewSave = v;          }

        MU ND bool dirty() const         { return isDirty;         }
        void    markDirty(bool v= true)  { isDirty = v;            }

        MU ND bool written() const       { return isWritten;       }
        void    markWritten(bool v=true) { isWritten = v;          }
        void    clearWritten()           { isWritten = 0;          }
    };


    class ChunkHandle {
    public:

        ChunkHeader header;
        Buffer      buffer;
        std::unique_ptr<ChunkData> data;

        // ctors

        ChunkHandle() : data(std::make_unique<ChunkData>()) {}
        ~ChunkHandle() = default;
        ChunkHandle(ChunkHandle&&)            noexcept = default;
        ChunkHandle& operator=(ChunkHandle&&) noexcept = default;
        ChunkHandle(const ChunkHandle&)                 = delete;
        ChunkHandle& operator=(const ChunkHandle&)      = delete;

        /// FUNCTIONS

        MU int read(DataReader& rdr, lce::CONSOLE c);
        MU int write(DataWriter& wtr, lce::CONSOLE c);

        MU int decodeChunk(lce::CONSOLE c);
        MU int encodeChunk(WriteSettings& settings);

        void unpackSizeFlags(u32 word);
        ND u32 packSizeFlags() const;
    };

} // namespace editor