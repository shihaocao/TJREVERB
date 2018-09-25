import smbus
import time
from smbus2 import SMBusWrapper

address = 43
bus = smbus.SMBus(1)

SW0 = 2
Sw1 = 3
SW2 = 4
SW3 = 5
SW4 = 6
SW5 = 7
SW6 = 8
SW7 = 9
SW8 = 10
SW9 = 11

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
    with SMBusWrapper(1) as bus:
        PDM_val = [PDM_val]
        bus.write_i2c_block_data(address, 0x12, PDM_val)

def pin_off(PDM_val):
    with SMBusWrapper(1) as bus:
        PDM_val = [PDM_val]
        bus.write_i2c_block_data(address, 0x13, PDM_val)

def get_board_status():
    with SMBusWrapper(1) as bus:
        return bus.read_i2c_block_data(address, 0x01)

def set_system_watchdog_timeout(timeout):
    with SMBusWrapper(1) as bus:
        timeout = [timeout]
        bus.write_i2c_block_data(address, 0x06, timeout)

def get_BCR1_volts():
    with SMBusWrapper(1) as bus:
        bus.write_i2c_block_data(address, 0x10, 0x00)
        return bus.read_byte(address)

def get_BCR1_amps_A():
    with SMBusWrapper(1) as bus:
        bus.write_i2c_block_data(address, 0x10, 0x01)
        return bus.read_byte(address)

def get_BCR1_amps_B():
    with SMBusWrapper(1) as bus:
        bus.write_i2c_block_data(address, 0x10, 0x02)
        return bus.read_byte(address)

# Flashes LED at D3 (Pin 3) in 1 second increments
while True:
    pin_on(SW0)
    time.sleep(1)
    pin_off(SW0)
    time.sleep(1)
