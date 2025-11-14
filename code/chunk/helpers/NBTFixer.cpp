#include "NBTFixer.hpp"

#include "common/data/GetExecutablePath.hpp"

namespace editor {

    std::unordered_set<std::string> NBTFixer::s_crashItems;

    bool NBTFixer::s_configLoaded = false;

    NBTFixer NBTFixer::s_instance;

    NBTFixer::NBTFixer() {
        if (!s_configLoaded) {
            try {
                Yaml::Node root;
                const fs::path exe_path = ExecutablePath::getExecutableDir();
                Yaml::Parse(root, (exe_path / "replace_ids.yaml").string());

                // std::cout << root["replace_ids"][0].As<std::string>() << std::endl;

                Yaml::Node& item = root["replace_ids"];
                for (int i = 0; i < item.Size(); i++) {
                    auto val = root["replace_ids"][i].As<std::string>();
                    s_crashItems.insert(val);
                    // std::cout << val << "\n";
                }
            } catch (const std::exception& e) {
                std::cerr << "Failed to load replace_ids.yaml: " << e.what() << "\n";
            }
            s_configLoaded = true;
        }
    }

}
