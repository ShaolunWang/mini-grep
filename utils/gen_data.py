#!/usr/bin/env python3
import argparse

def generate_file(path, size_mb, pattern, stride):
    total_size = size_mb * 1024  * 1024
    chunk_size = 1024 * 1024

    pattern_bytes = pattern.encode()
    pattern_len = len(pattern_bytes)

    with open(path, "wb") as f:
        written = 0

        while written < total_size:
            buf = bytearray(b'x' * chunk_size)

            for i in range(0, chunk_size - pattern_len, stride):
                buf[i:i + pattern_len] = pattern_bytes

            to_write = min(chunk_size, total_size - written)
            f.write(buf[:to_write])
            written += to_write

    print(f"Generated {path} ({size_mb} mb, stride={stride})")


def main():
    parser = argparse.ArgumentParser(description="Generate benchmark input file")
    parser.add_argument("output", help="Output file path")
    parser.add_argument("--size", type=int, default=16, help="Size in MB")
    parser.add_argument("--pattern", type=str, default="abcdef", help="Pattern to embed")
    parser.add_argument("--stride", type=int, default=4096,
                        help="Distance between pattern insertions")

    args = parser.parse_args()

    generate_file(args.output, args.size, args.pattern, args.stride)


if __name__ == "__main__":
    main()
