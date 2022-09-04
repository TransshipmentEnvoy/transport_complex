from .dep import setup_env
import subprocess
import os
import sys


def spawn():
    env = os.environ.copy()
    setup_env(env)

    # spawn the game process
    main_process = subprocess.Popen(
        [sys.executable, "-m", "tcomplex.main"] + sys.argv[1:],
        env=env,
    )
    ret = main_process.wait()
    return ret


def gdb_spawn():
    env = os.environ.copy()
    setup_env(env)

    # spawn the game process
    main_process = subprocess.Popen(
        ["gdb", "-q", "--args", sys.executable, "-m", "tcomplex.main"] + sys.argv[1:],
        env=env,
    )
    ret = main_process.wait()
    return ret


if __name__ == "__main__":
    ret = spawn()
    exit(ret)
