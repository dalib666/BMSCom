Import("env")
import os
from shutil import copyfile

print("Current CLI targets", COMMAND_LINE_TARGETS)
print("Current Build targets", BUILD_TARGETS)

def after_build(source, target, env):
    print("Running post-build task...")

    # Get the path to the compiled firmware
    firmware_path = str(target[0])  # Path to the firmware binary
    output_dir = "out"  # Directory to copy the file

    # Create the output directory if it doesn't exist
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    # Copy the firmware to the output directory
    output_file = os.path.join(output_dir, os.path.basename(firmware_path))
    copyfile(firmware_path, output_file)

    print(f"Firmware copied to {output_file}")

#
# Upload actions
#


#
# register action after generation of files
#
env.AddPostAction("$BUILD_DIR/${PROGNAME}.elf", after_build)
env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", after_build)