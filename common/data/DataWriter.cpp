#include "DataWriter.hpp"


Buffer DataWriter::take() {
    if (!_buf || !_cap)
        return {};
    uint8_t* raw = _buf.release();
    const uint32_t sz  = _pos;
    Buffer result{ std::unique_ptr<uint8_t[]>(raw), sz };
    _cap = _pos = 0;
    return result;
}
