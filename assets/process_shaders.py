# Small util script to run our shaders through glslang,
# targeting Vulkan 1.0
import subprocess
import os.path
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("shader_name")
group = parser.add_mutually_exclusive_group(required=True)
group.add_argument("--stages", nargs="+", help="Shader stages to be compiled to SPIR-V.")
group.add_argument("--all-stages", "-a", action='store_true', help="Process all available shader stages.")
args = parser.parse_args()

shader_name = args.shader_name
if args.all_stages:
    import glob

    stages = glob.glob(f"{shader_name}.[a-z]*")
else:
    stages = [f"{shader_name}.{stage}" for stage in args.stages]

print(f"Processing stages: {stages}")

for stage_file in stages:
    assert os.path.exists(stage_file)
    out_file = f"shaders/compiled/{stage_file}.spv"
    proc = subprocess.run(
        ["glslang", stage_file, "--target-env", "vulkan1.2", "-o", out_file], shell=False
    )
    print(proc.args)
