#include "mpu6050_i2c.h"

void i2c_read_register(const struct device *i2c_dev, uint8_t reg_adrr)
{
	if (!device_is_ready(i2c_dev)) {
		printk("I2C device not ready\n");
		return;
	} else {
		printk("I2C device is ready\n");
	}

	uint8_t read_data = 0x00; // Buffer to read data into

	// Use i2c_burst_read for more detailed debugging
	int ret = i2c_reg_read_byte(i2c_dev, I2C_DEV_ADDR, reg_adrr, &read_data);
	if (ret) {
		printk("I2C read failed: %d\n", ret);
		if (ret == -EIO) {
			printk("I2C communication error\n");
		} else if (ret == -ENODEV) {
			printk("I2C device not found\n");
		} else if (ret == -EINVAL) {
			printk("Invalid parameters\n");
		} else if (ret == -EFAULT) {
			printk("Bad address\n");
		} else {
			printk("Unknown error\n");
		}
	} else {
		printk("I2C read succeeded\n");
		printk("Read data from register 0x%02X: 0x%02X\n", reg_adrr, read_data);
	}
}

void i2c_write_register(const struct device *i2c_dev, uint8_t reg_adrr, uint8_t data)
{

	int ret = i2c_reg_write_byte(i2c_dev, I2C_DEV_ADDR, reg_adrr, data);
	if (ret) {
		printk("failed write");
		
		return;
	} else {
		printk("success write");
	}
}





int16_t read_accel_data(const struct device *i2c_dev, uint8_t reg_addr) {
    uint8_t high_byte, low_byte;
    int16_t accel_data;

    // Read high byte
    if (i2c_reg_read_byte(i2c_dev, I2C_DEV_ADDR, reg_addr, &high_byte)) {
        printk("Failed to read high byte from register 0x%02X\n", reg_addr);
        return 0;
    }

    // Read low byte
    if (i2c_reg_read_byte(i2c_dev, I2C_DEV_ADDR, reg_addr + 1, &low_byte)) {
        printk("Failed to read low byte from register 0x%02X\n", reg_addr + 1);
        return 0;
    }

    // Combine high and low bytes
    accel_data = (int16_t)((high_byte << 8) | low_byte);

    return accel_data;
}

void basic_configuration(const struct device *i2c_dev ){

    i2c_read_register(i2c_dev, WHO_AM_I);

    uint8_t pwr_mgmt_1_value = 0x01;  // 00000001: Wake up and select PLL with X-axis gyro reference
    i2c_write_register(i2c_dev, PWR_MGMT_1 , pwr_mgmt_1_value);


    uint8_t accel_config_value = 0x00;  // 00000000: No self-test, ±2g
    i2c_write_register(i2c_dev, ACCEL_CONFIG , accel_config_value);
}