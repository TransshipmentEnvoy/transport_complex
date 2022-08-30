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

