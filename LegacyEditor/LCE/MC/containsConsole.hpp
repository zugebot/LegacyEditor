#pragma once
#include "enums.hpp"

class ContainsConsole {
    CONSOLE console = CONSOLE::NONE;

public:
    ContainsConsole() {
        setConsole(CONSOLE::NONE);
    }

    explicit ContainsConsole(const CONSOLE consoleIn) {
        setConsole(consoleIn);
    }

    void setConsole(const CONSOLE consoleIn) {
        console = consoleIn;
    }

    ND CONSOLE getConsole() const {
        return console;
    }
};