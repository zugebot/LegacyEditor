#pragma once

#include <ranges>

#include "include/lce/processor.hpp"
#include "include/ghc/fs_std.hpp"

#include "stateSettings.hpp"
#include "writeSettings.hpp"

#include "code/LCEFile/LCEFile.hpp"
#include "common/error_status.hpp"


namespace editor {


    class FileListing {
        std::list<LCEFile> m_allFiles;
        i32 m_oldestVersion{};
        i32 m_currentVersion{};
        i32 m_isNewGen = false;

    public:



        /// Constructors

        FileListing() = default;
        ~FileListing();

        /// Modify State

        MU void setOldestVersion(i32 theVersion) { m_oldestVersion = theVersion; }
        MU ND i32 oldestVersion() const { return m_oldestVersion; }

        MU void setCurrentVersion(i32 theVersion) { m_currentVersion = theVersion; }
        MU ND i32 currentVersion() const { return m_currentVersion; }

        void setNewGen(bool isNewGen) { m_isNewGen = isNewGen; }
        bool isNewGen() const { return m_isNewGen; }

        // file stuff

        void deallocate();


        // processing

        ND int readListing(const Buffer & bufferIn, lce::CONSOLE consoleIn);
        ND Buffer writeListing(StateSettings& stateSettings, WriteSettings& writeSettings);
        MU ND int preprocess(StateSettings& stateSettings, WriteSettings& theWriteSettings);

        /// Region Helpers

        MU void convertRegions(lce::CONSOLE consoleOut);
        MU void pruneRegions();

        // accessors

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
            static const std::set<lce::FILETYPE> tmp;
            std::set<lce::FILETYPE> keys(iList);
            return view_of(keys);
        }
        ND auto view_of(std::initializer_list<lce::FILETYPE> iList) const {
            std::set<lce::FILETYPE> keys(iList);
            return view_of(keys);
        }

        std::optional<std::reference_wrapper<LCEFile>>
        findFile(lce::FILETYPE want) {
            for (auto &f : m_allFiles)
                if (f.m_fileType == want)
                    return std::ref(f);
            return std::nullopt;
        }

        std::optional<std::reference_wrapper<const LCEFile>>
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
}

