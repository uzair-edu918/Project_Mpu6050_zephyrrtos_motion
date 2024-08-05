#ifndef MPU6050_I2C_H
#define MPU6050_I2C_H


#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>


#define I2C_DEV_ADDR 0x68  // 7-bit I2C address of the device
#define WHO_AM_I 0x75 // Device ID for test
#define PWR_MGMT_1 0x6B // register for enabling power , configuring clock , sleepmode etc.
#define ACCEL_CONFIG 0x1C


#define ACCEL_XOUT_H 0x3B
#define ACCEL_YOUT_H 0x3D
#define ACCEL_ZOUT_H 0x3F



void i2c_read_register(const struct device *i2c_dev , uint8_t reg_adrr);
void i2c_write_register(const struct device *i2c_dev , uint8_t reg_adrr , uint8_t data);
void basic_configuration(const struct device *i2c_dev );
int16_t read_accel_data(const struct device *i2c_dev, uint8_t reg_addr);

#endif // MY_I2C_H
