import glob, os, shutil, fileinput
import datetime

Import("env", "projenv")
print("Post build scripts > copy_elf.py")


def after_upload(source, target, env):
    env_name = str(source[0]).split("/")[-2]
    print(f"Copying file from action {target[0]} for {env_name}")

    dest_dir = f"test/esp32"
    os.makedirs(dest_dir, exist_ok=True)

    shutil.copyfile(f".pio/build/{env_name}/firmware.elf", f"test/esp32/{env_name}.elf")
    shutil.copyfile(f".pio/build/{env_name}/firmware.bin", f"test/esp32/{env_name}.bin")
    e = datetime.datetime.now()
    print (e.strftime("%H:%M:%S %d-%m-%Y"))
    
    
    
env.AddPostAction("upload", after_upload)
env.AddPostAction("buildprog", after_upload)