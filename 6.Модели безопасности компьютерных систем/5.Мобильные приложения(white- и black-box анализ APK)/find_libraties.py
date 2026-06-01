import zipfile
import os

apk_path = r"C:\lab5\apk\Yandex_Mail_8.78.1_APKPure.apk"

with zipfile.ZipFile(apk_path, "r") as apk:
    so_files = [name for name in apk.namelist() if name.startswith("lib/") and name.endswith(".so")]

print(f"Found .so files: {len(so_files)}")
print()

for so in so_files:
    abi = so.split("/")[1] if "/" in so else "unknown"
    name = os.path.basename(so)
    print(f"{abi} | {name} | {so}")