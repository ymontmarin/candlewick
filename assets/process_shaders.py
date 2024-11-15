# Small util script to run our shaders through glslang,
# targeting Vulkan 1.0
import subprocess
import sys

filename = sys.argv[1]
out_file = f"shaders/compiled/{filename}.spv"
proc = subprocess.run(
    ["glslang", filename, "--target-env", "vulkan1.2", "-o", out_file], shell=False
)
print(proc.args)
