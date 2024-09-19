# Small util script to run our shaders through glslang,
# targeting Vulkan 1.0
import subprocess
import sys

filename = sys.argv[1]
proc = subprocess.run(["glslang", filename, "--target-env", "vulkan1.2", '-o', f'{filename}.spv'],
            shell=False
            )
print(proc.args)
