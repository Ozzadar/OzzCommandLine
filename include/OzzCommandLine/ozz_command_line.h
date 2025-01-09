//
// Created by paulm on 23/08/24.
//
#pragma once

#include <vector>
#include <iostream>
#include <sstream>

namespace ozz::commands {
    template<typename T>
    concept HasName = requires(T) {
        { T::command_string } -> std::convertible_to<std::string>;
    };

    template<typename T>
    concept HasExecute = requires(T) {
        { T::Execute } -> std::invocable<const std::vector<std::string> &>;
    };

    template<typename T>
    concept HasExecuteFunc = requires(T) {
        { T::ExecuteFunc } -> std::invocable<const std::vector<std::string> &>;
    };

    template<typename T>
    concept HasHelpString = requires(T) {
        { T::help_string } -> std::convertible_to<std::string>;
    };

    template<typename T>
    concept HasAdditionalHelp = requires(T) {
        { T::AdditionalHelp } -> std::invocable<uint16_t>;
    };

    template<typename CommandClass> requires HasName<CommandClass> && HasExecuteFunc<CommandClass> &&
                                             HasHelpString<CommandClass> && HasAdditionalHelp<CommandClass>
    class Command {
    public:
        Command() = default;

        static bool Execute(const std::vector<std::string> &tokens) {
            if (tokens.empty()) {
                return false;
            }

            std::vector<std::string> commandTokens;
            // tokenize command_string using space as delimiter
            std::istringstream iss(CommandClass::command_string);
            for (std::string token; std::getline(iss, token, ' ');) {
                commandTokens.push_back(token);
            }

            // if the first token is the command string, execute func with all remaining tokens
            if (std::find(commandTokens.begin(), commandTokens.end(), tokens[0]) != commandTokens.end()) {
                // take a subarray of tokens starting from the second element
                std::vector<std::string> subArguments(tokens.begin() + 1, tokens.end());
                return CommandClass::ExecuteFunc(subArguments);
            }
            return false;
        }

        constexpr static const char *command_string = CommandClass::command_string;
        constexpr static const char *help_string = CommandClass::help_string;

        static void PrintHelp(uint16_t indent) {
            std::cout << std::string(indent, ' ') << command_string << "\n"
                      << std::string(indent + 4, ' ') << help_string << "\n\n";
        }

        static void AdditionalHelp(uint16_t indent) {
            CommandClass::AdditionalHelp(indent);
        }
    };

    template<typename... Commands> requires (
    (HasName<Commands> && HasExecute<Commands> && HasHelpString<Commands>) && ...)

    struct CommandList {
        static bool execute(int argc, char *argv[]) {
            std::vector<std::string> tokens;
            for (int i = 1; i < argc; i++) {
                tokens.emplace_back(argv[i]);
            }

            return execute(tokens);
        }

        static bool execute(std::vector<std::string> tokens) {
            if (tokens.empty()) {
                return false;
            }

            if (tokens[0] == "help") {
                HelpFunction(0);
                return true;
            }

            for (const auto& result: {Commands::Execute(tokens)...}) {
                if (result) {
                    return true;
                }
            }
            return false;
        }

        static bool HelpFunction(uint16_t indent) {
            ((Commands::PrintHelp(indent), Commands::AdditionalHelp(indent + 4)), ...);
            return false;
        }

    private:
    };
} // namespace ozz::commands

template <typename... T>
using OzzCommandList = ozz::commands::CommandList<T...>;
template <typename T>
using OzzCommand = ozz::commands::Command<T>;
