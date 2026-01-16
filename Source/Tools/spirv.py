import argparse
import subprocess
from pathlib import Path
import sys


def main():
    parser = argparse.ArgumentParser(description="Compile HLSL to SPIR-V using DXC")
    parser.add_argument("input", help="Path to input HLSL file")
    parser.add_argument(
        "-o",
        "--output",
        default="PixelShader.frag.spv",
        help="Output SPIR-V file (default: PixelShader.frag.spv)",
    )
    parser.add_argument(
        "--dxc",
        default="dxc.exe",
        help="Path to dxc.exe (default: dxc.exe in PATH)",
    )

    args = parser.parse_args()

    input_path = Path(args.input).resolve()
    output_path = Path(args.output).resolve()

    if not input_path.exists():
        print(f"Error: input file does not exist: {input_path}")
        sys.exit(1)

    cmd = [
        args.dxc,
        "-T", "ps_6_0",
        "-E", "main",
        "-spirv",
        "-fspv-target-env=vulkan1.3",
        "-fspv-extension=SPV_KHR_physical_storage_buffer",
        "-fspv-extension=SPV_EXT_descriptor_indexing",
        "-fvk-use-dx-layout",
        "-O3",
        "-Fo", str(output_path),
        str(input_path),
    ]

    print("Running DXC:")
    print(" ".join(cmd))

    result = subprocess.run(cmd)

    if result.returncode != 0:
        print(f"DXC failed with exit code {result.returncode}")
        sys.exit(result.returncode)

    print(f"Compilation succeeded: {output_path}")


if __name__ == "__main__":
    main()
