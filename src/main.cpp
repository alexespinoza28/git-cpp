#include "Commands.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>

using gitlet::commands::init;
using gitlet::commands::add;
using gitlet::commands::commit;
using gitlet::commands::remove;
using gitlet::commands::log;
using gitlet::commands::globalLog;
using gitlet::commands::find;
using gitlet::commands::status;
using gitlet::commands::restore;
using gitlet::commands::branch;
using gitlet::commands::switchBranch;
using gitlet::commands::rmBranch;
using gitlet::commands::reset;
using gitlet::commands::merge;

static void exitError(const std::string& msg) {
    std::cout << msg << "\n";
    std::exit(0);
}

int main(int argc, char** argv) {
    if (argc <= 1) {
        exitError("Please enter a command.");
    }

    std::string firstArg = argv[1];

    
    std::vector<std::string> args;
    args.reserve(argc - 2);
    for (int i = 2; i < argc; ++i) args.emplace_back(argv[i]);

    if (firstArg == "init") {
        init();

    } else if (firstArg == "add") {
        if (args.size() < 1) exitError("Missing file operand.");
        add(args[0]);

    } else if (firstArg == "commit") {
        if (args.size() < 1) exitError("Please enter a commit message.");
        if (args.size() > 1) exitError("please enter a valid command");
        commit(args[0]);

    } else if (firstArg == "rm") {
        if (args.size() < 1) exitError("Missing file operand.");
        remove(args[0]);

    } else if (firstArg == "log") {
        log();

    } else if (firstArg == "global-log") {
        globalLog();

    } else if (firstArg == "find") {
        if (args.size() < 1) exitError("Missing message operand.");
        find(args[0]);

    } else if (firstArg == "status") {
        status();

    } else if (firstArg == "restore") {
        // Mirror your Java checks:
        // if (args.length == 4 & !args[2].equals("--")) -> Incorrect operands.
        // Here we just pass argv tail and let restore() validate.
        {
            std::vector<std::string> full;
            full.reserve(argc - 1);
            for (int i = 1; i < argc; ++i) full.emplace_back(argv[i]);
            restore(full);
        }

    } else if (firstArg == "branch") {
        if (args.size() < 1) exitError("Missing branch name.");
        branch(args[0]);

    } else if (firstArg == "switch") {
        if (args.size() < 1) exitError("Missing branch name.");
        switchBranch(args[0], "");

    } else if (firstArg == "rm-branch") {
        if (args.size() < 1) exitError("Missing branch name.");
        rmBranch(args[0]);

    } else if (firstArg == "reset") {
        if (args.size() < 1) exitError("Missing commit id.");
        reset(args[0]); // internally call switchBranch(commitId, "commit")

    } else if (firstArg == "merge") {
        if (args.size() < 1) exitError("Missing branch name.");
        merge(args[0]);

    } else {
        exitError("No command with that name exists.");
    }

    return 0;
}
