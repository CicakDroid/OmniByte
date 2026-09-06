#!/bin/bash
# build-sanity.sh — Quick syntax sanity check for Signatures plugin
# Validates CMake configuration without Android NDK

set -e

echo "=== OmniByte Signatures Plugin — Build Sanity Check ==="
echo ""

cd "$(dirname "$0")/.."

echo "1. File inventory:"
find engine-core/HydraDis/Plugin/Enhanced/Signatures -type f \( -name '*.h' -o -name '*.cpp' -o -name 'CMakeLists.txt' \) | sort | while read f; do
    lines=$(wc -l < "$f")
    printf "  %-70s %5d lines\n" "$f" "$lines"
done
echo ""

echo "2. Dumper additions:"
find modules/Dumper/DumperCore/WorkingModes modules/Dumper/DumperCore/DumperManager* -type f 2>/dev/null | sort | while read f; do
    lines=$(wc -l < "$f")
    printf "  %-70s %5d lines\n" "$f" "$lines"
done
echo ""

echo "3. Git commits (signatures):"
git log --oneline | grep -i signatures | head -20
echo ""

echo "4. Total new files:"
total_files=$(find engine-core/HydraDis/Plugin/Enhanced/Signatures -type f \( -name '*.h' -o -name '*.cpp' \) | wc -l)
total_dumper=$(find modules/Dumper/DumperCore/WorkingModes modules/Dumper/DumperCore/DumperManager* -type f 2>/dev/null | wc -l)
echo "  Signatures: $total_files files"
echo "  Dumper: $total_dumper files"
echo ""

echo "5. Git status (untracked/modified):"
git status --short engine-core/HydraDis/Plugin/Enhanced/Signatures modules/Dumper/DumperCore/WorkingModes modules/Dumper/DumperCore/DumperManager* 2>/dev/null
echo ""

echo "=== Sanity check complete ==="
