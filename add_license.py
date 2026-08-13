import os

HEADER = """// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.
"""

TARGET_EXTENSIONS = {
    '.h', '.hpp', '.cpp', '.c', 
    '.glsl', '.vert', '.frag', '.comp', '.geom', '.mesh', '.task'
}

EXCLUDE_DIRS = {
    'External',
    '.git',
    '.xmake',
    '.idea',
    'cmake-build-debug', 
    'cmake-build-release',
    'build', 
    'Docs'
}

def prepend_header(filepath):
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    if content.startswith("// Copyright (C) 2026"):
        return False

    with open(filepath, 'w', encoding='utf-8') as f:
        f.write(HEADER + "\n" + content)
    return True

def process_directory(root_dir):
    modified_count = 0
    for dirpath, dirnames, filenames in os.walk(root_dir):
        dirnames[:] = [d for d in dirnames if d not in EXCLUDE_DIRS]

        for filename in filenames:
            ext = os.path.splitext(filename)[1].lower()
            if ext in TARGET_EXTENSIONS:
                filepath = os.path.join(dirpath, filename)
                try:
                    if prepend_header(filepath):
                        print(f"Added header to: {filepath}")
                        modified_count += 1
                except Exception as e:
                    print(f"Error processing {filepath}: {e}")

    print(f"\nDone! Modified {modified_count} files.")

if __name__ == "__main__":
    process_directory(".")