import os
import shutil

if __name__ == "__main__":
    currentPath = os.path.dirname(os.path.abspath(__file__))
    destPath = os.path.abspath(os.path.join(
        currentPath, '..', 'wulpus', 'production-frontend'))
    os.chdir(currentPath)

    print("Starting build process...")
    build_result = os.system("npm run build")

    if build_result != 0:
        print(
            f"Build failed with exit code {build_result}. Aborting copy operation.")
        exit(build_result)

    print("Build succeeded. Proceeding to copy files...")

    # Clearing the destination path if it exists, preserving .gitignore
    if os.path.exists(destPath):
        print(f"Clearing destination directory: {destPath}")
        for item in os.listdir(destPath):
            if item != '.gitignore':
                item_path = os.path.join(destPath, item)
                if os.path.isdir(item_path):
                    shutil.rmtree(item_path)
                else:
                    os.remove(item_path)

    print("Copying built files to production directory...")
    shutil.copytree("dist", destPath, dirs_exist_ok=True)
    print(f"Successfully copied files to {destPath}")
    print("Successfully Finished!")
