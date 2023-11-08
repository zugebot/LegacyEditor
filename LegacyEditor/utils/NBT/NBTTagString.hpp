#pragma once

#include "LegacyEditor/utils/processor.hpp"


class NBTTagString {
public:
    char* data;
    i64 size;
    NBTTagString() : data(nullptr), size(0) {}

    explicit NBTTagString(const std::string& dataIn) {
        size = (int) dataIn.size();
        data = (char*) malloc(size);
        memcpy(data, dataIn.c_str(), size);
    }

    ND inline bool hasNoTags() const { return size; }

    ND std::string getString() const { return std::string(data, size); }

    ND std::string toStringNBT() const {
        std::string stringBuilder = "\"";
        std::string currentString = getString();
        for (char c0: currentString) {
            if (c0 == '\\' || c0 == '"') { stringBuilder.append("\\"); }

            stringBuilder.push_back(c0);
        }
        return stringBuilder.append("\"");
    }
};
