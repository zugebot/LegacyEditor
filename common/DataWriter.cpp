#include "DataWriter.hpp"



Buffer DataWriter::take() {
    if (!_buf || !_cap)
        return {};
    u8* raw = _buf.release();
    u32 sz  = _pos;
    Buffer result{ std::unique_ptr<u8[]>(raw), sz };
    _cap = _pos = 0;
    return result;
}
