

#include "msp430g2553.h"
#include "i2c.h"

#define ADDR_SLAVE              0x45   	// TCA6507 Led Driver Address

#define REG_VALUE_CTRL_AUTO_INC 0x10    // Auto increment
#define REG_VALUE_CTRL          0x00    // Not auto increment
#define REG_VALUE_FADEON        0x44
#define REG_VALUE_FULLON        0x44
#define REG_VALUE_FADEOFF       0x44
#define REG_VALUE_FIRSTOFF      0x44
#define REG_VALUE_SECONDOFF     0x44
#define REG_VALUE_MAXINTENS     0xFF    // 0bAAAABBBB; Maximum intensity, B for PWM0, A for PWM1,
#define REG_VALUE_MASTERINTENS  0x30    // 0bBBAAXXXX; X values represent ALD value,
                                        // AA 1: use ALD value instead of MAXINTENS for PWMs.
                                        // BB 0: use normal mode for PWMs, 1: one-shot mode
#define REG_VALUE_INITIAL       0x00

#define LED_STATE_OFF           0x00
#define LED_STATE_ON_PWM0       0x02
#define LED_STATE_ON_PWM1       0x03
#define LED_STATE_ON_FULL       0x04
#define LED_STATE_ON_ONESHOT    0x05
#define LED_STATE_BLINK_PWM0    0x06
#define LED_STATE_BLINK_PWM1    0x07

#define REG_ADDR_MAXIMUM	   	0x08
#define REG_ADDR_MASTERINTENS   0x09

#define MAX_VALUE_PWM 15

unsigned char countPWM0 = 0;
unsigned char countPWM1 = 0;
unsigned char bytePWMs = 0xFF;
char portValues[7];

void initLedDriver(char portValues[]){
    unsigned char i = 0, j = 0;
    unsigned char cmd[4] = {0x00, 0x00, 0x00, 0x00};
    cmd[0] = REG_VALUE_CTRL_AUTO_INC;
    
    for(i = 1; i < 4; i+=1){
        for(j = 0; j < 7; j+=1){
            cmd[i] |= ( ( (portValues[j] >> (i-1)) & 0x01 ) << j );
        }
    }
  	I2C_Write(cmd, 4, 0);
}
void transmitCommand(unsigned char regAddr, unsigned char value){
    unsigned char cmd[2] = {0x00, 0x00};

    if(regAddr <= 0x0A){
        cmd[0] = REG_VALUE_CTRL | regAddr;
        cmd[1] = value;
    	I2C_Write(cmd, 2, 0);
    }
}

unsigned char incrementPWM0(){
	countPWM0 += 1;
	if(countPWM0 > MAX_VALUE_PWM)
		countPWM0 = 0;
	
	bytePWMs &= 0xF0;
	bytePWMs |= countPWM0;	
	transmitCommand(REG_ADDR_MAXIMUM, bytePWMs );	
	
	while ( I2C_isChannelBusy() );         // wait for bus to be free
	initLedDriver(portValues);
  
    return countPWM0;	
}
unsigned char incrementPWM1(){
	countPWM1 += 1;
	if(countPWM1 > MAX_VALUE_PWM)
		countPWM1 = 0;
	
	bytePWMs &= 0x0F;
	bytePWMs |= countPWM1 << 4;	
	
	transmitCommand(REG_ADDR_MAXIMUM, bytePWMs);	
	
	while ( I2C_isChannelBusy() );         // wait for bus to be free
	initLedDriver(portValues);
  		
    return countPWM1;	
}


void testPorts(){
    unsigned char i;
    for(i = 0; i < 7; i +=1){
        portValues[i] = LED_STATE_OFF;
    }
  
    I2C_init(ADDR_SLAVE);
    while ( I2C_isChannelBusy() );         // wait for bus to be free
  	if ( I2C_isExist(ADDR_SLAVE) )    // slave address may differ from
  	{
		transmitCommand(REG_ADDR_MASTERINTENS, 0x3F);
		
		for(i = 0; i < 7; i +=1){
	        portValues[i] = LED_STATE_ON_PWM0;
	 
	 		while ( I2C_isChannelBusy() );         
	        initLedDriver(portValues);

			//insecure for loop delay
			//delay_cycle function creates problems with my interrupts
			//and putting these loop into a function does not work, probably because of compiler optimizations
			int k, j, x=0;
			for(k = 0; k < 1000; k +=1){
				for(j = 0; j < 100; j +=1){
					x += j;
			    }
			}

	        portValues[i] = LED_STATE_OFF;
   		}
 
   		initLedDriver(portValues);
  	}
}


void countTest(){ 

    unsigned char i;
    for(i = 0; i < 7; i +=1){
        portValues[i] = LED_STATE_ON_PWM0;
    }
    
    
    I2C_init(ADDR_SLAVE);
    while ( I2C_isChannelBusy() );         // wait for bus to be free
  	if ( I2C_isExist(ADDR_SLAVE) )    // slave address may differ from
  	{ 
		
  
  		while(1){
			while ( I2C_isChannelBusy() );         // wait for bus to be free
			char count = incrementPWM0();

			int k, j, x=0;
			for(k = 0; k < 1000; k +=1){
				for(j = 0; j < 100; j +=1){
					x += j;
			    }
			}
  		}

  	}
    
}

void basicTest(void){
    unsigned char i;
    for(i = 0; i < 7; i++){
        portValues[i] = LED_STATE_ON_FULL;
    }
    
//    for(i = 0; i < 3; i++){
//        portValues[i] = LED_STATE_OFF;
//    }

    I2C_init(ADDR_SLAVE);
    while ( I2C_isChannelBusy() );         // wait for bus to be free
  	if ( I2C_isExist(ADDR_SLAVE) )    // slave address may differ from
  	{ 
    	initLedDriver(portValues);
  	}
}

void main(void){
    WDTCTL = WDTPW + WDTHOLD;                   // Stop watchdog timer
    _enable_interrupts();
    
 
	//basicTest();
    
testPorts();
	//countTest();

	while(1);
	
	
}
