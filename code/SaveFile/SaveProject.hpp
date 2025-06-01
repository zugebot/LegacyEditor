#pragma once

#include "code/ConsoleParser/helpers/detectConsole.hpp"
#include "code/ConsoleParser/helpers/makeParserForConsole.hpp"
#include "code/DisplayMetadata/DisplayMetadata.hpp"
#include "code/SaveFile/fileListing.hpp"

namespace editor {

    class SaveProject {
    public:
        fs::path m_tempFolder;

        DisplayMetadata m_displayMetadata;
        StateSettings m_stateSettings;

        std::list<LCEFile> m_allFiles;
        i32 m_oldestVersion{};
        i32 m_currentVersion{};

        // move to state settings grr
        i32 m_isNewGen = false;

        MU void setOldestVersion(i32 theVersion) { m_oldestVersion = theVersion; }
        MU ND i32 oldestVersion() const { return m_oldestVersion; }

        MU void setCurrentVersion(i32 theVersion) { m_currentVersion = theVersion; }
        MU ND i32 currentVersion() const { return m_currentVersion; }

        void setNewGen(bool isNewGen) { m_isNewGen = isNewGen; }
        bool isNewGen() const { return m_isNewGen; }




        int read(const fs::path& theFilePath);
        int write(WriteSettings& theWriteSettings);


        MU void printDetails() const;
        MU void printFileList() const;
        MU ND int dumpToFolder(const std::string& detail) const;

        static lce::CONSOLE detectConsole(const fs::path& savePath);








        ND size_t size() const { return m_allFiles.size(); }


        template<class... Args>
        LCEFile& emplaceFile(Args&&... args) {
            m_allFiles.emplace_back(std::forward<Args>(args)...);
            return m_allFiles.back();
        }


        auto begin()             { return m_allFiles.begin(); }
        auto end()               { return m_allFiles.end();   }
        MU ND auto begin()   const     { return m_allFiles.begin(); }
        MU ND auto end()     const     { return m_allFiles.end();   }

        template<class SetT>
        auto view_of(const SetT& keepTypes) {
            using std::views::filter;
            return m_allFiles | filter([&keepTypes](const LCEFile& f) {
                       return keepTypes.contains(f.m_fileType);
                   });
        }
        template<class SetT>
        ND auto view_of(const SetT& keepTypes) const {
            using std::views::filter;
            return m_allFiles | filter([&keepTypes](const LCEFile& f) {
                       return keepTypes.contains(f.m_fileType);
                   });
        }

        auto view_of(std::initializer_list<lce::FILETYPE> iList) {
            static_assert(std::is_same_v<lce::FILETYPE, lce::FILETYPE>, "This version is for FILETYPE only.");
            std::shared_ptr<std::set<lce::FILETYPE>> keys = std::make_shared<std::set<lce::FILETYPE>>(iList);
            using std::views::filter;

            return m_allFiles | filter([keys](const LCEFile& f) {
                       return keys->contains(f.m_fileType);
                   });
        }

        ND auto view_of(std::initializer_list<lce::FILETYPE> iList) const {
            std::shared_ptr<std::set<lce::FILETYPE>> keys = std::make_shared<std::set<lce::FILETYPE>>(iList);
            using std::views::filter;

            return m_allFiles | filter([keys](const LCEFile& f) {
                       return keys->contains(f.m_fileType);
                   });
        }

        std::optional<std::reference_wrapper<LCEFile>>
        findFile(lce::FILETYPE want) {
            for (auto &f : m_allFiles)
                if (f.m_fileType == want)
                    return std::ref(f);
            return std::nullopt;
        }

        ND std::optional<std::reference_wrapper<const LCEFile>>
        findFile(lce::FILETYPE want) const {
            for (auto &f : m_allFiles)
                if (f.m_fileType == want)
                    return std::cref(f);
            return std::nullopt;
        }

        ND std::size_t countFiles(lce::FILETYPE t) const {
            return std::ranges::count_if(
                    m_allFiles,
                    [t](auto const& f) { return f.m_fileType == t; });
        }
        void removeFileTypes(const std::set<lce::FILETYPE>& typesToRemove);
        MU void addFiles(std::list<LCEFile>&& filesIn);
        MU std::list<LCEFile> collectFiles(lce::FILETYPE fileType);
        MU std::list<LCEFile> collectFiles(const std::set<lce::FILETYPE>& typesToCollect);
    };

} // namespace editor
