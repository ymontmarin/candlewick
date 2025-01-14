# Small util script to run our shaders through glslang,
# targeting Vulkan 1.0
import subprocess
import os.path
import argparse

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
    "--cross",
    action="store_true",
    help="Use shadercross to transpile the SPIR-V shader to MSL and produce JSON metadata",
)
args = parser.parse_args()

shader_name = args.shader_name
SHADER_SRC_DIR = os.path.abspath("shaders/src")
SHADER_OUT_DIR = os.path.abspath("shaders/compiled")
print("Shader src dir:", SHADER_SRC_DIR)
if args.all_stages:
    import glob

    stages = glob.glob(os.path.join(SHADER_SRC_DIR, f"{shader_name}.[a-z]*"))
else:
    stages = [
        os.path.join(SHADER_SRC_DIR, f"{shader_name}.{stage}") for stage in args.stages
    ]

print(f"Processing stages: {stages}")
assert len(stages) > 0, "No stages found!"

for stage_file in stages:
    assert os.path.exists(stage_file)
    spv_file = os.path.join(SHADER_OUT_DIR, os.path.relpath(stage_file, SHADER_SRC_DIR))
    spv_file = f"{spv_file}.spv"
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
    print(proc.args)

    if args.cross:
        for ext in ("json", "msl"):
            out_file = os.path.join(
                SHADER_OUT_DIR, os.path.relpath(stage_file, SHADER_SRC_DIR)
            )
            out_file = f"{out_file}.{ext}"
            proc2 = subprocess.run(
                [
                    "shadercross",
                    spv_file,
                    "-o",
                    out_file,
                    "-d",
                    ext.upper(),
                ],
                shell=False,
            )
            print(proc2)
