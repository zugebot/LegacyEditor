#pragma once

#include <memory>
#include <stdexcept>

#include "include/lce/enums.hpp"
#include "common/error_status.hpp"
#include "common/data/buffer.hpp"

#include "chunkData.hpp"

namespace editor {

    class WriteSettings;

    struct ChunkHeader {
    public:
        u32 rleSize{0};
        u32 timestamp : 32 {0};
        u32 decSize   : 28 {0};
        u32 isRLE     : 1  {1};
        u32 isNewSave : 1  {1};
        u32 isDirty   : 1  {0};

        MU u32 getTimestamp() const { return timestamp; }
        void   setTimestamp(u32 v)  { timestamp = v; }

        MU u32 getDecSize() const   { return decSize; }
        void   setDecSize(u32 v)    { decSize = v; }

        MU u32 getRLESize() const   { return rleSize; }
        void   setRLESize(u32 v)    { rleSize = v; }

        MU ND bool rle() const      { return isRLE; }
        void   setRle(bool v)       { isRLE = v; }

        MU ND bool newSave() const  { return isNewSave; }
        void   setNewSave(bool v)   { isNewSave = v; }

        MU ND bool dirty() const        { return isDirty; }
        void   markDirty(bool v = true) { isDirty = v; }
    };

    enum class ChunkState : u8 {
        EMPTY = 0,
        COMPRESSED,
        DECODED
    };

    class ChunkHandle {
    public:
        // Public header stays the same usage-wise
        ChunkHeader header;

        // When state == COMPRESSED, buffer holds the on-disk compressed bytes (zlib/xdecompress output).
        // When state != COMPRESSED, buffer is empty.
        Buffer buffer;

        // Proxy so existing call sites keep working:
        //     handle->data->setBlock(...)
        // This auto-decodes and marks dirty every time you touch it.
        struct DataProxy {
            ChunkHandle* owner = nullptr;

            ChunkData* operator->() {
                if (!owner) throw std::runtime_error("ChunkHandle::data proxy has null owner");
                return owner->decodeAndTouch();
            }
            const ChunkData* operator->() const {
                if (!owner) throw std::runtime_error("ChunkHandle::data proxy has null owner");
                return owner->decodeAndTouch();
            }
        } data;

        ChunkHandle()
            : m_state(ChunkState::EMPTY),
              m_srcConsole(lce::CONSOLE::NONE),
              m_decoded(nullptr),
              data{this} {}

        ~ChunkHandle() = default;

        // custom move so proxy owner stays correct
        ChunkHandle(ChunkHandle&& other) noexcept;
        ChunkHandle& operator=(ChunkHandle&& other) noexcept;

        ChunkHandle(const ChunkHandle&) = delete;
        ChunkHandle& operator=(const ChunkHandle&) = delete;

        ND ChunkState state() const { return m_state; }
        ND bool empty() const { return m_state == ChunkState::EMPTY; }

        // IO
        MU int read(DataReader& rdr, lce::CONSOLE c);
        MU int write(DataWriter& wtr, lce::CONSOLE c);

        // State transitions
        MU int decodeChunk(lce::CONSOLE c);          // COMPRESSED -> DECODED (buffer cleared)
        MU int encodeChunk(WriteSettings& settings); // DECODED -> COMPRESSED (decoded cleared)

        void setEmpty();

        void unpackSizeFlags(u32 word);
        ND u32 packSizeFlags() const;

    private:
        ChunkState m_state;
        lce::CONSOLE m_srcConsole;
        std::unique_ptr<ChunkData> m_decoded;

        // Called by DataProxy. Always marks dirty. Always ensures decoded.
        ChunkData* decodeAndTouch();

        void resetProxyOwner() { data.owner = this; }
    };

} // namespace editor
