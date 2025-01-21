#include <iostream>
#include <ozz_command_line.h>

using namespace ozz::commands;

struct SubCommand {
    // delete all constructors
    SubCommand() = delete;

    static bool ExecuteFunc(const TokenListType &tokens, const FlagMapType &flags) {
        std::cout << "subcommand\n";

        // Print all flags
        for (const auto &flag : flags) {
            std::cout << flag.first << "\n";
        }
        std::unordered_map<int, int> map;
        map[0] = 1;

        const auto param = std::get<std::string>(flags.at("paramname"));
        std::cout << param << "\n";

        return true;
    }

    static void AdditionalHelp(uint16_t indent) {
        // CommandListType::HelpFunction(indent);
    }

    constexpr static const char *help_string = "This subcommand does subcommand things";
    constexpr static const char *command_string = "sub s"; // <-- note you can specify multiple command
};

// OzzCommand
struct ExampleCommand {
    // delete all constructors
    ExampleCommand() = delete;

    static bool ExecuteFunc(const TokenListType &tokens, const FlagMapType &flags) {
        // if (tokens.empty()) {
        //     std::cout << "Require subcommand\n";
        //     return false;
        // }

        if (CommandListType::execute(tokens, flags)) {
            return true;
        }

        return false;
    }

    static void AdditionalHelp(uint16_t indent) {
        CommandListType::HelpFunction(indent);
    }

    constexpr static const char *help_string = "This command does command things";
    constexpr static const char *command_string = "command c"; // <-- note you can specify multiple command

private:
    using CommandListType = OzzCommandList<
        OzzCommand<SubCommand>
    >;
};

int main(int argc, char *argv[]) {
    if (!OzzCommandList<OzzCommand<ExampleCommand>, OzzCommand<SubCommand>>::execute(argc, argv)) {
        std::cout << "Command not found";
    }
    return 0;
}
