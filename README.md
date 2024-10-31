# Command Line Application Library

I didn't like the available libraries for building command line applications in C++, so I wrote my own. This library is designed to be simple to use, and to provide a lot of functionality with minimal effort.

It was built largely as an exercise to use templates to build out the commands at compile time.

## Features

- [x] Commands / Subcommands
- [x] Help
- [ ] Flags

## Usage

Here is an example application that has a base command, and a subcommand:

```cpp
#include <iostream>
#include <OzzCommandLine/ozz_command_line.h>

struct BaseCommand {
    // delete all constructors
    BaseCommand() = delete;

    static bool ExecuteFunc(const std::vector<std::string>& tokens) {
        if (tokens.empty()) {
            return false;
        }

        if (CommandListType::execute(tokens)) {
            return true;
        }

        std::cout << "Tokens: ";
        for (const auto &token : tokens) {
            std::cout << token << " ";
        }
        std::cout << std::endl;
        return false;
    }

    static void AdditionalHelp(uint16_t indent) {
        CommandListType::HelpFunction(indent);
    }

    constexpr static const char* help_string = "This is the help string for base command zz";
    constexpr static const char* command_string = "base";

private:
    struct BaseSubcommand {
        BaseSubcommand() = delete;

        static bool ExecuteFunc(const std::vector<std::string>& tokens) {
            std::cout << "subcommand executed\n";
            return true;
        }

        static void AdditionalHelp(uint16_t indent) {
            std::cout << std::string(indent, ' ') << "Additional help for base subcommand" << std::endl;
        }

        constexpr static const char* help_string = "This is the help string for base subcommand";
        constexpr static const char* command_string = "subcommand";
    };

    using CommandListType = OzzCommandList<
                OzzCommand<BaseSubcommand>
            >;
};

int main(int argc, char *argv[]) {
    using CommandListType = OzzCommandList<
                OzzCommand<BaseCommand>
            >;

    if (!CommandListType::execute(argc, argv)) {
        std::cout << "Command not found\n";
    }
}
```