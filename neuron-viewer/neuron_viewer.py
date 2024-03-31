# Simple UI debugging tool.
# 
# Tested under: Python 3.11
#
# Author: Antonov Alexandr (@Bismuth208)
#
# LICENSE: MIT


import time
import asyncio
import argparse
import sys


import numpy as np
import yaml
import json

from functools import lru_cache
from dataclasses import dataclass

from mqtt_service import MQTTService
from logs import *


from PySide6 import QtCore
from PySide6.QtCore import QTimer

from PySide6.QtWidgets import QApplication, QMainWindow
from PySide6.QtWidgets import QGridLayout, QWidget, QPushButton
from PySide6.QtGui import QPainter
from PySide6.QtCharts import QChart, QChartView, QLineSeries

from qasync import QEventLoop


from paho.mqtt import client as mqtt_client


# ------------------------------------------------------------------------ #
parser = argparse.ArgumentParser()

app_settings = None

mqtt_app: MQTTService = None
mqtt_core_topic: str = 'Pettie/Cells/'

ui_app: QApplication = None
ui_main_window = None

neuron_story_size: int = 32

# All 397 names of cells stored here and loaded from connectome.json
neurons: list = []

# ------------------------------------------------------------------------ #
class NeuronWindow(QMainWindow):
    def __init__(self, win_title: str):
        super().__init__()
  
        self.setWindowTitle(win_title)

        self.series = QLineSeries()
        
        self.chart = QChart()
        self.chart.legend().hide()
        
        self._chart_view = QChartView(self.chart)
        self._chart_view.setRenderHint(QPainter.RenderHint.Antialiasing)

        if sys.platform == 'darwin':
            # Dummy fix on MacOS and PySide6
            self._chart_view.viewport().setAttribute(QtCore.Qt.WidgetAttribute.WA_AcceptTouchEvents, False)

        self.setCentralWidget(self._chart_view)
        self.resize(320, 240)
        self.move(0, 0)


    def update_plot(self, points: list) -> None:
        if not self.series.count():
            for i, point in enumerate(points):
                self.series.append(i, point)

            self.chart.addSeries(self.series)
            self.chart.createDefaultAxes()
            self.chart.axes(orientation=QtCore.Qt.Orientation.Vertical)[0].setRange(0, 255)
        else:
            for i, point in enumerate(points):
                self.series.replace(i, i, point)


@dataclass
class NeuronUI:
    neuron_item: QPushButton
    neuron_graph: NeuronWindow
    name: str
    values: list
    val: str = ''


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()

        self.setWindowTitle('Pettie\'s Brain')

        self.after_ui_callback = None
        self.ui_neuron_labels: list[NeuronUI] = []

        row: int = 0
        col: int = 0

        grid = QGridLayout()
        grid.setHorizontalSpacing(5)       

        for neuron in neurons:
            neuron_item = QPushButton()
            neuron_item.setText(neuron)
            neuron_item.setStyleSheet('background-color: #1f1f1f')
            neuron_item.setMinimumWidth(50)
            neuron_item.setMinimumHeight(15)
            neuron_item.clicked.connect(self.show_neuron_graph)

            story = NeuronWindow(neuron)
            story.hide()

            grid.addWidget(neuron_item, row, col)

            self.ui_neuron_labels.append(NeuronUI(neuron_item, story, neuron, [int(0)] * neuron_story_size))

            # Why 18? Dunno, it was rolled on D20 dice :/
            col += 1
            if col > 18:
                col = 0
                row += 1

        stage = QWidget()
        stage.setLayout(grid)

        self.setCentralWidget(stage)

        self.updater_timer = QTimer()
        self.updater_timer.timeout.connect(self.neuron_graph_update)
        self.updater_timer.start(1000)


    def neuron_graph_update(self) -> None:
        for ui_neuron in ui_main_window.ui_neuron_labels:
            if not ui_neuron.neuron_graph.isHidden():
                ui_neuron.neuron_graph.update_plot(ui_neuron.values)


    def show_neuron_graph(self) -> None:
        # FIXME: using sender() is a bad practice
        neuron_name: str = self.sender().text()
        
        for ui_neuron in ui_main_window.ui_neuron_labels:
            if ui_neuron.name == neuron_name:
                if not ui_neuron.neuron_graph.isVisible():
                    ui_neuron.neuron_graph.show()
                    break
    

    def close_sub_windows(self) -> None:
        for ui_neuron in ui_main_window.ui_neuron_labels:
            if ui_neuron.neuron_graph.isVisible():
                ui_neuron.neuron_graph.close()


    def closeEvent(self, event):
        self.shutdown()
        event.accept()
 

    def shutdown(self, signal=None):
        try:
            if callable(self.after_ui_callback):
                self.after_ui_callback()

            self.close_sub_windows()
            self.close()
        except asyncio.InvalidStateError:
            raise SystemExit

# ------------------------------------------------------------------------ #
def app_load_settings(settings_path: str) -> dict:
    loaded_conf: dict = {}

    try:
        with open(settings_path) as f:
            loaded_conf = yaml.safe_load(f)  
    except Exception as _:
        logging.error('No App config found!')
        sys.exit(1)
        
    return loaded_conf


def app_load_connectocome(connectome_path: str) -> dict:
    connectome_dict: dict = {}
    
    try:
        with open(connectome_path, 'r') as f:
            connectome_dict = json.loads(f.read())
    except Exception as _:
        logging.error('No connectome.json found!')
        sys.exit(1)
        
    return connectome_dict


@lru_cache(maxsize=255)
def convert_cell_state(topic_data: str) -> tuple[str, int]:
    cell_value = float(topic_data)

    if cell_value < -128 or cell_value > 127:
        logging.warn('Invalid Input')
        raise ValueError("Value out of range")

    cell_state: int = int(np.interp(int(topic_data), [-128, 127], [0,255]))
    return f'background-color: #00{cell_state:02x}00', cell_state

# ------------------------------------------------------------------------ #
def mqtt_subscribe_callback(client: mqtt_client) -> None:
    for neuron in neurons:
        neuron_topic = mqtt_core_topic + neuron
        client.subscribe(neuron_topic)


def mqtt_parse_callback(topic_name: str, topic_data: str) -> None:
    for ui_neuron in ui_main_window.ui_neuron_labels:
        if ui_neuron.name == topic_name:
            new_cell_state_s, new_cell_state_i = convert_cell_state(topic_data)

            if new_cell_state_i != ui_neuron.values[0]:
                # shift data window
                ui_neuron.values[0] = new_cell_state_i
                ui_neuron.values = ui_neuron.values[neuron_story_size-1:] + ui_neuron.values[:neuron_story_size-1]

                ui_neuron.val = new_cell_state_s
                ui_neuron.neuron_item.setStyleSheet(new_cell_state_s)


# ------------------------------------------------------------------------ #
async def main_async() -> None:
    mqtt_app.connect()
    mqtt_app.start()


def ui_close_cleanup() -> None:
    mqtt_app.stop()


def init_neurons(connectome: dict) -> None:
    for neuron in connectome.keys():
        neurons.append(neuron)


if __name__ == '__main__':
    init_logs(sys.argv)

    parser.add_argument('-c', '--config', 
                        action='store',
                        default='./settings.yaml',
                        help='Pass the folder with server settings')
    
    parser.add_argument('-n', '--neurons', 
                        action='store',
                        default='./connectome.json',
                        help='Pass the folder with connectome')

    parsed_args = parser.parse_args()

    app_settings = app_load_settings(parsed_args.config)
    connectome_settings = app_load_connectocome(parsed_args.neurons)

    init_neurons(connectome_settings)

    mqtt_app = MQTTService(mqtt_config=app_settings["MQTT"])
    mqtt_app.subscribe_callback = mqtt_subscribe_callback
    mqtt_app.parse_callback = mqtt_parse_callback

    ui_app = QApplication([])
    loop = QEventLoop(ui_app)

    asyncio.set_event_loop(loop)

    ui_main_window = MainWindow()
    ui_main_window.after_ui_callback = ui_close_cleanup
    ui_main_window.show()

    loop.create_task(main_async())
    exit_code = loop.run_forever()

    logging.info(f'App finished with: {exit_code}')
    sys.exit(exit_code)
