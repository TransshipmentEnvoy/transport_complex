import logging

from . import _if
from .util.upkeep import log

_logger = logging.getLogger(__name__)

def main():
    _if.init()

    log.log_init()
    log.enable_ext()

    # log.enable_console()

    _logger.info("x")
    _logger.warning("w")
    _logger.critical("z")
    import time
    time.sleep(0.1)
    log._logger.info("xx")
    log._logger.warning("ww")
    log._logger.critical("zz")
    log._log.reset_logging(log._log.log_param_t(log._log.log_type_t.file_only))
    log._logger.critical("zz")
    log._log.reset_logging(log._log.log_param_t(log._log.log_type_t.console_file))
    log._logger.critical("zzz")
    log._logger.info("xxx")

    _if.deinit()


if __name__ == "__main__":
    main()
