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
void playNoteContinuous(int note); // Play note continuously
void stopNote(); // Stop playing note
void TIMER1A_Handler(void); // Timer1A ISR

// Note Frequencies for C4 to C5
#define C4  261
#define Db4 277
#define D4  294
#define Eb4 311
#define E4  329
#define F4  349
#define Gb4 370
#define G4  392
#define Ab4 415
#define A4  440
#define Bb4 466
#define B4  494
#define C5  523
#define Db5 554
#define D5  587
#define Eb5 622
#define E5  659

volatile int currentNote = 0; // To track the current note being played

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
   
    /* Initialize TIMER1 for buzzer */
    SYSCTL->RCGCTIMER |= 2;     /* Enable clock to TIMER1 */
    TIMER1->CTL = 0;            /* Disable TIMER1 during setup */
    TIMER1->CFG = 0x04;         /* 16-bit timer mode */
    TIMER1->TAMR = 0x02;        /* Periodic mode */
    TIMER1->TAILR = 16000000 / C4 - 1;  /* Initial interval load value for C4 */
    TIMER1->ICR = 0x1;          /* Clear the TimerA timeout flag */
    TIMER1->IMR |= 0x01;        /* Enable TimerA timeout interrupt */
    TIMER1->CTL |= 0x01;        /* Enable TIMER1A */

    NVIC->ISER[0] = 1 << 21;    /* Enable IRQ21 for TIMER1A */

    UART5_Write_String("UART5 Initialized. Type something:\r\n");

    while (1) {
        /* Read a character from UART5 */
        if ((UART5->FR & (1 << 4)) == 0) {  /* If the RX buffer is not empty */
            char c = UART5_Read();
           
            if (c == 'x') {
                stopNote();
                UART5_Write_String("Stopping Note\n");
            } else {
                switch(c) {
                    case 'C':
                        playNoteContinuous(C4);
                        UART5_Write_String("Playing C4 (C)\n");
                        break;
                    case 'd':
                        playNoteContinuous(Db4);
                        UART5_Write_String("Playing Db4 (Db)\n");
                        break;
                    case 'D':
                        playNoteContinuous(D4);
                        UART5_Write_String("Playing D4 (D)\n");
                        break;
                    case 'e':
                        playNoteContinuous(Eb4);
                        UART5_Write_String("Playing Eb4 (Eb)\n");
                        break;
                    case 'E':
                        playNoteContinuous(E4);
                        UART5_Write_String("Playing E4 (E)\n");
                        break;
                    case 'F':
                        playNoteContinuous(F4);
                        UART5_Write_String("Playing F4 (F)\n");
                        break;
                    case 'g':
                        playNoteContinuous(Gb4);
                        UART5_Write_String("Playing Gb4 (Gb)\n");
                        break;
                    case 'G':
                        playNoteContinuous(G4);
                        UART5_Write_String("Playing G4 (G)\n");
                        break;
                    case 'a':
                        playNoteContinuous(Ab4);
                        UART5_Write_String("Playing Ab4 (Ab)\n");
                        break;
                    case 'A':
                        playNoteContinuous(A4);
                        UART5_Write_String("Playing A4 (A)\n");
                        break;
                    case 'b':
                        playNoteContinuous(Bb4);
                        UART5_Write_String("Playing Bb4 (Bb)\n");
                        break;
                    case 'B':
                        playNoteContinuous(B4);
                        UART5_Write_String("Playing B4 (B)\n");
                        break;
                    case 'c':
                        playNoteContinuous(C5);
                        UART5_Write_String("Playing C5 (C)\n");
                        break;
case '1':
playNoteContinuous(Db5);
                        UART5_Write_String("Playing Db5 (Db)\n");
break;
case '2':
playNoteContinuous(D5);
                        UART5_Write_String("Playing D5 (D)\n");
break;
case '3':
playNoteContinuous(Eb5);
UART5_Write_String("Playing Eb5 (Eb)\n");
break;
case '4':
 playNoteContinuous(E5);
                        UART5_Write_String("Playing E5 (E)\n");
break;
                    default:
                        UART5_Write_String("Unknown command\n");
                        break;
                }

                Delay(500000); // Delay for 500ms after each button press
            }
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

void playNoteContinuous(int note) {
    currentNote = note;
    int halfPeriod = 16000000 / (2 * note); // Calculate half period in timer ticks
    TIMER1->TAILR = halfPeriod - 1;        // Load the half period value into the timer
    TIMER1->CTL |= 0x01;                   // Enable TIMER1A
}

void stopNote() {
    currentNote = 0;
    GPIOC->DATA &= ~0x10; // Ensure the buzzer is off
    TIMER1->CTL &= ~0x01; // Disable TIMER1A
}

void TIMER1A_Handler(void) {
    TIMER1->ICR = 0x1; // Clear the TimerA timeout flag
    if (currentNote != 0) {
        GPIOC->DATA ^= 0x10; // Toggle PC4
    }
}
