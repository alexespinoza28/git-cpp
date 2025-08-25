# gitcpp

A C++ implementation of Git's core functionality, originally ported from a Java "gitcpp" project.

## Building

```bash
cmake -B build && make -C build
```

## Usage

```bash
./build/gitcpp <command> [args...]
```

Available commands: init, add, commit, rm, log, global-log, find, status, restore, branch, switch, rm-branch, reset, merge
