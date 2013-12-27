#ifndef I2C_H_
#define I2C_H_

void I2C_init(unsigned char slaveAddress);
void I2C_Write (unsigned char*, unsigned char, unsigned char);
unsigned char I2C_isExist(unsigned char slave_address);
unsigned char I2C_isChannelBusy();

#endif /*I2C_H_*/
