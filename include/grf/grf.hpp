#pragma once

#include "grfHeader.hpp"

#include "grfHeader.hpp"
#include <algorithm>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

/**
 * Entire Game Rule File structure (header + rules + embedded files).
 * Provides a C++20‑style API similar to the original C# class.
 */
class GameRuleFile {
public:
    // ─── Static rule/parameter whitelists (same text as C#) ──────────────
    static inline const std::vector<std::string_view> ValidGameRules = {
            "MapOptions", "ApplySchematic", "GenerateStructure", "GenerateBox", "PlaceBlock", "PlaceContainer",
            "PlaceSpawner", "BiomeOverride", "StartFeature", "AddItem", "AddEnchantment", "WeighedTreasureItem",
            "RandomItemSet", "DistributeItems", "WorldPosition", "LevelRules", "NamedArea", "ActiveChunkArea",
            "TargetArea", "ScoreRing", "ThermalArea", "PlayerBoundsVolume", "Killbox", "BlockLayer", "UseBlock",
            "CollectItem", "CompleteAll", "UpdatePlayer", "OnGameStartSpawnPositions", "OnInitialiseWorld",
            "SpawnPositionSet", "PopulateContainer", "DegradationSequence", "RandomDissolveDegrade",
            "DirectionalDegrade", "GrantPermissions", "AllowIn", "LayerGeneration", "LayerAsset", "AnyCombinationOf",
            "CombinationDefinition", "Variations", "BlockDef", "LayerSize", "UniformSize", "RandomizeSize",
            "LinearBlendSize", "LayerShape", "BasicShape", "StarShape", "PatchyShape", "RingShape", "SpiralShape",
            "LayerFill", "BasicLayerFill", "CurvedLayerFill", "WarpedLayerFill", "LayerTheme", "NullTheme",
            "FilterTheme", "ShaftsTheme", "BasicPatchesTheme", "BlockStackTheme", "RainbowTheme", "TerracottaTheme",
            "FunctionPatchesTheme", "SimplePatchesTheme", "CarpetTrapTheme", "MushroomBlockTheme", "TextureTheme",
            "SchematicTheme", "BlockCollisionException", "Powerup", "Checkpoint", "CustomBeacon", "ActiveViewArea",
    };

    static inline const std::vector<std::string_view> ValidParameters = {
            /* huge list omitted for brevity – copy from C# if needed */
    };

    // ─── Nested structures ───────────────────────────────────────────────
    struct FileEntry {
        std::string             Name;
        std::vector<uint8_t>    Data;
        FileEntry(std::string n, std::vector<uint8_t> d) : Name(std::move(n)), Data(std::move(d)) {}
    };

    class GameRule {
    public:
        std::string                          Name;
        GameRule*                            Parent = nullptr;
        std::unordered_map<std::string,std::string> Parameters;
        std::vector<GameRule>                Children;

        explicit GameRule(std::string n = "", GameRule* p = nullptr)
            : Name(std::move(n)), Parent(p) {}

        // Create and return a child rule.  If validate == true the rule name
        // must appear in ValidGameRules.
        GameRule& AddRule(const std::string& ruleName, bool validate = false) {
            if (validate && std::find(ValidGameRules.begin(), ValidGameRules.end(), ruleName) == ValidGameRules.end())
                throw std::invalid_argument("Invalid game‑rule name: " + ruleName);
            Children.emplace_back(ruleName, this);
            return Children.back();
        }
        GameRule& AddRule(const std::string& ruleName, std::initializer_list<std::pair<std::string,std::string>> params) {
            GameRule& r = AddRule(ruleName, false);
            for (auto&& kv : params) r.Parameters[kv.first] = kv.second;
            return r;
        }
    };

    // ─── Public data ─────────────────────────────────────────────────────
    GameRuleFileHeader  Header;
    GameRule            Root;           ///< Always exists; name "__ROOT__"
    std::vector<FileEntry> Files;

    // ─── Construction ────────────────────────────────────────────────────
    GameRuleFile() : Header{}, Root("__ROOT__", nullptr) {}
    explicit GameRuleFile(GameRuleFileHeader hdr) : Header(std::move(hdr)), Root("__ROOT__", nullptr) {}

    // ─── Convenience methods mirroring the C# API ───────────────────────
    void AddFile(const std::string& name, const std::vector<uint8_t>& data) {
        Files.emplace_back(name, data);
    }

    template<class Iter>
    void AddGameRules(Iter first, Iter last) { Root.Children.insert(Root.Children.end(), first, last); }

    GameRule& AddRule(const std::string& name, bool validate = false)             { return Root.AddRule(name, validate); }
    GameRule& AddRule(const std::string& name, std::initializer_list<std::pair<std::string,std::string>> params) {
        return Root.AddRule(name, params);
    }
};
