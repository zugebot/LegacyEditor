#pragma once

#include "code/ConsoleParser/helpers/detectConsole.hpp"
#include "code/ConsoleParser/helpers/makeParserForConsole.hpp"
#include "code/DisplayMetadata/DisplayMetadata.hpp"
#include "code/SaveFile/fileListing.hpp"

namespace editor {

    class SaveProject {
    public:
        static const std::set<lce::FILETYPE> s_OLD_REGION_ANY;

        static const std::set<lce::FILETYPE> s_NEW_REGION_ANY;

        static const std::set<lce::FILETYPE> s_ENTITY_ANY;





        fs::path m_tempFolder;

        DisplayMetadata m_displayMetadata;
        StateSettings m_stateSettings;

        std::list<LCEFile> m_allFiles;
        i32 m_oldestVersion{};
        i32 m_latestVersion{};

        // move to state settings grr
        i32 m_isNewGen = false;

        MU void setOldestVersion(i32 theVersion) { m_oldestVersion = theVersion; }
        MU ND i32 oldestVersion() const { return m_oldestVersion; }

        MU void setLatestVersion(i32 theVersion) { m_latestVersion = theVersion; }
        MU ND i32 latestVersion() const { return m_latestVersion; }

        MU void setNewGen(bool isNewGen) { m_isNewGen = isNewGen; }
        MU ND bool isNewGen() const { return m_isNewGen; }




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
        MU ND auto begin() const { return m_allFiles.begin(); }
        MU ND auto end()   const { return m_allFiles.end();   }


    private:
        template<class Pred>
        auto as_view(Pred&& p) {
            using std::views::filter;
            return m_allFiles | filter(std::forward<Pred>(p));
        }

        template<class Pred>
        auto as_view(Pred&& p) const {
            using std::views::filter;
            return m_allFiles | filter(std::forward<Pred>(p));
        }

    public:

        template<class SetT>
        auto view_of(const SetT& keep) {
            return as_view([&keep](const LCEFile& f) { return keep.contains(f.m_fileType); });
        }

        template<class SetT>
        ND auto view_of(const SetT& keep) const {
            return as_view([&keep](const LCEFile& f) { return keep.contains(f.m_fileType); });
        }

        auto view_of(lce::FILETYPE type) {
            // I know this leaks but who cares
            const std::set<lce::FILETYPE>& ref =
                    (type == lce::FILETYPE::OLD_REGION_ANY) ? s_OLD_REGION_ANY :
                    (type == lce::FILETYPE::NEW_REGION_ANY) ? s_NEW_REGION_ANY :
                    (type == lce::FILETYPE::ENTITY_ANY    ) ? s_ENTITY_ANY     :
                    *(new std::set<lce::FILETYPE>{type});

            return view_of(ref);
        }

        ND auto view_of(lce::FILETYPE type) const { return const_cast<SaveProject*>(this)->view_of(type); }

        auto view_of(std::initializer_list<lce::FILETYPE> list) {
            auto keys = std::make_shared<std::set<lce::FILETYPE>>(list);
            return as_view([keys](const LCEFile& f) { return keys->contains(f.m_fileType); });
        }

        ND auto view_of(std::initializer_list<lce::FILETYPE> list) const {
            auto keys = std::make_shared<std::set<lce::FILETYPE>>(list);
            return as_view([keys](const LCEFile& f) { return keys->contains(f.m_fileType); });
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
