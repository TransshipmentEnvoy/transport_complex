from .dep import setup_env
import subprocess
import os
import sys


def spawn():
    setup_env()

    # spawn the game process
    main_process = subprocess.Popen(
        [sys.executable, "-m", "tcomplex.main"] + sys.argv[1:],
        env=os.environ.copy(),
    )
    ret = main_process.wait()
    return ret


def gdb_spawn():
    setup_env()

    # spawn the game process
    main_process = subprocess.Popen(
        ["gdb", "-q", "--args", sys.executable, "-m", "tcomplex.main"] + sys.argv[1:],
        env=os.environ.copy(),
    )
    ret = main_process.wait()
    return ret


if __name__ == "__main__":
    ret = spawn()
    exit(ret)
