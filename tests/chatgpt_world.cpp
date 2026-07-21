#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <cmath>
#include <cstdint>
#include <algorithm>

#include "common/data/ghc/fs_std.hpp"

#include "include/lce/processor.hpp"
#include "include/lce/blocks/blockID.hpp"

#include "code/SaveFile/World.hpp"
#include "code/include.hpp"
#include "common/fmt.hpp"

namespace atlas {

    // World bounds you gave
    static constexpr int X_MIN = -432;
    static constexpr int X_MAX =  432;
    static constexpr int Z_MIN = -432;
    static constexpr int Z_MAX =  432;
    static constexpr int Y_MIN =  0;
    static constexpr int Y_MAX =  256;

    static constexpr double PI = 3.141592653589793238462643383279502884;

    static inline bool inBounds(int x, int y, int z) {
        return (x >= X_MIN && x <= X_MAX) &&
               (z >= Z_MIN && z <= Z_MAX) &&
               (y >= Y_MIN && y <= Y_MAX);
    }

    static inline u16 RB(lce::blocks::BlockID id, u16 data = 0) {
        return static_cast<u16>((static_cast<u16>(id) << 4) | (data & 0xF));
    }

    static inline void set(editor::World& w, int x, int y, int z, u16 raw) {
        if (inBounds(x, y, z)) w.setBlock(x, y, z, raw);
    }

    static inline int iround(double v) {
        return static_cast<int>(std::llround(v));
    }

    static inline double clampd(double v, double a, double b) {
        return std::max(a, std::min(b, v));
    }

    // ------------------------------------------------------------
    // Deterministic hash and noise for patterns
    // ------------------------------------------------------------
    static inline uint32_t mix32(uint32_t x) {
        x ^= x >> 16;
        x *= 0x7feb352dU;
        x ^= x >> 15;
        x *= 0x846ca68bU;
        x ^= x >> 16;
        return x;
    }

    static inline uint32_t hash2i(int x, int z, uint32_t seed) {
        uint32_t a = static_cast<uint32_t>(x) * 0x9e3779b1U;
        uint32_t b = static_cast<uint32_t>(z) * 0x85ebca6bU;
        return mix32(a ^ (b + seed));
    }

    static inline double h01(int x, int z, uint32_t seed) {
        const uint32_t h = hash2i(x, z, seed);
        return (h & 0x00FFFFFF) / double(0x01000000);
    }

    static inline double fade(double t) {
        // Smoothstep (quintic) for nicer interpolation
        return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
    }

    static inline double lerp(double a, double b, double t) {
        return a + (b - a) * t;
    }

    static inline double valueNoise(double x, double z, double freq, uint32_t seed) {
        const double xf = x * freq;
        const double zf = z * freq;

        const int x0 = static_cast<int>(std::floor(xf));
        const int z0 = static_cast<int>(std::floor(zf));
        const int x1 = x0 + 1;
        const int z1 = z0 + 1;

        const double tx = fade(xf - x0);
        const double tz = fade(zf - z0);

        const double v00 = h01(x0, z0, seed);
        const double v10 = h01(x1, z0, seed);
        const double v01 = h01(x0, z1, seed);
        const double v11 = h01(x1, z1, seed);

        const double a = lerp(v00, v10, tx);
        const double b = lerp(v01, v11, tx);
        return lerp(a, b, tz);
    }

    static inline double fbm(double x, double z, double freq, int oct, uint32_t seed) {
        double amp = 1.0;
        double sum = 0.0;
        double norm = 0.0;
        double f = freq;

        for (int i = 0; i < oct; ++i) {
            const double n = valueNoise(x, z, f, seed + 1013U * static_cast<uint32_t>(i));
            sum += (n * 2.0 - 1.0) * amp;
            norm += amp;
            amp *= 0.5;
            f *= 2.0;
        }
        return (norm > 0.0) ? (sum / norm) : 0.0;
    }

    // ------------------------------------------------------------
    // Geometry primitives
    // ------------------------------------------------------------
    static inline void fillColumn(editor::World& w, int x, int z, int y0, int y1, u16 raw) {
        if (y0 > y1) std::swap(y0, y1);
        y0 = std::max(y0, Y_MIN);
        y1 = std::min(y1, Y_MAX);
        for (int y = y0; y <= y1; ++y) set(w, x, y, z, raw);
    }

    static inline void disc(editor::World& w, int cx, int y, int cz, int r, u16 raw) {
        const int r2 = r * r;
        for (int z = cz - r; z <= cz + r; ++z) {
            for (int x = cx - r; x <= cx + r; ++x) {
                const int dx = x - cx;
                const int dz = z - cz;
                if (dx * dx + dz * dz <= r2) set(w, x, y, z, raw);
            }
        }
    }

    static inline void ring(editor::World& w, int cx, int y, int cz, int r, int thick, u16 raw) {
        const int r0 = std::max(0, r - thick);
        const int r1 = r + thick;
        const int r02 = r0 * r0;
        const int r12 = r1 * r1;

        for (int z = cz - r1; z <= cz + r1; ++z) {
            for (int x = cx - r1; x <= cx + r1; ++x) {
                const int dx = x - cx;
                const int dz = z - cz;
                const int d2 = dx * dx + dz * dz;
                if (d2 >= r02 && d2 <= r12) set(w, x, y, z, raw);
            }
        }
    }

    static inline void boxHollow(editor::World& w, int x0, int y0, int z0, int x1, int y1, int z1, u16 raw) {
        if (x0 > x1) std::swap(x0, x1);
        if (y0 > y1) std::swap(y0, y1);
        if (z0 > z1) std::swap(z0, z1);

        for (int y = y0; y <= y1; ++y) {
            for (int z = z0; z <= z1; ++z) {
                set(w, x0, y, z, raw);
                set(w, x1, y, z, raw);
            }
            for (int x = x0; x <= x1; ++x) {
                set(w, x, y, z0, raw);
                set(w, x, y, z1, raw);
            }
        }
        for (int z = z0; z <= z1; ++z) {
            for (int x = x0; x <= x1; ++x) {
                set(w, x, y0, z, raw);
                set(w, x, y1, z, raw);
            }
        }
    }

    static inline void sphereShell(editor::World& w, int cx, int cy, int cz, int r, int band, u16 raw) {
        const int r2 = r * r;
        const int lo = std::max(0, r2 - band);
        const int hi = r2 + band;

        for (int z = cz - r; z <= cz + r; ++z) {
            for (int y = cy - r; y <= cy + r; ++y) {
                for (int x = cx - r; x <= cx + r; ++x) {
                    const int dx = x - cx;
                    const int dy = y - cy;
                    const int dz = z - cz;
                    const int d2 = dx * dx + dy * dy + dz * dz;
                    if (d2 >= lo && d2 <= hi) set(w, x, y, z, raw);
                }
            }
        }
    }

    static inline void line3D(editor::World& w,
                              int x0, int y0, int z0,
                              int x1, int y1, int z1,
                              int radius,
                              u16 raw) {
        const int dx = std::abs(x1 - x0);
        const int dy = std::abs(y1 - y0);
        const int dz = std::abs(z1 - z0);

        const int steps = std::max({dx, dy, dz, 1});
        for (int i = 0; i <= steps; ++i) {
            const double t = double(i) / double(steps);
            const int x = iround(lerp(double(x0), double(x1), t));
            const int y = iround(lerp(double(y0), double(y1), t));
            const int z = iround(lerp(double(z0), double(z1), t));

            if (radius <= 0) {
                set(w, x, y, z, raw);
            } else {
                sphereShell(w, x, y, z, radius, radius * 2, raw);
            }
        }
    }

    // ------------------------------------------------------------
    // Binary mural (Sea Lantern = 1, Obsidian = 0)
    // Each character is 8 bits (MSB at top). Read left to right.
    // ------------------------------------------------------------
    enum class Facing : uint8_t { POS_X, NEG_X, POS_Z, NEG_Z };

    static inline void binaryMural(editor::World& w,
                                   int x0, int y0, int z0,
                                   Facing face,
                                   const std::string& msg,
                                   u16 one,
                                   u16 zero,
                                   u16 frame,
                                   int scale = 1) {
        const int sx = (face == Facing::POS_X) ? 1 : (face == Facing::NEG_X ? -1 : 0);
        const int sz = (face == Facing::POS_Z) ? 1 : (face == Facing::NEG_Z ? -1 : 0);

        const int charW = 9 * scale;   // 8 bits + 1 gap
        const int bitH  = 1 * scale;   // each bit row
        const int muralH = 8 * bitH;

        // Frame
        const int wLen = static_cast<int>(msg.size()) * charW;
        for (int i = -scale; i <= wLen + scale; ++i) {
            for (int j = -scale; j <= muralH + scale; ++j) {
                const bool border = (i == -scale || i == wLen + scale || j == -scale || j == muralH + scale);
                if (!border) continue;

                const int x = x0 + i * sx;
                const int z = z0 + i * sz;
                const int y = y0 + j;
                set(w, x, y, z, frame);
            }
        }

        // Bits
        for (int c = 0; c < static_cast<int>(msg.size()); ++c) {
            const uint8_t ch = static_cast<uint8_t>(msg[c]);
            for (int b = 0; b < 8; ++b) {
                const int bit = (ch >> (7 - b)) & 1;
                const u16 raw = bit ? one : zero;

                for (int yy = 0; yy < bitH; ++yy) {
                    for (int xx = 0; xx < scale; ++xx) {
                        const int i = c * charW + (b * scale) + xx;
                        const int x = x0 + i * sx;
                        const int z = z0 + i * sz;
                        const int y = y0 + (7 - b) * bitH + yy;
                        set(w, x, y, z, raw);
                    }
                }
            }
        }
    }

    // ------------------------------------------------------------
    // Site building
    // ------------------------------------------------------------
    struct Site {
        int x, y, z;
        const char* name;
        u16 base;
        u16 accent;
        u16 light;
    };

    static inline void buildFragmentPlatform(editor::World& w, int cx, int cy, int cz,
                                             int r, u16 top, u16 rim, u16 under, u16 light) {
        // Top surface
        disc(w, cx, cy, cz, r, top);

        // Rim and inlay
        ring(w, cx, cy, cz, r, 1, rim);
        ring(w, cx, cy, cz, r - 6, 1, light);

        // Underside "stalactite ribs"
        const int ribs = 14;
        for (int i = 0; i < ribs; ++i) {
            const double a = (2.0 * PI) * (double(i) / ribs);
            const int rx = cx + iround((r - 5) * std::cos(a));
            const int rz = cz + iround((r - 5) * std::sin(a));

            const int drop = 18 + (hash2i(rx, rz, 0xA11CE5U) % 26);
            for (int k = 0; k < drop; ++k) {
                const int yy = cy - 1 - k;
                if (yy < Y_MIN) break;
                const double taper = 1.0 - (double(k) / std::max(1, drop));
                const int rr = std::max(1, iround(2.2 * taper));
                sphereShell(w, rx, yy, rz, rr, rr * 2, under);
                if (k % 7 == 0) set(w, rx, yy, rz, light);
            }
        }

        // Center marker
        sphereShell(w, cx, cy + 1, cz, 6, 4, light);
        sphereShell(w, cx, cy + 1, cz, 4, 4, rim);
    }

    static inline void buildCompassDial(editor::World& w, int cx, int cy, int cz) {
        const u16 stone = RB(lce::blocks::STONE_BRICKS_ID);
        const u16 obs   = RB(lce::blocks::OBSIDIAN_ID);
        const u16 gold  = RB(lce::blocks::GOLD_BLOCK_ID);
        const u16 lapis = RB(lce::blocks::LAPIS_LAZULI_BLOCK_ID);
        const u16 red   = RB(lce::blocks::REDSTONE_BLOCK_ID);
        const u16 emer  = RB(lce::blocks::EMERALD_BLOCK_ID);
        const u16 light = RB(lce::blocks::SEA_LANTERN_ID);

        // Big dial
        disc(w, cx, cy, cz, 38, stone);
        ring(w, cx, cy, cz, 38, 1, obs);
        ring(w, cx, cy, cz, 30, 1, light);
        ring(w, cx, cy, cz, 22, 1, obs);
        ring(w, cx, cy, cz, 14, 1, light);

        // Cardinal spikes (N E S W)
        for (int i = 0; i < 26; ++i) set(w, cx, cy, cz - (12 + i), lapis);
        for (int i = 0; i < 26; ++i) set(w, cx + (12 + i), cy, cz, red);
        for (int i = 0; i < 26; ++i) set(w, cx, cy, cz + (12 + i), gold);
        for (int i = 0; i < 26; ++i) set(w, cx - (12 + i), cy, cz, emer);

        // Clockwork rings above
        for (int y = cy + 1; y <= cy + 10; ++y) {
            const int r = 10 + (y - (cy + 1));
            ring(w, cx, y, cz, r, 1, (y % 2) ? obs : light);
        }

        // A simple “gate” frame (visual portal)
        boxHollow(w, cx - 6, cy + 1, cz + 18, cx + 6, cy + 16, cz + 18, obs);
        ring(w, cx, cy + 9, cz + 18, 6, 1, light);
        ring(w, cx, cy + 9, cz + 18, 4, 1, gold);
    }

    static inline void buildObservatory(editor::World& w, const Site& s) {
        const u16 quartz = RB(lce::blocks::QUARTZ_BLOCK_ID);
        const u16 lapis  = RB(lce::blocks::LAPIS_LAZULI_BLOCK_ID);
        const u16 light  = RB(lce::blocks::SEA_LANTERN_ID);
        const u16 glass  = RB(lce::blocks::GLASS_ID);

        // Tower core
        for (int y = s.y; y <= s.y + 86; ++y) {
            const double t = double(y - s.y) / 86.0;
            const int r = 8 + iround(6.0 * (1.0 - t));
            ring(w, s.x, y, s.z, r, 1, quartz);
            if ((y - s.y) % 9 == 0) ring(w, s.x, y, s.z, r - 2, 1, lapis);
            if ((y - s.y) % 7 == 0) ring(w, s.x, y, s.z, r - 3, 1, light);
        }

        // Dome
        for (int a = 0; a <= 90; ++a) {
            const double th = (PI * 0.5) * (double(a) / 90.0);
            const double rr = 18.0 * std::cos(th);
            const int yy = s.y + 86 + iround(18.0 * std::sin(th));
            ring(w, s.x, yy, s.z, std::max(1, iround(rr)), 1, glass);
            if (a % 7 == 0) ring(w, s.x, yy, s.z, std::max(1, iround(rr) - 2), 1, light);
        }

        // Star map disc above
        const int yMap = s.y + 112;
        disc(w, s.x, yMap, s.z, 34, glass);
        ring(w, s.x, yMap, s.z, 34, 1, light);

        for (int z = s.z - 30; z <= s.z + 30; ++z) {
            for (int x = s.x - 30; x <= s.x + 30; ++x) {
                const double r = std::hypot(double(x - s.x), double(z - s.z));
                if (r > 30.0) continue;

                const double a = std::atan2(double(z - s.z), double(x - s.x));
                const double f = 0.55 * std::sin(7.0 * a + 0.12 * r) + 0.45 * std::sin(0.08 * r * r);
                const double p = valueNoise(x, z, 0.18, 0x51A7U);
                if (p + f * 0.15 > 0.78) {
                    set(w, x, yMap + 1, z, light);
                    if (((hash2i(x, z, 0xBEEFCAFEU)) & 3U) == 0U) set(w, x, yMap + 2, z, lapis);
                }
            }
        }

        // Mural
        binaryMural(w,
                    s.x - 52, s.y + 10, s.z,
                    Facing::POS_X,
                    "ECHO 1: COUNT THE ARCS",
                    RB(lce::blocks::SEA_LANTERN_ID),
                    RB(lce::blocks::OBSIDIAN_ID),
                    RB(lce::blocks::LAPIS_LAZULI_BLOCK_ID),
                    1);
    }

    static inline void buildRedshiftTrench(editor::World& w, const Site& s) {
        const u16 obs   = RB(lce::blocks::OBSIDIAN_ID);
        const u16 red   = RB(lce::blocks::REDSTONE_BLOCK_ID);
        const u16 light = RB(lce::blocks::GLOWSTONE_ID);
        const u16 iron  = RB(lce::blocks::IRON_BLOCK_ID);

        // A spiral ramp down into the void-fragment
        const int turns = 7;
        const int steps = 1500;
        const double R0  = 32.0;
        const double R1  = 10.0;
        const int yTop   = s.y + 30;
        const int yBot   = 6;

        for (int i = 0; i < steps; ++i) {
            const double t = double(i) / double(steps - 1);
            const double ang = (2.0 * PI) * (turns * t);
            const double r = lerp(R0, R1, t);
            const int x = s.x + iround(r * std::cos(ang));
            const int z = s.z + iround(r * std::sin(ang));
            const int y = iround(lerp(double(yTop), double(yBot), t));

            // 3-wide path with rails of red
            for (int dx = -1; dx <= 1; ++dx) {
                for (int dz = -1; dz <= 1; ++dz) {
                    const int xx = x + dx;
                    const int zz = z + dz;
                    set(w, xx, y, zz, obs);
                    if (std::abs(dx) == 1 || std::abs(dz) == 1) set(w, xx, y + 1, zz, (i % 9 == 0) ? light : red);
                }
            }

            // Hanging “timing ticks”
            if (i % 37 == 0) {
                for (int k = 1; k <= 10; ++k) set(w, x, y - k, z, (k % 2) ? iron : obs);
            }
        }

        // A vault ring near the bottom
        const int vy = 10;
        ring(w, s.x, vy, s.z, 26, 1, iron);
        ring(w, s.x, vy + 1, s.z, 26, 1, red);
        ring(w, s.x, vy + 2, s.z, 26, 1, iron);

        // Mural
        binaryMural(w,
                    s.x, vy + 2, s.z + 40,
                    Facing::POS_Z,
                    "ECHO 2: FOLLOW THE RED DESCENT",
                    RB(lce::blocks::SEA_LANTERN_ID),
                    RB(lce::blocks::OBSIDIAN_ID),
                    RB(lce::blocks::REDSTONE_BLOCK_ID),
                    1);
    }

    static inline void buildPrismReef(editor::World& w, const Site& s) {
        const u16 prism = RB(lce::blocks::PRISMARINE_ID);
        const u16 dark  = RB(lce::blocks::OBSIDIAN_ID);
        const u16 light = RB(lce::blocks::SEA_LANTERN_ID);
        const u16 glass = RB(lce::blocks::GLASS_ID);
        const u16 water = RB(lce::blocks::STILL_WATER_ID);

        // Dome shell
        sphereShell(w, s.x, s.y + 24, s.z, 34, 7, glass);
        sphereShell(w, s.x, s.y + 24, s.z, 33, 6, prism);
        sphereShell(w, s.x, s.y + 24, s.z, 32, 6, light);

        // Shallow “reef basin” inside, but not filled completely (keeps block count sane)
        disc(w, s.x, s.y, s.z, 30, prism);
        ring(w, s.x, s.y, s.z, 30, 1, light);

        for (int z = s.z - 26; z <= s.z + 26; ++z) {
            for (int x = s.x - 26; x <= s.x + 26; ++x) {
                const double r = std::hypot(double(x - s.x), double(z - s.z));
                if (r > 26.0) continue;

                const double n = fbm(x, z, 0.08, 4, 0xC0A11U);
                const int h = 1 + std::max(0, iround(6.0 * (n + 0.2)));
                for (int k = 1; k <= h; ++k) set(w, x, s.y + k, z, water);

                // Coral-like lights as “glyph points”
                if (valueNoise(x, z, 0.18, 0xB00B1EU) > 0.83) {
                    set(w, x, s.y + h + 1, z, light);
                    set(w, x, s.y + h + 2, z, (hash2i(x, z, 0x77U) & 1U) ? dark : prism);
                }
            }
        }

        // Exterior “tide” rings
        for (int i = 0; i < 5; ++i) {
            const int y = s.y + 6 + i * 3;
            ring(w, s.x, y, s.z, 44 + i * 6, 1, (i % 2) ? prism : light);
        }

        // Mural
        binaryMural(w,
                    s.x - 60, s.y + 6, s.z - 6,
                    Facing::POS_X,
                    "ECHO 3: WATER IS A MIRROR",
                    RB(lce::blocks::SEA_LANTERN_ID),
                    RB(lce::blocks::OBSIDIAN_ID),
                    RB(lce::blocks::PRISMARINE_ID),
                    1);
    }

    static inline void buildVerdantOrchard(editor::World& w, const Site& s) {
        const u16 dirt  = RB(lce::blocks::DIRT_ID);
        const u16 grass = RB(lce::blocks::GRASS_ID);
        const u16 leaf  = RB(lce::blocks::OAK_LEAVES_ID);
        const u16 wood  = RB(lce::blocks::OAK_WOOD_ID);
        const u16 emer  = RB(lce::blocks::EMERALD_BLOCK_ID);
        const u16 light = RB(lce::blocks::SEA_LANTERN_ID);
        const u16 obs   = RB(lce::blocks::OBSIDIAN_ID);

        // Terraces
        for (int t = 0; t < 7; ++t) {
            const int r = 38 - t * 5;
            const int y = s.y + t * 3;
            disc(w, s.x, y, s.z, r, dirt);
            ring(w, s.x, y, s.z, r, 1, grass);
            ring(w, s.x, y + 1, s.z, r, 1, (t % 2) ? light : emer);
        }

        // Trees (stylized)
        const int trees = 12;
        const double R = 28.0;
        for (int i = 0; i < trees; ++i) {
            const double a = (2.0 * PI) * (double(i) / trees);
            const int x = s.x + iround(R * std::cos(a));
            const int z = s.z + iround(R * std::sin(a));

            const int h = 10 + (hash2i(x, z, 0xA0A0U) % 9);
            for (int k = 0; k < h; ++k) set(w, x, s.y + 1 + k, z, wood);

            // Canopy
            sphereShell(w, x, s.y + 1 + h, z, 6, 10, leaf);
            set(w, x, s.y + 1 + h, z, light);
        }

        // A Fibonacci path (a visible “route” clue)
        int fx = 1, fz = 1;
        int dir = 0;
        int px = s.x;
        int pz = s.z;
        for (int step = 0; step < 10; ++step) {
            const int len = fx;
            for (int i = 0; i < len; ++i) {
                set(w, px, s.y + 1, pz, obs);
                set(w, px, s.y + 2, pz, (i % 3 == 0) ? emer : light);
                if (dir == 0) px += 1;
                if (dir == 1) pz += 1;
                if (dir == 2) px -= 1;
                if (dir == 3) pz -= 1;
            }
            dir = (dir + 1) & 3;
            const int fn = fx + fz;
            fx = fz;
            fz = fn;
        }

        // Mural
        binaryMural(w,
                    s.x, s.y + 8, s.z - 60,
                    Facing::NEG_Z,
                    "ECHO 4: THE ROUTE IS THE PASSWORD",
                    RB(lce::blocks::SEA_LANTERN_ID),
                    RB(lce::blocks::OBSIDIAN_ID),
                    RB(lce::blocks::EMERALD_BLOCK_ID),
                    1);
    }

    static inline void buildOrigin(editor::World& w, const Site& s) {
        buildCompassDial(w, s.x, s.y, s.z);

        // The “first instruction” mural
        binaryMural(w,
                    s.x - 66, s.y + 3, s.z + 2,
                    Facing::POS_X,
                    "ECHO 0: SEEK NORTH SKY",
                    RB(lce::blocks::SEA_LANTERN_ID),
                    RB(lce::blocks::OBSIDIAN_ID),
                    RB(lce::blocks::GOLD_BLOCK_ID),
                    1);
    }

    static inline void buildSkyKnot(editor::World& w, int cx, int cy, int cz) {
        const u16 palA = RB(lce::blocks::SEA_LANTERN_ID);
        const u16 palB = RB(lce::blocks::LAPIS_LAZULI_BLOCK_ID);
        const u16 palC = RB(lce::blocks::REDSTONE_BLOCK_ID);
        const u16 palD = RB(lce::blocks::EMERALD_BLOCK_ID);

        const int steps = 5200;
        const double R = 120.0;
        const double r = 26.0;

        const int p = 2;
        const int q = 3;

        for (int i = 0; i < steps; ++i) {
            const double t = (2.0 * PI) * (double(i) / steps);

            const double ct = std::cos(p * t);
            const double st = std::sin(p * t);
            const double cq = std::cos(q * t);
            const double sq = std::sin(q * t);

            const double rr = R + r * cq;

            const int x = cx + iround(rr * ct);
            const int y = cy + iround(r * sq + 4.0 * std::sin(5.0 * t));
            const int z = cz + iround(rr * st);

            const double k = 0.5 + 0.5 * std::sin(4.0 * t);
            u16 raw = palA;
            if (k < 0.25) raw = palB;
            else if (k < 0.50) raw = palA;
            else if (k < 0.75) raw = palC;
            else raw = palD;

            sphereShell(w, x, y, z, 2, 5, raw);
        }

        // A single “axis needle” down to origin
        line3D(w, cx, cy, cz, cx, 10, cz, 1, RB(lce::blocks::GLOWSTONE_ID));
    }

    static inline void connectSites(editor::World& w, const Site& a, const Site& b, int y) {
        const u16 path = RB(lce::blocks::STONE_BRICKS_ID);
        const u16 edge = RB(lce::blocks::SEA_LANTERN_ID);

        const int x0 = a.x, z0 = a.z;
        const int x1 = b.x, z1 = b.z;

        const int dx = std::abs(x1 - x0);
        const int dz = std::abs(z1 - z0);
        const int steps = std::max(1, std::max(dx, dz));

        for (int i = 0; i <= steps; ++i) {
            const double t = double(i) / double(steps);
            const int x = iround(lerp(double(x0), double(x1), t));
            const int z = iround(lerp(double(z0), double(z1), t));

            // 5-wide bridge
            for (int ox = -2; ox <= 2; ++ox) {
                for (int oz = -2; oz <= 2; ++oz) {
                    const int xx = x + ox;
                    const int zz = z + oz;
                    set(w, xx, y, zz, path);

                    const bool border = (std::abs(ox) == 2 || std::abs(oz) == 2);
                    if (border && ((i % 11) == 0)) set(w, xx, y + 1, zz, edge);
                }
            }
        }
    }

    // ------------------------------------------------------------
    // World build
    // ------------------------------------------------------------
    static inline void generate(editor::World& w) {
        const Site ORIGIN {   0,  8,   0, "Origin Dial",
                          RB(lce::blocks::STONE_BRICKS_ID),
                          RB(lce::blocks::GOLD_BLOCK_ID),
                          RB(lce::blocks::SEA_LANTERN_ID) };

        const Site NORTH  {   0, 96, -260, "Lapis Observatory",
                         RB(lce::blocks::QUARTZ_BLOCK_ID),
                         RB(lce::blocks::LAPIS_LAZULI_BLOCK_ID),
                         RB(lce::blocks::SEA_LANTERN_ID) };

        const Site EAST   { 260, 38,   0, "Redshift Trench",
                        RB(lce::blocks::OBSIDIAN_ID),
                        RB(lce::blocks::REDSTONE_BLOCK_ID),
                        RB(lce::blocks::GLOWSTONE_ID) };

        const Site SOUTH  {   0, 34,  260, "Prism Reef",
                         RB(lce::blocks::PRISMARINE_ID),
                         RB(lce::blocks::GLASS_ID),
                         RB(lce::blocks::SEA_LANTERN_ID) };

        const Site WEST   {-260, 64,   0, "Verdant Orchard",
                        RB(lce::blocks::DIRT_ID),
                        RB(lce::blocks::EMERALD_BLOCK_ID),
                        RB(lce::blocks::SEA_LANTERN_ID) };

        // Platforms (fragments of the sim)
        buildFragmentPlatform(w, ORIGIN.x, ORIGIN.y, ORIGIN.z, 72,
                              RB(lce::blocks::STONE_BRICKS_ID),
                              RB(lce::blocks::OBSIDIAN_ID),
                              RB(lce::blocks::STONE_ID),
                              RB(lce::blocks::SEA_LANTERN_ID));

        buildFragmentPlatform(w, NORTH.x, NORTH.y, NORTH.z, 64,
                              RB(lce::blocks::QUARTZ_BLOCK_ID),
                              RB(lce::blocks::LAPIS_LAZULI_BLOCK_ID),
                              RB(lce::blocks::STONE_ID),
                              RB(lce::blocks::SEA_LANTERN_ID));

        buildFragmentPlatform(w, EAST.x, EAST.y, EAST.z, 60,
                              RB(lce::blocks::OBSIDIAN_ID),
                              RB(lce::blocks::REDSTONE_BLOCK_ID),
                              RB(lce::blocks::STONE_ID),
                              RB(lce::blocks::GLOWSTONE_ID));

        buildFragmentPlatform(w, SOUTH.x, SOUTH.y, SOUTH.z, 62,
                              RB(lce::blocks::PRISMARINE_ID),
                              RB(lce::blocks::GLASS_ID),
                              RB(lce::blocks::STONE_ID),
                              RB(lce::blocks::SEA_LANTERN_ID));

        buildFragmentPlatform(w, WEST.x, WEST.y, WEST.z, 66,
                              RB(lce::blocks::DIRT_ID),
                              RB(lce::blocks::EMERALD_BLOCK_ID),
                              RB(lce::blocks::STONE_ID),
                              RB(lce::blocks::SEA_LANTERN_ID));

        // Connective bridges (the “route” layer of the ARG)
        connectSites(w, ORIGIN, NORTH, ORIGIN.y);
        connectSites(w, ORIGIN, EAST,  ORIGIN.y);
        connectSites(w, ORIGIN, SOUTH, ORIGIN.y);
        connectSites(w, ORIGIN, WEST,  ORIGIN.y);

        // Build sites
        buildOrigin(w, ORIGIN);
        buildObservatory(w, NORTH);
        buildRedshiftTrench(w, EAST);
        buildPrismReef(w, SOUTH);
        buildVerdantOrchard(w, WEST);

        // Sky knot (visible from most places)
        buildSkyKnot(w, 0, 180, 0);

        // Final “vault” marker, subtle and not announced
        // The intended "end" is to realize the password is a route: walk the bridges in a specific order.
        // This vault is placed under origin as a reward space.
        {
            const u16 obs   = RB(lce::blocks::OBSIDIAN_ID);
            const u16 light = RB(lce::blocks::SEA_LANTERN_ID);
            const u16 iron  = RB(lce::blocks::IRON_BLOCK_ID);
            const int vx = 0, vz = 0;
            const int y0 = 4, y1 = 18;

            boxHollow(w, vx - 10, y0, vz - 10, vx + 10, y1, vz + 10, obs);
            ring(w, vx, y1, vz, 9, 1, iron);
            ring(w, vx, y1 - 2, vz, 7, 1, light);

            binaryMural(w,
                        vx - 34, y0 + 2, vz + 10,
                        Facing::POS_X,
                        "THE KEY IS A ROUTE",
                        RB(lce::blocks::SEA_LANTERN_ID),
                        RB(lce::blocks::OBSIDIAN_ID),
                        RB(lce::blocks::IRON_BLOCK_ID),
                        1);
        }
    }

} // namespace atlas

int main() {
    const std::string wiiu = R"(E:\Emulators\cemu_1.27.1\mlc01\usr\save\00050000\101d9d00\user\80000001\)";
    const std::string TEST_IN  = wiiu + "260104235224";

    constexpr auto consoleOut = lce::CONSOLE::WIIU;
    const std::string TEST_OUT = wiiu;

    editor::WriteSettings settings(editor::sch::AquaticTU69, consoleOut, TEST_OUT);

    editor::World world;

    if (world.read(TEST_IN) != 0) {
        cmn::log(cmn::eLog::error, "Failed to load file: {}\n", TEST_IN);
        return -1;
    }

    world.setWorldName(L"The Atlas Protocol");

    // Generate the entire ARG world into an empty (all-air) world.
    atlas::generate(world);

    const int statusOut = world.write(settings);
    if (statusOut != 0) {
        printf("converting to %s failed...\n", consoleToCStr(consoleOut));
        return statusOut;
    }

    printf("Finished!\nFile Out: %s", TEST_OUT.c_str());
    return 0;
}
