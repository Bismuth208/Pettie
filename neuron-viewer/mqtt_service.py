#!/usr/bin/python
# coding:utf-8
# 
# Created: 28.01.2024
# 
# author: Antonov Alexandr (Bismuth208)
#
# Tested on:
#  Python 3.11.6


import asyncio
import logging

import paho.mqtt.client as mqtt_client

from asyncio.queues import Queue


# ------------------------------------------------------------------------ #
class MQTTService(object):
    
    def __init__(self, mqtt_config: dict) -> None:
        super(MQTTService, self).__init__()

        '''
        General flag indicating if tasks is still running 
        and MQTT is working

        NOTE this flag is controlled by start() and stop() functions
        '''
        self.is_running = False

        '''
        Callback functions for external activity and data parsing
        '''
        self.subscribe_callback = None
        self.parse_callback = None

        self.__mqtt_config = mqtt_config
        self.__client = mqtt_client.Client(mqtt_client.CallbackAPIVersion.VERSION2,
                                           client_id=self.__mqtt_config['client_id'])

        self.__client.on_connect = self.__on_connect
        self.__client.on_disconnect = self.__on_disconnect
        self.__client.on_message = self.__on_message

        '''
        Store outgoing messages to parse them in dedicated thread/task
        '''
        self.__pub_queue = Queue(maxsize=5)
        '''
        Store incoming messages to parse them in dedicated thread/task
        '''
        self.__new_msg_queue = Queue(maxsize=50)


    def __pub_report(self) -> None:
        try:
            if publish_data := self.__pub_queue.get_nowait():
                self.__pub_queue.task_done()

                for key, value in publish_data.items():
                    self.__client.publish(key, value)
        except Exception as ex:
            pass
            # if ex is not QueueEmpty:
            #     logging.error(f'{ex}')


    def __on_connect(self, client: mqtt_client, userdata, flags, reason, properties):
        if reason == 0:
            logging.info('Connected to MQTT Broker!')
            
        else:
            logging.info(f'Failed to connect, return code {reason}')
            
        if reason == 0 and callable(self.subscribe_callback):
            self.subscribe_callback(client)


    def __on_disconnect(self, client, userdata, flags, reason, properties):
        logging.info('Disconnected from MQTT Broker!')


    def __on_message(self, client: mqtt_client, userdata, msg):
        try:
            self.__new_msg_queue.put_nowait(msg)
        except Exception as ex:
            logging.warning(f'MQTT Rx queue doesn\'t keep up! {ex}')


    async def __parse_in(self) -> None:
        loop_time: float = 0.00001

        while self.is_running:
            try:
                if msg := self.__new_msg_queue.get_nowait():
                    self.__new_msg_queue.task_done()

                    topic_name: str = msg.topic.split('/')[-1]
                    topic_data: str = msg.payload.decode()

                    if callable(self.parse_callback):
                        self.parse_callback(topic_name, topic_data)
            except Exception as ex:
                pass
            
            await asyncio.sleep(loop_time)


    async def __run(self):
        """
        Main service Coroutine task which is:
          - Process MQTT loop
          - Checks incoming queue for new topics to be sent
        """

        loop_time: float = 0.0001

        while self.is_running:
            self.__pub_report()

            rc = self.__client.loop(timeout=loop_time)
            if rc != 0:
                # TODO add reconnection handle
                pass

            await asyncio.sleep(loop_time)

    # ------------------------------------------------------------------------ #
    async def send_data(self, new_data: dict) -> None:
        """
        Post all data to the MQTT broker under [ topic, data ]

        NOTE:
          MQTT service must be launched with .start() !
        """
        try:
            await self.__pub_queue.put(new_data)
        except Exception as ex:
            logging.warning(f'MQTT Tx queue doesn\'t keep up! {ex}')
  
  
    def send_data_nowait(self, new_data: dict) -> None:
        """
        Post all data to the MQTT broker under [ topic, data ]

        NOTE:
          MQTT service must be launched with .start() !
        """
        try:
            self.__pub_queue.put_nowait(new_data)
        except Exception as ex:
            logging.warning(f'MQTT Tx queue doesn\'t keep up! {ex}')


    def connect(self) -> mqtt_client:
        try:
            self.__client.connect(self.__mqtt_config["broker_ip"],
                                  self.__mqtt_config["broker_port"])

            self.is_running = True

        except Exception as ex:
            logging.error(f'{ex}')
            self.is_running = False

        return self.__client


    def disconnect(self) -> None:
        self.__client.disconnect()
        logging.info('Force disconnect from MQTT Broker!')


    def stop(self) -> None:
        '''
        Stop all MQTT related stack.
        Has to be done only once before closing App.
        
        NOTE may also do disconnection if there still any.
        '''
        self.is_running = False
        self.disconnect()


    def start(self) -> None:
        '''
        Launch all requred internal stuff to MQTT client service
        Has to be done only once when main App is in initialization
        '''
        self.is_running = True

        asyncio.create_task(self.__run())
        asyncio.create_task(self.__parse_in())
