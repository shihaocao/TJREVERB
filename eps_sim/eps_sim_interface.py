import smbus
import time
from smbus2 import *

address = 43
b = smbus.SMBus(1)

SW0 = 1
Sw1 = 2
SW2 = 3
SW3 = 4
SW4 = 5
SW5 = 6
SW6 = 7
SW7 = 8
SW8 = 9
SW9 = 10

GET_BOARD_STATUS = b"0x01"
SET_PCM_RESET = b"0x02"
GET_VERSION_NUMBER = b"0x04"
SET_WATCHDOG_TIMEOUT = b"0x06"
RESET_SYSTEM_WATCHDOG = b"0x07"
GET_NUMBER_SYSTEM_RESETS = b"0x09"
SET_PDM_INITIAL_ON = b"0x0A"
SET_PDM_INITIAL_OFF = b"0x0B"
SET_ALL_PDM_ON = b"0x0C"
SET_ALL_PDM_OFF = b"0x0D"
GET_PDM_STATUS = b"0x0E"
GET_PDM_INITIAL_STATE = b"0x0F"
GET_BOARD_TELEM = b"0x10"
GET_WATCHDOG_TIMEOUT = b"0x11"
SET_PDM_ON = b"0x12"
SET_PDM_OFF = b"0x13"
GET_SOFT_RESET_NUM = b"0x14"
GET_EXPECTED_PDM_STATE = b"0x16"
GET_BOARD_TEMP = b"0x17"
RESET_NODE = b"0x80"


def pin_on(PDM_val):
    with SMBusWrapper(1) as b:
        PDM_val = [PDM_val]
        b.write_i2c_block_data(address, 0x12, PDM_val)

def pin_off(PDM_val):
    with SMBusWrapper(1) as b:
        PDM_val = [PDM_val]
        b.write_i2c_block_data(address, 0x13, PDM_val)

def get_board_status():
    with SMBusWrapper(1) as b:
        return b.read_i2c_block_data(address, 0x01)

def set_system_watchdog_timeout(timeout):
    with SMBusWrapper(1) as b:
        timeout = [timeout]
        b.write_i2c_block_data(address, 0x06, timeout)

def get_BCR1_volts():
    with SMBusWrapper(1) as b:
        b.write_i2c_block_data(address, 0x10, 0x00)
        return b.read_byte(address)

def get_BCR1_amps_A():
    with SMBusWrapper(1) as b:
        b.write_i2c_block_data(address, 0x10, 0x01)
        return b.read_byte(address)

def get_BCR1_amps_B():
    with SMBusWrapper(1) as b:
        b.write_i2c_block_data(address, 0x10, 0x02)
        return b.read_byte(address)

# Flashes LED at D3 (Pin 3) in 1 second increments
while True:
    pin_on(SW0)
    time.sleep(1)
    pin_off(SW0)
    time.sleep(1)
