# gitcpp

A C++ implementation of Git's core functionality, originally ported from a Java "gitlet" project.

## Building

```bash
cmake -B build && make -C build
```

### Manual Install

```bash
# Build first
cmake -B build && make -C build

# Install to system PATH
sudo cp build/gitcpp /usr/local/bin/
```

### Alternative: Add to PATH

```bash
# Add to your ~/.zshrc or ~/.bashrc
export PATH="$PATH:/path/to/git_cpp/build"
```

## Usage

After installation:

```bash
gitcpp <command> [args...]
```

Or without installation:

```bash
./build/gitcpp <command> [args...]
```

Available commands: init, add, commit, rm, log, global-log, find, status, restore, branch, switch, rm-branch, reset, merge
