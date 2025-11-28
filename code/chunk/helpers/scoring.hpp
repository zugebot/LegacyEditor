#pragma once

#include "code/chunk/chunkData.hpp"


namespace editor {

    namespace scoring {

        struct Score {
            std::size_t solidsBelow64;
            std::size_t airAbove192;
        };

        template<typename T>
        inline u8 get(editor::eBlockOrder O, const std::vector<T>& a, int x, int y, int z) {
            return a[toIndex(O, x, y, z)];
        }

        template<typename T, int Bits, bool ShouldBitBeSet, editor::eBlockOrder Order>
        Score scoreChunk(const std::vector<T>& a) {
            const bool isHalf = (Order == editor::eBlockOrder::yXZ); // only 0-127 encoded
            const int YMax = isHalf ? 128 : 256;
            Score s{0, 0};

            for (int z = 0; z < 16; ++z) {
                for (int x = 0; x < 16; ++x) {
                    // bottom 64 layers
                    for (int y = 0; y < 64 && y < YMax; ++y) {
                        if constexpr (ShouldBitBeSet) {
                            if (get(Order, a, x, y, z) != 0) { ++s.solidsBelow64; }
                        } else {
                            if (get(Order, a, x, y, z) == 0) { ++s.solidsBelow64; }
                        }
                    }
                    // top quarter: y ∈ [192,255]   (skip if half-height format)
                    if (!isHalf) {
                        for (int y = 192; y < 256; ++y) {
                            if constexpr (ShouldBitBeSet) {
                                if (get(Order, a, x, y, z) != 0) { ++s.airAbove192; }
                            } else {
                                if (get(Order, a, x, y, z) == 0) { ++s.airAbove192; }
                            }
                        }
                    }
                }
            }
            return s;
        }


        template<typename T, int Bits = 8, bool ShouldBitBeSet = true, bool UseHeightMap = false>
        MU editor::eBlockOrder guessOrder(const std::vector<T>& data,
                                                 const std::vector<u8>& heightMap = {}) {
            using editor::eBlockOrder;

            struct Candidate {
                eBlockOrder o;
                Score s;
            };

            std::vector<Candidate> cand = {{
                    {eBlockOrder::XZY, scoreChunk<T, Bits, ShouldBitBeSet, eBlockOrder::XZY>(data)},
                    {eBlockOrder::YXZ, scoreChunk<T, Bits, ShouldBitBeSet, eBlockOrder::YXZ>(data)},
                    {eBlockOrder::YZX, scoreChunk<T, Bits, ShouldBitBeSet, eBlockOrder::YZX>(data)},
            }};

            // Y=128
            if (data.size() == 32768 / Bits * 8)
                cand.emplace_back(eBlockOrder::yXZ,
                                  scoreChunk<T, Bits, ShouldBitBeSet, eBlockOrder::yXZ>(data));

            // Y=256
            if (data.size() == 65536 / Bits * 8)
                cand.emplace_back(eBlockOrder::yXZy,
                                  scoreChunk<T, Bits, ShouldBitBeSet, eBlockOrder::yXZy>(data));

            // Pick the candidate with the best (solidsBelow64 + airAbove192)
            auto order = std::max_element(cand.begin(), cand.end(),
                                          [](const Candidate& a, const Candidate& b) {
                                              return (a.s.solidsBelow64 + a.s.airAbove192) <
                                                     (b.s.solidsBelow64 + b.s.airAbove192);
                                          })
                                 ->o;

            // disambiguate XZ
            if (order == eBlockOrder::Yqq && UseHeightMap) {
                auto score = [&](eBlockOrder ord) {
                    int ok = 0;
                    for (int z = 0; z < 16; ++z)
                        for (int x = 0; x < 16; ++x) {
                            int h = heightMap[x + z * 16];
                            if (h < 16 && get(ord, data, x, h, z) != 0)
                                ++ok;
                        }
                    return ok;
                };
                int sYXZ = score(eBlockOrder::YXZ);
                int sYZX = score(eBlockOrder::YZX);
                return (sYXZ >= sYZX) ? eBlockOrder::YXZ : eBlockOrder::YZX;
            }
            return order;
        }
    }










}