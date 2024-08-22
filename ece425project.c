#include "TM4C123.h"
#include <stdint.h>
#include <stdlib.h>

#define BUFFER_SIZE 64

/*-----------USER FUNCTION DECLARATIONS--------------*/
void Delay(unsigned long counter); // used to add delay
void UART5_Init(void); // Initialize UART5 module
char UART5_Read(void); // Read data from Rx5 pin of TM4C123
void UART5_Write(unsigned char data); // Transmit a character over Tx5 pin 
void UART5_Write_String(char *str); // Transmit a string over Tx5 pin 
void delayUs(int ttime); // Delay in microseconds
void playNote(int note, int duration); // Play note

// Note Frequencies for Do Re Mi Fa So La Ti Do
#define C4  261
#define D4  294
#define E4  329
#define F4  349
#define G4  392
#define A4  440
#define B4  494
#define C5  523

int main(void) {
    UART5_Init(); // Call UART5_Init() to initialize UART5 of TM4C123GH6PM

    /* Set PF1, PF2 and PF3 as digital output pins */
    SYSCTL->RCGCGPIO |= 0x20;   /* Enable clock to GPIOF */
    GPIOF->DIR |= 0x0E;         // Set PF1, PF2 and PF3 as digital output pins
    GPIOF->DEN |= 0x0E;         // Enable PF1, PF2 and PF3 as digital GPIO pins
    Delay(10); 
    
    /* Set PC4 as output pin for buzzer */
    SYSCTL->RCGCGPIO |= 0x04;   /* Enable clock to GPIOC */
    GPIOC->AMSEL &= ~0x10;      /* Disable analog function */
    GPIOC->DIR |= 0x10;         /* Set PC4 as output pin */
    GPIOC->DEN |= 0x10;         /* Set PC4 as digital pin */
    
    /* Initialize TIMER0 for delays */
    SYSCTL->RCGCTIMER |= 1;     /* Enable clock to TIMER0 */
    TIMER0->CTL = 0;            /* Disable TIMER0 during setup */
    TIMER0->CFG = 0x04;         /* 16-bit timer mode */
    TIMER0->TAMR = 0x02;        /* Periodic mode */
    TIMER0->TAILR = 16 - 1;     /* Interval load value for 1us ticks */
    TIMER0->ICR = 0x1;          /* Clear the TimerA timeout flag */
    TIMER0->CTL |= 0x01;        /* Enable TIMER0A */
    
    UART5_Write_String("UART5 Initialized. Type something:\r\n");

    char buffer[BUFFER_SIZE];
    int bufferIndex = 0;

    while (1) {
        /* Read a character from UART5 */
        if ((UART5->FR & (1 << 4)) == 0) {  /* If the RX buffer is not empty */
            buffer[bufferIndex] = UART5_Read();
            bufferIndex = (bufferIndex + 1) % BUFFER_SIZE;
        }

        /* Process characters in the buffer */
        if (bufferIndex > 0) {
            for (int i = 0; i < bufferIndex; i++) {
                char c = buffer[i];
                
                /* Play note based on received character */
                switch(c) {
                    case 'C':
                        playNote(C4, 500); // 500ms for better duration
                        UART5_Write_String("Playing C4 (Do)\n");
                        break;
                    case 'D':
                        playNote(D4, 500);
                        UART5_Write_String("Playing D4 (Re)\n");
                        break;
                    case 'E':
                        playNote(E4, 500);
                        UART5_Write_String("Playing E4 (Mi)\n");
                        break;
                    case 'F':
                        playNote(F4, 500);
                        UART5_Write_String("Playing F4 (Fa)\n");
                        break;
                    case 'G':
                        playNote(G4, 500);
                        UART5_Write_String("Playing G4 (So)\n");
                        break;
                    case 'A':
                        playNote(A4, 500);
                        UART5_Write_String("Playing A4 (La)\n");
                        break;
                    case 'B':
                        playNote(B4, 500);
                        UART5_Write_String("Playing B4 (Ti)\n");
                        break;
                    case 'c':
                        playNote(C5, 500);
                        UART5_Write_String("Playing C5 (Do)\n");
                        break;
                    default:
                        UART5_Write_String("Unknown command\n");
                        break;
                }
            }
            bufferIndex = 0;  // Reset buffer index after processing
        }
    }
}

void UART5_Init(void) {
    SYSCTL->RCGCUART |= 0x20;  /* Enable clock to UART5 */
    SYSCTL->RCGCGPIO |= 0x10;  /* Enable clock to PORTE for PE4/Rx and PE5/Tx */
    Delay(1);

    /* UART5 initialization */
    UART5->CTL = 0;            /* Disable UART5 module */
    UART5->IBRD = 325;         /* For 9600 baud rate, integer = 325 */
    UART5->FBRD = 34;          /* For 9600 baud rate, fractional = 34 */
    UART5->CC = 0;             /* Select system clock */
    UART5->LCRH = 0x60;        /* Data length 8-bit, no parity bit, no FIFO */
    UART5->CTL = 0x301;        /* Enable UART5 module, Rx and Tx */

    /* UART5 TX5 and RX5 use PE4 and PE5. Configure them as digital and enable alternate function */
    GPIOE->DEN |= 0x30;        /* Set PE4 and PE5 as digital */
    GPIOE->AFSEL |= 0x30;      /* Use PE4, PE5 alternate function */
    GPIOE->AMSEL = 0;          /* Turn off analog function */
    GPIOE->PCTL = (GPIOE->PCTL & 0xFF00FFFF) + 0x00110000; /* Configure PE4 and PE5 for UART */
}

char UART5_Read(void) {
    char data;
    while ((UART5->FR & (1 << 4)) != 0); /* Wait until Rx buffer is not full */
    data = UART5->DR;                     /* Read received data */
    return data;
}

void UART5_Write(unsigned char data) {
    while ((UART5->FR & (1 << 5)) != 0); /* Wait until Tx buffer is not full */
    UART5->DR = data;                    /* Transmit data */
}

void UART5_Write_String(char *str) {
    while (*str) {
        UART5_Write(*(str++));
    }
}

void Delay(unsigned long counter) {
    unsigned long i = 0;
    for (i = 0; i < counter; i++);
}

void delayUs(int ttime) {
    int i;
    for (i = 0; i < ttime; i++) {
        while ((TIMER0->RIS & 0x1) == 0); // wait for TimerA timeout flag
        TIMER0->ICR = 0x1; // clear the TimerA timeout flag
    }
}

void playNote(int note, int duration) {
    int halfPeriod = 1000000 / (2 * note); 
    int cycles = (duration * 1000) / (2 * halfPeriod);

    for (int i = 0; i < cycles; i++) { 
        GPIOC->DATA |= 0x10; // PC4: HIGH
        delayUs(halfPeriod);
        GPIOC->DATA &= ~0x10; // PC4: LOW
        delayUs(halfPeriod);
    }
}