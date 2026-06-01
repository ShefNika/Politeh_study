import os
import re
import csv
import sys
from collections import defaultdict

PACKAGE_RE = re.compile(r'^\s*package\s+([\w.]+)\s*;', re.MULTILINE) # строки типа package com.yandex.mail
LOAD_LIBRARY_RE = re.compile(r'System\.loadLibrary\s*\(\s*"([^"]+)"\s*\)')
SYSTEM_LOAD_RE = re.compile(r'System\.load\s*\(\s*([^)]+)\)')
NATIVE_RE = re.compile(
    r'(?P<mods>(?:public|private|protected|static|final|synchronized|abstract|strictfp|\s)+)?'
    r'\bnative\s+'
    r'(?P<ret>[\w.$<>\[\], ?]+)\s+'
    r'(?P<name>\w+)\s*'
    r'\((?P<params>[^)]*)\)\s*;',
    re.MULTILINE
)

def normalize_spaces(s: str) -> str:
    return " ".join((s or "").split())

def classify_sink(method_name: str, class_name: str, package_name: str):
    text = f"{package_name}.{class_name}.{method_name}".lower()
    if any(x in text for x in ["sqlite", "sql", "query", "bind", "column", "step", "exec"]):
        return "SQL/native database operation"
    if any(x in text for x in ["socket", "connect", "network", "send", "recv", "receive", "webrtc", "jingle", "peerconnection"]):
        return "Network/data transfer"
    if any(x in text for x in ["audio", "speech", "voice", "opus", "record", "microphone"]):
        return "Audio/speech processing"
    if any(x in text for x in ["camera", "image", "jpeg", "bitmap", "yuv", "scanner", "ocr", "eye"]):
        return "Image/camera processing"
    if any(x in text for x in ["file", "open", "read", "write", "path", "save"]):
        return "File read/write operation"
    if any(x in text for x in ["encrypt", "decrypt", "cipher", "hash", "ssl", "tls", "crypto"]):
        return "Crypto/TLS operation"
    if any(x in text for x in ["init", "create", "new", "alloc"]):
        return "Native object creation/initialization"
    if any(x in text for x in ["free", "destroy", "release", "close", "dispose"]):
        return "Native resource release"
    if any(x in text for x in ["parse", "decode", "encode", "deserialize"]):
        return "Parsing / encoding / decoding"
    return "Generic native operation"


def classify_library(package_name: str, class_name: str, libraries: list[str]) -> str:
    text = " ".join([package_name, class_name] + libraries).lower()
    if "speech" in text or "speechkit" in text:
        return "Yandex SpeechKit/voice"
    if "webrtc" in text or "jingle" in text or "peerconnection" in text or "telemost" in text:
        return "calls/Telemost"
    if "eye" in text or "camera" in text or "scanner" in text or "ocr" in text:
        return "Camera/scanner"
    if "sqlite" in text or "sql" in text:
        return "SQLite/database"
    if "appmetrica" in text or "metrica" in text or "crash" in text:
        return "AppMetrica/crash analytics"
    return "Unknown"


def parse_file(path: str):
    with open(path, "r", encoding="utf-8", errors="ignore") as f:
        content = f.read()
    package_match = PACKAGE_RE.search(content)
    package_name = package_match.group(1) if package_match else "unknown"
    class_name = os.path.basename(path).replace(".java", "")
    load_libraries = LOAD_LIBRARY_RE.findall(content)
    system_loads = SYSTEM_LOAD_RE.findall(content)
    results = []
    for match in NATIVE_RE.finditer(content):
        line = content[:match.start()].count("\n") + 1
        method_name = match.group("name")
        return_type = normalize_spaces(match.group("ret"))
        params = normalize_spaces(match.group("params"))
        mods = normalize_spaces(match.group("mods"))
        declaration = f"{mods} native {return_type} {method_name}({params});"
        declaration = normalize_spaces(declaration)
        call_pattern = re.compile(r'\b' + re.escape(method_name) + r'\s*\(')
        call_count = max(0, len(call_pattern.findall(content)) - 1)
        library_group = classify_library(package_name, class_name, load_libraries)
        sink_category = classify_sink(method_name, class_name, package_name)
        results.append({
            "file": path,
            "line": line,
            "package": package_name,
            "class": class_name,
            "method": method_name,
            "return_type": return_type,
            "params": params,
            "modifiers": mods,
            "declaration": declaration,
            "load_libraries": ", ".join(load_libraries),
            "system_loads": ", ".join(system_loads),
            "library_group": library_group,
            "sink_category": sink_category,
            "call_count_in_same_file": call_count,
        })
    return results


def scan_sources(root: str):
    all_results = []
    for dirpath, _, filenames in os.walk(root): # возвращает путь к текущей папке, список подпапок и список имен в текущей папке
        for filename in filenames:
            if filename.endswith(".java"):
                path = os.path.join(dirpath, filename)
                all_results.extend(parse_file(path))
    return all_results


def save_csv(results, output_csv):
    fields = [
        "package",
        "class",
        "method",
        "declaration",
        "file",
        "line",
        "load_libraries",
        "system_loads",
        "library_group",
        "sink_category",
        "call_count_in_same_file",
    ]
    with open(output_csv, "w", encoding="utf-8-sig", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fields, delimiter=";")
        writer.writeheader()
        for item in results:
            writer.writerow({field: item.get(field, "") for field in fields})


def main():
    root = 'C:/lab5/yandex_mail_src/sources'
    if not os.path.isdir(root):
        print(f"Directory not found: {root}")
        sys.exit(1)
    results = scan_sources(root)
    output_csv = "native_functions.csv"
    save_csv(results, output_csv)
    print(f"Total native methods found: {len(results)}")
    print("\nSummary by library group:")
    by_library = defaultdict(int)
    for item in results:
        by_library[item["library_group"]] += 1
    for group, count in sorted(by_library.items(), key=lambda x: x[1], reverse=True):
        print(f"  {group}: {count}")
    print("\nSummary by sink category:")
    by_sink = defaultdict(int)
    for item in results:
        by_sink[item["sink_category"]] += 1
    for group, count in sorted(by_sink.items(), key=lambda x: x[1], reverse=True):
        print(f"  {group}: {count}")


if __name__ == "__main__":
    main()