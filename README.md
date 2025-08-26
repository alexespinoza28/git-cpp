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
