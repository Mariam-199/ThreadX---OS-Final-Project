#!/bin/bash

# =============================================================
# boot.sh
# -------
# This script prepares the Operating Systems final project.
# It cleans old builds, recreates folders, compiles all modules,
# reports success/failure, and finally runs the main menu.
# =============================================================

PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"
SRC_DIR="$PROJECT_ROOT/src"
BIN_DIR="$PROJECT_ROOT/bin"
LOG_DIR="$PROJECT_ROOT/logs"
FILES_DIR="$PROJECT_ROOT/files"

printf "\n=== Cleaning old build folders ===\n"
rm -rf "$BIN_DIR" "$LOG_DIR"

printf "\n=== Creating required folders ===\n"
mkdir -p "$BIN_DIR" "$LOG_DIR" "$FILES_DIR"

compile_module() {
    local source_file="$1"
    local output_file="$2"
    local extra_flags="$3"

    printf "Compiling %s ... " "$source_file"

    if gcc "$SRC_DIR/$source_file" -o "$BIN_DIR/$output_file" $extra_flags; then
        echo "SUCCESS"
    else
        echo "FAILED"
    fi
}

printf "\n=== Compiling logger first ===\n"
compile_module "logger.c" "logger" ""

printf "\n=== Compiling other modules ===\n"
compile_module "file_management.c" "file_management" ""
compile_module "peterson.c" "peterson" ""
compile_module "memory.c" "memory" ""
compile_module "amdahl.c" "amdahl" ""
compile_module "main_menu.c" "main_menu" ""

printf "\n=== Starting project ===\n"
cd "$BIN_DIR" || exit 1
./main_menu
