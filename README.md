# Transport Complex

## Development Guide

1. Setup dev environment

    0. (optional) select python version with pyenv
    1. create venv in project root

        ```bash
        python -m venv .venv --prompt tcomplex
        . .venv/bin/activate
        ```

    2. update `pip`, `setuptools` and `pip-tools`

        ```bash
        pip install --upgrade pip setuptools pip-tools
        ```

2. (Optional) Re-generate `requirements.txt` and `requirements_dev.txt`:

    ```bash
    pip-compile requirements.in
    pip-compile requirements_dev.in
    ```

3. Sync environment

## Development Plan

+ [ ] get rid of `dep_spdlog` and spwan new process for launch
+ [ ] bundle `spdlog` as `clib`
+ [ ] handle the rpath
   1. for packaged wheels
   2. for develop build
+ [ ] (windows) handle dll searching (runtime `PATH` handling should work)
