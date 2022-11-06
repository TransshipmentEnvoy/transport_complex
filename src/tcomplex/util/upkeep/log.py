import logging
import logging.handlers
import os

from typing import Optional
from .rotate_file import rotate
from .empty import keep_fist_fn

try:
    from termcolor import colored
except ImportError:
    colored = keep_fist_fn

# import ext
from . import _log

# module local logger
_logger = logging.getLogger(__name__)

# root logger
_root_logger = logging.getLogger()
_console_handler: Optional[logging.Handler] = None
_file_handler: dict[str, logging.Handler] = {}
_ext_handler: Optional[logging.Handler] = None
_queue_handler = None  # for multiprocessing


def log_init():
    _root_logger.setLevel(logging.DEBUG)


def enable_console(formatter=None):
    global _root_logger, _console_handler

    if _console_handler is not None:
        return

    _console_handler = logging.StreamHandler()
    _console_handler.setLevel(logging.INFO)
    if formatter is None:
        formatter = ConsoleFormatter()
    _console_handler.setFormatter(formatter)
    _root_logger.addHandler(_console_handler)


def disable_console():
    global _root_logger, _console_handler

    if _console_handler is None:
        return

    _root_logger.removeHandler(_console_handler)
    _console_handler = None


def enable_file(filename, formatter=None):
    global _root_logger, _file_handler

    filename = os.path.normcase(os.path.normcase(filename))
    if filename in _file_handler:
        return

    if os.path.exists(filename):
        rotate(filename)

    file_handler = logging.FileHandler(filename)
    if formatter is None:
        formatter = FileFormatter()
    file_handler.setFormatter(formatter)
    file_handler.setLevel(logging.DEBUG)

    _file_handler[filename] = file_handler
    _root_logger.addHandler(file_handler)


def disable_file(filename):
    global _root_logger, _file_handler

    filename = os.path.normcase(os.path.normcase(filename))
    if filename not in _file_handler:
        return

    file_handler = _file_handler.pop(filename)
    _root_logger.removeHandler(file_handler)


def enable_ext():
    global _ext_handler

    _log.reset_logging(_log.log_param_t(_log.log_type_t.console_file, "multisink2.txt"))

    if _ext_handler is not None:
        return

    _ext_handler = ExtHandler()
    _root_logger.addHandler(_ext_handler)


def disable_ext():
    global _ext_handler

    if _ext_handler is None:
        return

    _root_logger.removeHandler(_ext_handler)
    _ext_handler = None


def configure_mp_main():
    # TODO
    pass


def configure_mp_worker():
    # TODO
    pass


def deconfigure_mp_main():
    # TODO
    pass


# log handler
class ExtHandler(logging.Handler):
    def __init__(self) -> None:
        super().__init__()
        if not _log.check_init():
            raise Exception("logging not initialized in main library!")

        self.ctx = _log.LogCtx()

    def emit(self, record):
        self.ctx.log(
            record.levelno, record.getMessage(), record.created, record.filename, record.funcName, record.lineno
        )

    def flush(self):
        self.acquire()
        _log.flush_logging()
        self.release()


# log format
class Formatter(logging.Formatter):
    """Logging Formatter to add colors and count warning / errors"""

    time_str = "%(asctime)s"
    level_str = "[%(levelname)s]"
    msg_str = "%(message)s"
    src_str = "(%(name)s @ %(filename)s:%(lineno)d)"

    def format(self, record):
        # self.FORMATS defined in derived class
        log_fmt = self.FORMATS.get(record.levelno)
        formatter = logging.Formatter(log_fmt)
        return formatter.format(record)


class ConsoleFormatter(Formatter):
    FORMATS = {
        logging.DEBUG: colored(
            " ".join([Formatter.time_str, Formatter.level_str, Formatter.src_str, ""]), "cyan", attrs=["dark"]
        )
        + colored(Formatter.msg_str, "cyan"),
        logging.INFO: colored(
            " ".join([Formatter.time_str, Formatter.level_str, Formatter.src_str, ""]), "white", attrs=["dark"]
        )
        + colored(Formatter.msg_str, "white"),
        logging.WARNING: colored(
            " ".join([Formatter.time_str, Formatter.level_str, Formatter.src_str, ""]), "yellow", attrs=["dark"]
        )
        + colored(Formatter.msg_str, "yellow"),
        logging.ERROR: colored(
            " ".join([Formatter.time_str, Formatter.level_str, Formatter.src_str, ""]), "red", attrs=["dark"]
        )
        + colored(Formatter.msg_str, "red"),
        logging.CRITICAL: colored(
            " ".join([Formatter.time_str, Formatter.level_str, Formatter.src_str, ""]), "red", attrs=["dark", "bold"]
        )
        + colored(Formatter.msg_str, "red", attrs=["bold"]),
    }


class FileFormatter(Formatter):
    FORMATS = {
        logging.DEBUG: " ".join([Formatter.time_str, Formatter.level_str, Formatter.src_str, Formatter.msg_str]),
        logging.INFO: " ".join([Formatter.time_str, Formatter.level_str, Formatter.src_str, Formatter.msg_str]),
        logging.WARNING: " ".join([Formatter.time_str, Formatter.level_str, Formatter.src_str, Formatter.msg_str]),
        logging.ERROR: " ".join([Formatter.time_str, Formatter.level_str, Formatter.src_str, Formatter.msg_str]),
        logging.CRITICAL: " ".join([Formatter.time_str, Formatter.level_str, Formatter.src_str, Formatter.msg_str]),
    }


# interactive fn
def ask_for_confirm():
    global _console_handler
    if _console_handler is not None:
        _logger.warning("ask for confirmation!")
        while True:
            ret = input("confirm (y/n)?")
            if ret.upper() == "Y":
                _logger.info("-> confirm")
                return True
            elif ret.upper() == "N":
                _logger.info("-> exit")
                return False
    else:
        return True


# wrap module
log_type = _log.log_type_t


def reset_logging(log_type=None, log_filename=None):
    _log.reset_logging(_log.log_param_t(log_type=log_type, log_filename=log_filename))
