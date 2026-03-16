//
// Created by paulm on 23/08/24.
//
#pragma once

#include <algorithm>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <sstream>
#include <variant>
#include <cstdint>

namespace ozz::commands {
    using TokenListType = std::vector<std::string>;
    using FlagMapType = std::unordered_map<std::string, std::variant<std::string, bool>>;

    struct Flag {
        std::string flag;
        std::variant<std::string, bool> value;
    };

    template<typename T>
    concept HasName = requires(T) {
        { T::command_string } -> std::convertible_to<std::string>;
    };

    template<typename T>
    concept HasExecute = requires(T) {
        { T::Execute } -> std::invocable<const TokenListType&, const FlagMapType&>;
    };

    template<typename T>
    concept HasExecuteFunc = requires(T) {
        { T::ExecuteFunc } -> std::invocable<const TokenListType&, const FlagMapType&>;
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

        static bool Execute(const TokenListType& tokens, const FlagMapType& flags = {}) {
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
            if (std::ranges::find(commandTokens, tokens[0]) != commandTokens.end()) {
                // take a subarray of tokens starting from the second element
                std::vector<std::string> subArguments(tokens.begin() + 1, tokens.end());
                return CommandClass::ExecuteFunc(subArguments, flags);
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

        static bool execute(const TokenListType& tokens, const FlagMapType& flags = {}) {
            if (tokens.empty()) {
                return false;
            }

            if (tokens[0] == "help") {
                HelpFunction(0);
                return true;
            }

            // if no flags are passed, parse them from the tokens.
            bool bParseFlags = flags.empty();

            /**
             * Flags are stored in a vector of Flag structs
             * Supported flag formats:
             *  --key=value
             *  --booleanFlag
             *  -key value
             */

            // make a copy of the tokens
            TokenListType commandTokens = bParseFlags ? std::vector<std::string>{} : std::vector<std::string>{tokens.begin(), tokens.end()};
            FlagMapType outFlags = bParseFlags ? std::unordered_map<std::string, std::variant<std::string, bool>>{} : flags;

            if (bParseFlags) {
                std::string cachedKey {};
                bool bNextTokenIsValue { false };

                for (const auto& token : tokens) {
                    if (bNextTokenIsValue) {
                        outFlags[cachedKey] = token;
                        bNextTokenIsValue = false;
                        cachedKey.clear();
                        continue;
                    }

                    for (auto i = 0; i < token.size(); i++) {
                        // first character is a flag
                        if (i == 0) {
                            if ('-' == token[i]) {
                                continue;
                            }
                            // first character is not flag, so whole token is a command
                            commandTokens.push_back(token);
                            break;
                        }

                        if (i == 1) {
                            // second character is a flag, so we're expecting a key=value to be the remainder of the token
                            if ('-' == token[i]) {
                                // split the remainder of the token by =
                                auto split = token.find('=');
                                if (split != std::string::npos) {
                                    auto key = token.substr(2, split - 2);
                                    auto value = token.substr(split + 1);
                                    outFlags[key] = value;
                                    break;
                                }
                                // if no = is found, treat it as a boolean flag
                                outFlags[token.substr(2)] = true;
                                break;
                            }

                            if (token[0] == '-') {
                                cachedKey = token.substr(1);
                                bNextTokenIsValue = true;
                                break;
                            }
                            commandTokens.push_back(token);

                        }
                    }
                }
            }

            // separate flags from arguments
            for (const auto& result: {Commands::Execute(commandTokens, outFlags)...}) {
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
