import logging

from . import _if
from .util.upkeep import log

_logger = logging.getLogger(__name__)

def main():
    _if.init()

    log.log_init()
    log.enable_ext()

    _logger.info("x")
    _logger.warning("w")
    _logger.critical("z")

    _if.deinit()


if __name__ == "__main__":
    main()
