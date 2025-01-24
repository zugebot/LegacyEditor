#include "include/lce/processor.hpp"

#include "code/include.hpp"
#include "unit_tests.hpp"
#include "include/sfo/sfo.hpp"


int main() {
    PREPARE_UNIT_TESTS();


    auto [fst, snd] = TESTS["RPCS3_1.00"];
    fs::path paramPath = fst;
    paramPath /= "PARAM.SFO";
    SFOManager sfo(paramPath.string());
    auto attrs = sfo.getAttributes();
    for (auto& attr : attrs) {
        std::cout << attr.toString() << std::endl;
    }





}