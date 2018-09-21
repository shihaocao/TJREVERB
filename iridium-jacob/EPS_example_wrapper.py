import smbus

address = 43

b = smbus.SMBus(0)

def turn_PDM_on(PDM_val):
    b.write_12c_block_data(address, 0x12, PDM_val)

def turn_PDM_off(PDM_val):
    b.write_i2c_block_data(address, 0x13, PDM_val)
