//
//  msp430-tca6507-noninterrupt.c
//  

#include "msp430g2553.h"
#define ADDR_SLAVE              0x45    // TCA6507 Led Driver Address
#define REG_VALUE_CTRL_AUTO_INC 0x10    // Auto increment
#define REG_VALUE_CTRL          0x00    // Not auto increment
#define REG_VALUE_FADEON        0x44
#define REG_VALUE_FULLON        0x44
#define REG_VALUE_FADEOFF       0x44
#define REG_VALUE_FIRSTOFF      0x44
#define REG_VALUE_SECONDOFF     0x44
#define REG_VALUE_MAXINTENS     0xFF    // 0bAAAABBBB; Maximum intensity, B for PWM0, A for PWM1,
#define REG_VALUE_MASTERINTENS  0x38    // 0bBBAAXXXX; X values represent ALD value,
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

#define REG_ADDR_MASTERINTENS   0x09


void initI2C(unsigned char slaveAddress){
    P1SEL |= BIT6 + BIT7;                       // Assign I2C pins to USCI_B0
    P1SEL2|= BIT6 + BIT7;
    
    //    P1OUT |= BIT6 + BIT7;                 // We can enable internal pull-ups
    //    P1REN |= BIT6 + BIT7;
    
    UCB0CTL1 |= UCSWRST;                        // Enable SW reset, to configure USCI
    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;       // I2C Master, synchronous mode
    UCB0CTL1 = UCSSEL_2 + UCSWRST;              // Use SMCLK, keep SW reset
    UCB0BR0 = 12;                               // fSCL = SMCLK/10 = >100kHz
    UCB0BR1 = 0;
    UCB0I2CSA = slaveAddress;                  // Set slave adress
    UCB0CTL1 &= ~UCSWRST;                       // Clear SW reset
    UCB0I2CIE = UCNACKIE;
    
    //IE2 = UCB0TXIE + UCB0RXIE ;
    IE2 = UCB0RXIE ;
}

void transmitCommand(unsigned char regAddr, unsigned char value){
    unsigned char cmd[2] = {0x00, 0x00};
    if(regAddr <= 10){
        cmd[0] = REG_VALUE_CTRL | regAddr;
        cmd[1] = value;
    }
    
    UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
    
    for (i = 0; i < 2; i+=1) {
        UCB0TXBUF = cmd[i];              // Loading Tx buffer with Data
        while(!(IFG2 & UCB0TXIFG));
    }
    
    UCB0CTL1 |= UCTXSTP;       //Always send/generate STOP condifiton before last
    IFG2 &= ~UCB0TXIFG;        //Clear USCI_B0 TX int flag
    while(UCB0CTL1 & UCTXSTP); //Wait till stop condition is sent
}

void initLedDriver(char *portValues){
    unsigned char i = 0, j = 0;
    char cmd[4] = {0x00, 0x00, 0x00, 0x00};
    cmd[0] = REG_VALUE_CTRL_AUTO_INC;
    
    for(i = 1; i < 4; i+=1){
        for(j = 0; j < 7; j+=1){
            cmd[i] |= ( ( (portValues[j] >> (i-1)) & 0x01 ) << j );
        }
    }
    
    UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
    
    for (i = 0; i < 4; i+=1) {
        UCB0TXBUF = cmd[i];              // Loading Tx buffer with Data
        while(!(IFG2 & UCB0TXIFG));
    }
    
    UCB0CTL1 |= UCTXSTP;       //Always send/generate STOP condifiton before last
    IFG2 &= ~UCB0TXIFG;        // Clear USCI_B0 TX int flag
    while(UCB0CTL1 & UCTXSTP); //Wait till stop condition is sent
}

void testPorts(){
    char portValues[7];
    unsigned char i;
    for(i = 0; i < 7; i++){
        portValues[i] = LED_STATE_OFF;
    }
    WDTCTL = WDTPW + WDTHOLD;                   // Stop watchdog timer
    
    initI2C(ADDR_SLAVE);
    transmitCommand(REG_ADDR_MASTERINTENS, 0x3F);
    initLedDriver(portValues);
    
    for(int i = 0; i < 7; i++){
        portValues[i] = LED_STATE_ON_PWM0;
        initPorts(portValues);
        //delay 1 s
        portValues[i] = LED_STATE_OFF;
    }
    initPorts(portValues);
}
void basicTest(){
    WDTCTL = WDTPW + WDTHOLD;                   // Stop watchdog timer
    
    char portValues[7];
    unsigned char i;
    for(i = 0; i < 7; i++){
        portValues[i] = LED_STATE_ON_FULL;
    }
    initI2C(ADDR_SLAVE);
    initLedDriver(portValues);
}
void main(void){
    basicTest():
    //testPorts();
}

//Note : STOP condition in Master receive mode is always preceeded by NACK
#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCIAB0RX_ISR(void)
{
     // Not-acknowledge interrupt. This flag is set when an acknowledge is expected
     // but is not received. UCNACKIFG is automatically cleared when a START condition
     // is received.
     if( UCB0STAT & UCNACKIFG )
     {
         UCB0CTL1 |= UCTXSTP;            // I2C stop condition
         UCB0STAT &= ~UCNACKIFG;            // Clear interrupt flag
     }
}

