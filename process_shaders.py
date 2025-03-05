# Small util script to run our shaders through glslc
# https://github.com/google/shaderc
import subprocess
import argparse
import pathlib as pt

parser = argparse.ArgumentParser()
parser.add_argument("shader_name")
group = parser.add_mutually_exclusive_group(required=True)
group.add_argument(
    "--stages", nargs="+", help="Shader stages to be compiled to SPIR-V."
)
group.add_argument(
    "--all-stages",
    "-a",
    action="store_true",
    help="Process all available shader stages.",
)
parser.add_argument(
    "--no-cross",
    action="store_true",
    help="Skip the SPIR-V -> MSL transpiling and JSON metadata step.",
)
args = parser.parse_args()

shader_name = args.shader_name
SHADER_SRC_DIR = pt.Path("shaders/src")
SHADER_OUT_DIR = pt.Path("shaders/compiled")
print("Shader src dir:", SHADER_SRC_DIR.absolute())
if args.all_stages:
    stages = list(SHADER_SRC_DIR.glob(f"{shader_name}.[a-z]*"))
else:
    stages = [SHADER_SRC_DIR / f"{shader_name}.{stage}" for stage in args.stages]

print(f"Processing files: {stages}")
assert len(stages) > 0, "No stages found!"

for stage_file in stages:
    assert stage_file.exists()
    spv_file = SHADER_OUT_DIR / stage_file.name
    spv_file = spv_file.with_suffix(spv_file.suffix + ".spv")
    proc = subprocess.run(
        [
            "glslc",
            stage_file,
            f"-I{SHADER_SRC_DIR}",
            "--target-env=vulkan1.2",
            "-Werror",
            "-o",
            spv_file,
        ],
        shell=False,
    )
    print(f"Compiling SPV file {spv_file}")

    if not args.no_cross:
        for ext in (".json", ".msl"):
            out_file = spv_file.with_suffix(ext)
            proc2 = subprocess.run(
                [
                    "shadercross",
                    spv_file,
                    "-o",
                    out_file,
                    "--msl-version",
                    "2.1.0",
                ],
                shell=False,
            )

if args.no_cross:
    print("Skipping SPIR-V -> MSL transpiling and JSON metadata steps.")
