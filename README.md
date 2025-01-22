# Command Line Application Library

I didn't like the available libraries for building command line applications in C++, so I wrote my own. This library is designed to be simple to use, and to provide a lot of functionality with minimal effort.

It was built largely as an exercise to use templates to build out the commands at compile time.

## Features

- [x] Commands / Subcommands
- [x] Help
- [x] Flags

## Usage

There is an example application provided in `examples/main.cpp` that demonstrates all major features.

The gist:

There are two types:

1. OzzCommand
2. OzzCommandList

### OzzCommand

You create OzzCommand structs in your code base. Similar to inheritance but templated because why not. 
An OzzCommand requires an ExecuteFunc, an AdditionalHelp function, a help string and a command string.

```cpp
struct MainCommand {
    using SubCommands = OzzCommandList<OzzCommand<SubCommand>>;

    // delete all constructors
    MainCommand() = delete;

    static bool ExecuteFunc(const TokenListType &tokens, const FlagMapType &flags) {
        std::cout << "command\n";
        if (!SubCommands::execute(tokens, flags)) {
            std::cout << "Invalid subcommand\n";
        }
        return true;
    }

    static void AdditionalHelp(uint16_t indent) {
    }

    constexpr static const char *help_string = "This command does command things";
    constexpr static const char *command_string = "command c"; // <-- note you can specify multiple command
};
```

The idea being that each command can be self contained.

### OzzCommandList

In order to build your list of commands, you define a OzzCommandList type:

```cpp
using MyCommandList = OzzCommandList<OzzCommand<MainCommand>, OzzCommand<Command2>>;
```

You can then execute your function from the top level of your application:

```cpp
int main (int argc, char** argv) {
    if (!OzzCommandList<OzzCommand<MainCommand>>::execute(argc, argv)) {
        // command not found
    }
}
```