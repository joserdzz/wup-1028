#!/usr/bin/env python3
import sys
from pathlib import Path

UF2_MAGIC_START0 = 0x0A324655
UF2_MAGIC_START1 = 0x9E5D5157
UF2_MAGIC_END = 0x0AB16F30
UF2_FLAG_FAMILY_ID_PRESENT = 0x00002000
RP2040_FAMILY_ID = 0xE48BFF56
FLASH_BASE = 0x10000000
PAYLOAD_SIZE = 256


def pad(data: bytes, size: int) -> bytes:
    return data + bytes(size - len(data))


def main() -> int:
    if len(sys.argv) != 3:
        print("usage: bin_to_uf2.py input.bin output.uf2", file=sys.stderr)
        return 2

    data = Path(sys.argv[1]).read_bytes()
    block_count = (len(data) + PAYLOAD_SIZE - 1) // PAYLOAD_SIZE
    blocks = []

    for block_no in range(block_count):
        offset = block_no * PAYLOAD_SIZE
        chunk = pad(data[offset:offset + PAYLOAD_SIZE], PAYLOAD_SIZE)
        target_addr = FLASH_BASE + offset

        header = (
            UF2_MAGIC_START0.to_bytes(4, "little")
            + UF2_MAGIC_START1.to_bytes(4, "little")
            + UF2_FLAG_FAMILY_ID_PRESENT.to_bytes(4, "little")
            + target_addr.to_bytes(4, "little")
            + PAYLOAD_SIZE.to_bytes(4, "little")
            + block_no.to_bytes(4, "little")
            + block_count.to_bytes(4, "little")
            + RP2040_FAMILY_ID.to_bytes(4, "little")
        )
        blocks.append(header + chunk + bytes(512 - 32 - PAYLOAD_SIZE - 4) + UF2_MAGIC_END.to_bytes(4, "little"))

    Path(sys.argv[2]).write_bytes(b"".join(blocks))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
