import os
import shutil

def rename_and_merge(mapping_file):
    root_dir = os.getcwd()

    if not os.path.exists(mapping_file):
        print(f"Error: {mapping_file} not found in {root_dir}")
        return

    with open(mapping_file, 'r') as f:
        lines = [line.strip() for line in f if '|' in line]

    folder_conversions = {}

    print(f"Moving matched files...")
    for line in lines:
        parts = line.split('|')
        if len(parts) != 2:
            continue

        old_rel_path, target_rel_name = parts

        old_abs_path = os.path.join(root_dir, old_rel_path)

        if not os.path.exists(old_abs_path):
            continue

        old_dir = os.path.dirname(old_abs_path)

        target_parts = target_rel_name.replace('\\', '/').split('/')
        depth = len(target_parts)

        base_dir = old_abs_path
        for _ in range(depth):
            base_dir = os.path.dirname(base_dir)

        new_abs_path = os.path.join(base_dir, *target_parts)
        new_dir = os.path.dirname(new_abs_path)

        if old_dir.lower() == new_dir.lower() and old_dir != new_dir:
            folder_conversions[old_dir] = new_dir

        os.makedirs(new_dir, exist_ok=True)

        try:
            if old_abs_path != new_abs_path:
                shutil.move(old_abs_path, new_abs_path)
        except Exception as e:
            print(f"  Error moving file {old_rel_path}: {e}")

    print(f"Merging unmatched files from lowercase folders...")
    sorted_old_dirs = sorted(folder_conversions.keys(), key=len, reverse=True)

    for old_dir in sorted_old_dirs:
        new_dir = folder_conversions[old_dir]
        if os.path.exists(old_dir) and old_dir != new_dir:
            print(f"  Merging: {os.path.relpath(old_dir, root_dir)} -> {os.path.relpath(new_dir, root_dir)}")
            for filename in os.listdir(old_dir):
                old_file_path = os.path.join(old_dir, filename)
                new_file_path = os.path.join(new_dir, filename)

                os.makedirs(new_dir, exist_ok=True)

                try:
                    if not os.path.exists(new_file_path):
                        shutil.move(old_file_path, new_file_path)
                    else:
                        print(f"    Skipped {filename} (exists in {os.path.basename(new_dir)})")
                except Exception as e:
                    print(f"    Error merging {filename}: {e}")

    print(f"Cleaning up empty folders...")
    for old_dir in sorted_old_dirs:
        try:
            if os.path.exists(old_dir) and not os.listdir(old_dir):
                os.rmdir(old_dir)
                print(f"  Removed empty folder: {os.path.relpath(old_dir, root_dir)}")
        except Exception as e:
            pass

    print("\nProcess Complete!")

if __name__ == "__main__":
    rename_and_merge('rename_map.txt')
