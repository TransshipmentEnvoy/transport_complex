# from . import _if
import os

print(os.environ["LD_LIBRARY_PATH"])


def main():
    from . import _if

    print(_if)


if __name__ == "__main__":
    main()
