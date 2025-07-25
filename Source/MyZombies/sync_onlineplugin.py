import os
import shutil
from datetime import datetime

# === CONFIGURATION ===
# Path to the OnlinePlugin source files
SOURCE_H = r"C:\Documents\Unreal Projects\OnlinePlugin\Plugins\MultiplayerPlugin\Source\MultiplayerPlugin\Public\MultiplayerSessions.h"
SOURCE_CPP = r"C:\Documents\Unreal Projects\OnlinePlugin\Plugins\MultiplayerPlugin\Source\MultiplayerPlugin\Private\MultiplayerSessions.cpp"

# Path to the MyZombies plugin destination files
DEST_H = r"C:\Documents\Unreal Projects\MyZombies\Plugins\MultiplayerPlugin\Source\MultiplayerPlugin\Public\MultiplayerSessions.h"
DEST_CPP = r"C:\Documents\Unreal Projects\MyZombies\Plugins\MultiplayerPlugin\Source\MultiplayerPlugin\Private\MultiplayerSessions.cpp"

# Backup directory for old versions
BACKUP_DIR = r"C:\Documents\Unreal Projects\MyZombies\Scripts\MultiplayerPluginSyncBackups"

# List of (source, destination) file pairs
FILES = [
    (SOURCE_H, DEST_H),
    (SOURCE_CPP, DEST_CPP),
]

def ensure_dir(path):
    if not os.path.exists(path):
        os.makedirs(path)

def backup(dest):
    ensure_dir(BACKUP_DIR)
    base = os.path.basename(dest)
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    backup_name = f"{base}.{timestamp}.bak"
    shutil.copy2(dest, os.path.join(BACKUP_DIR, backup_name))

def sync(source, dest):
    if not os.path.exists(source):
        print(f"Source not found: {source}")
        return
    ensure_dir(os.path.dirname(dest))
    if os.path.exists(dest):
        if os.path.getmtime(source) <= os.path.getmtime(dest):
            print(f"No changes detected for {os.path.basename(source)}")
            return
        backup(dest)
    shutil.copy2(source, dest)
    print(f"Synced {os.path.basename(source)} -> {dest}")

def main():
    for src, dst in FILES:
        sync(src, dst)

if __name__ == "__main__":
    main()
