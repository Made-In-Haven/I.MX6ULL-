#ifndef __BPS_AP3216C_H
#define __BPS_AP3216C_H
#include "imx6ul.h"




#define AP3216C_ADDR 0X1E
#define AP3216C_SYSTEM_CONFIG 0X00
#define AP3216C_INT_STATUS 0X01
#define AP3216C_IR_DATA_LOW 0X0A
#define AP3216C_IR_DATA_HIGH 0X0B
#define AP3216C_ALS_DATA_LOW 0X0C
#define AP3216C_ALS_DATA_HIGH 0X0D
#define AP3216C_PS_DATA_LOW 0X0E
#define AP3216C_PS_DATA_HIGH 0X0F

void ap3216c_init(I2C_Type * base);
unsigned char ap3216c_read_one_byte(I2C_Type *base, unsigned char * data, 
                                    unsigned char dev_addr, unsigned char reg_addr);
unsigned char ap3216c_write_one_byte(I2C_Type *base, unsigned char * data, 
                                    unsigned char dev_addr, unsigned char reg_addr);
void ap3216c_read_data(I2C_Type * base,unsigned short * ir, 
                    unsigned short * als, unsigned short *ps);
void ap3216c_read_als(I2C_Type * base,unsigned short * als);




#endif