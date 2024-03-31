#

import os
import logging

from datetime import datetime

# ------------------------------------------------------------------------ #
def init_logs(args: list[str]) -> None:
    if '--debug' in args:
        logging_level = logging.DEBUG
    else:
        logging_level = logging.INFO

    logs_path: str = "./logs"

    def init_logs_dir() -> str:
        if not os.path.exists(logs_path):
            os.mkdir(logs_path)
            
        return datetime.now().strftime('logs/pettie_%H_%M_%d_%m_%Y.log')

    logging.basicConfig(level=logging_level, format="%(asctime)s %(levelname)s %(message)s")

    file_handler = logging.FileHandler(init_logs_dir())
    file_handler.setLevel(logging_level)
    file_handler.setFormatter(logging.Formatter('%(asctime)s %(levelname)s %(message)s'))
    logging.root.addHandler(file_handler)