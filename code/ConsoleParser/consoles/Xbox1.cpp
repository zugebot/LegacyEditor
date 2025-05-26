#include "Xbox1.hpp"


#include "code/FileListing/fileListing.hpp"
#include "code/SaveProject/SaveProject.hpp"

namespace editor {


    int Xbox1::inflateFromLayout(const fs::path& theFilePath, MU SaveProject* saveProject) {
        m_filePath = theFilePath;
        return NOT_IMPLEMENTED;
    }


    int Xbox1::inflateListing(MU SaveProject* saveProject) {
        return NOT_IMPLEMENTED;
    }


    int Xbox1::deflateToSave(MU SaveProject* saveProject, MU WriteSettings& theSettings) const {
        printf("Xbox1.write(): not implemented!\n");
        return NOT_IMPLEMENTED;
    }


    int Xbox1::deflateListing(MU const fs::path& gameDataPath, MU Buffer& inflatedData, MU Buffer& deflatedData) const {
        return NOT_IMPLEMENTED;
    }

}