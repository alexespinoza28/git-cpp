# gitcpp

A C++ implementation of Git's core functionality, originally ported from a Java "gitlet" project. This is a fully functional version control system that handles repositories, commits, branches, merging, and more.

## Platform Support

Currently tested and working on **macOS**. Windows and Linux support coming soon.

**TODO:** Add Windows and Linux compatibility

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

## Commands

### Basic Operations

- `init` - Initialize a new repository
- `add <file>` - Stage files for commit
- `commit <message>` - Create a new commit
- `rm <file>` - Remove files from staging and working directory
- `status` - Show repository status

### History & Information

- `log` - Show commit history for current branch
- `global-log` - Show all commits across all branches
- `find <message>` - Find commits by message

### Branching & Merging

- `branch <name>` - Create a new branch
- `switch <name>` - Switch to a branch
- `rm-branch <name>` - Delete a branch
- `merge <branch>` - Merge another branch into current branch

### Advanced

- `restore <file>` - Restore files from commits
- `reset <commit>` - Reset to a specific commit

### Configuration

- `config <key> <value>` - Set configuration values
  - `gitcpp config user.name "Your Name"`
  - `gitcpp config user.email "your@email.com"`

## Features

### .gitcppignore Support

Create a `.gitcppignore` file in your repository root to ignore files and directories:

```
*.log
temp/
build/
.DS_Store
```

Supports basic patterns:

- `*.extension` - Ignore all files with specific extension
- `directory/` - Ignore entire directories
- `filename` - Ignore specific files

### Merge Conflicts

When merging branches with conflicting changes, gitcpp will create conflict markers in affected files:

```
<<<<<<< HEAD
Current branch content
=======
Other branch content
>>>>>>> branch_name
```

Resolve conflicts by editing the file, then `add` and `commit` the resolved version.

## Repository Structure

gitcpp stores all data in a `.gitcpp/` directory:

- `commits/` - Commit objects
- `blob_files/` - File content storage
- `heads/` - Branch pointers
- `staged_files/` - Staging area
- `config/` - Configuration files

## Quick Demo

Try this 5-minute walkthrough to see gitcpp in action:

```bash
# Build the project
cmake -B build && make -C build

# Create a test directory
mkdir gitcpp_demo && cd gitcpp_demo

# Initialize repository
../build/gitcpp init

# Set up user info
../build/gitcpp config user.name "Demo User"
../build/gitcpp config user.email "demo@example.com"

# Create and add a file
echo "Hello gitcpp!" > hello.txt
../build/gitcpp add hello.txt
../build/gitcpp commit "Initial commit"

# Create a branch and make changes
../build/gitcpp branch feature
../build/gitcpp switch feature
echo "Feature branch changes" >> hello.txt
../build/gitcpp add hello.txt
../build/gitcpp commit "Add feature"

# Switch back and merge
../build/gitcpp switch main
../build/gitcpp merge feature

# Check the result
../build/gitcpp log
cat hello.txt
```

Expected output: You should see both commits in the log and the merged content in `hello.txt`.

### Test .gitcppignore

```bash
# Create files to ignore
echo "debug info" > debug.log
mkdir temp && echo "temp file" > temp/cache.txt

# Create ignore file
echo "*.log" > .gitcppignore
echo "temp/" >> .gitcppignore

# Check status - ignored files won't show up
../build/gitcpp status
```

## Testing

The project includes Google Test-based unit tests to verify functionality:

```bash
# Build tests (included in main build)
cmake -B build && make -C build

# Run tests (work in progress - some tests need isolation fixes)
./build/tests/gitcpp_tests
```

**Note:** Test suite is currently being refined to properly isolate test environments.
