import smbus

#Need to define hex code as variables here?
#Ex: GET_BOARD_STATUS = 0x01?
address = 43
b = smbus.SMBus(0)
#Where is smbus imported from?

def turn_PDM_on(PDM_val):
    b.write_i2c_block_data(address, 0x12, PDM_val)

def turn_PDM_off(PDM_val):
    b.write_i2c_block_data(address, 0x13, PDM_val)

def get_board_status():
    #I have no idea how this works
    #0x00, represents voltage reading to the BCR(1), returns 4 bytes

def get_version_number():
    return b.read_i2c_block_data(address, 0x04, 0x00)

def set_system_watchdog_timeout(timeout):
    watchdog = timeout

def set_PCM_reset(int data):
    b.write_i2c_block_data(address, 0x02, data)
