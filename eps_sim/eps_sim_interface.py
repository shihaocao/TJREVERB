import smbus

address = 43
b = smbus.SMBus(1)

def turn_PDM_on(PDM_val):
    b.write_i2c_block_data(address, 0x12, PDM_val)

def turn_PDM_off(PDM_val):
    b.write_i2c_block_data(address, 0x13, PDM_val)

def get_board_status():
    return b.read_i2c_block_data(address, 0x01)

def set_system_watchdog_timeout(timeout):
    b.write_i2c_block_data(address, 0x06, timeout)

def get_BCR1_volts():
    b.write_i2c_block_data(address, 0x10, 0x00)
    return b.read_byte(address)

def get_BCR1_amps_A():
    b.write_i2c_block_data(address, 0x10, 0x01)
    return b.read_byte(address)

def get_BCR1_amps_B():
    b.write_i2c_block_data(address, 0x10, 0x02)
    return b.read_byte(address)
while true:
    turn_PDM_on(SW0)
    time.sleep(2)
    turn_PDM_off(SW0)
    time.sleep(2)
