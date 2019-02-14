#include <xc.h>         /* XC8 General Include File */
#include <stdint.h>        /* For uint8_t definition */
#include <stdbool.h>       /* For true/false definition */
#include <pic16f690.h>
#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "user.h"          /* User funct/params, such as InitApp */
#include "usart_pic16.h"
#define _XTAL_FREQ 8000000L

void moveRight();
void moveLeft();
void moveForward();
void moveBack();
void stopMove();
void servoRotate0();
void servoRotate90();
void servoRotate180();
int readPing();
void pensar();
char calculaMelhorDistancia();
int calcularDistanciaCentro();
int calcularDistanciaDireita();
int calcularDistanciaEsquerda();
void reposicionaServoSonar();
int leftDistance, rightDistance;
int distanciaObstaculo = 30;
int start = 0;

void main(void)
    {

    ConfigureOscillator();
    InitApp();
    USARTInit(2400);

    ANSELH = 0x00;
    ANSEL = 0x00;
    // TIMER2
    // periodo = (PR2 +1) x ciclo de maquina(1/(8MHz/4)) x prescaler Timer 2
    // periodo = (256) x 0,5us x 1 = 0,128 ms
    // frequencia = 1/periodo = 1/0,128 ms = 7,8k Hz

    // PR2+1 = TMR2 (qd há estouro do timer2, low para high)

    PR2 = 0xFF; // inicializa o registrador de controle do timer2 em 255
    T2CON = 0x04; // liga o timer2 e prescaler 1:1 

    TRISCbits.TRISC6 = 1; // input ultrasonic
    TRISBbits.TRISB6 = 0; // led
    TRISBbits.TRISB4 = 0; // servo
    TRISCbits.TRISC3 = 0; // out ultrasonic
    TRISCbits.TRISC0 = 0;
    TRISCbits.TRISC1 = 0;
    TRISCbits.TRISC2 = 0;
    TRISCbits.TRISC4 = 0;
    TRISCbits.TRISC5 = 0;
    TRISCbits.TRISC7 = 0;

    CCPR1L = 0b10001111;
    CM2CON0bits.C2ON = 0; //desabilita o c2out

    T1CON = 0x10; // inicializa o timer0

    stopMove();
    while (1)
        {

        if (start == 0)
            {
            uint8_t n = USARTDataAvailable();
            if (n != 0)
                {
                char data = USARTReadData();

                if (data == 'f')
                    {
                    PORTBbits.RB6 = 0;
                    moveForward();
                    }
                else if (data == 'b')
                    {
                    PORTBbits.RB6 = 1;
                    moveBack();
                    }
                else if (data == 'r')
                    {
                    moveRight();
                    }
                else if (data == 'l')
                    {
                    moveLeft();
                    }
                else if (data == 's')
                    {
                    stopMove();
                    }
                else if (data == 'i')
                    {
                    start = 1;
                    }
                else
                    {
                    stopMove();
                    }
                }
            }
        else
            {
            pensar();
            }

        }

    }

void moveRight()
    {
    PORTCbits.RC0 = 1;
    PORTCbits.RC1 = 0;
    CCP1CON = 0b01001100;
    }

void moveLeft()
    {
    PORTCbits.RC0 = 0;
    PORTCbits.RC1 = 1;
    CCP1CON = 0b01001100;
    }

void moveBack()
    {
    PORTCbits.RC0 = 1;
    PORTCbits.RC1 = 1;
    CCP1CON = 0b11001100; // habilita o modo PWM
    }

void moveForward()
    {
    PORTCbits.RC0 = 1;
    PORTCbits.RC1 = 1;
    CCP1CON = 0b01001100; // habilita o modo PWM
    }

void stopMove()
    {
    PORTCbits.RC0 = 0;
    PORTCbits.RC1 = 0;
    CCP1CONbits.CCP1M0 = 0;
    CCP1CONbits.CCP1M1 = 0;
    CCP1CONbits.CCP1M2 = 0;
    CCP1CONbits.CCP1M3 = 0;
    }

void servoRotate0() //0 Degree direita
    {
    unsigned int i;
    for (i = 0; i < 50; i++)
        {
        PORTBbits.RB4 = 1;
        __delay_us(530); //700
        PORTBbits.RB4 = 0;
        __delay_us(19200);
        }
    }

void servoRotate90() //90 Degree centro
    {
    unsigned int i;
    for (i = 0; i < 50; i++)
        {
        PORTBbits.RB4 = 1;
        __delay_us(1380);
        PORTBbits.RB4 = 0;
        __delay_us(18500);
        }
    }

void servoRotate180() //180 Degree esquerda
    {
    unsigned int i;
    for (i = 0; i < 50; i++)
        {
        PORTBbits.RB4 = 1;
        __delay_us(2280); //2100
        PORTBbits.RB4 = 0;
        __delay_us(17800);
        }
    }

int readPing()
    {
    int a;
    TMR1H = 0;
    TMR1L = 0; 

    PORTCbits.RC3 = 1; //TRIGGER HIGH
    __delay_us(10); //10uS Delay 
    PORTCbits.RC3 = 0; //TRIGGER LOW

    while (!PORTCbits.RC6); //Waiting for Echo
    TMR1ON = 1; //Timer Starts
    while (PORTCbits.RC6); //Waiting for Echo goes LOW
    TMR1ON = 0; //Timer Stops

    a = (TMR1L | (TMR1H << 8)); //Reads Timer Value
    a = a / 58.82; //Converts Time to Distance
    a = a + 1;
    return a;
    }

int calcularDistanciaCentro()
    {
    servoRotate90();
    __delay_ms(20);
    int leituraDoSonar = readPing();
    __delay_ms(500);
    leituraDoSonar = readPing();
    __delay_ms(500);
    return leituraDoSonar;
    }

int calcularDistanciaDireita()
    {
    servoRotate0();
    __delay_ms(200);
    int leituraDoSonar = readPing();
    __delay_ms(500);
    leituraDoSonar = readPing();
    __delay_ms(500);
    return leituraDoSonar;
    }

int calcularDistanciaEsquerda()
    {
    servoRotate180();
    __delay_ms(200);
    int leituraDoSonar = readPing();
    __delay_ms(500);
    leituraDoSonar = readPing();
    __delay_ms(500);
    return leituraDoSonar;
    }

char calculaMelhorDistancia()
    {
    int esquerda = calcularDistanciaEsquerda();
    int centro = calcularDistanciaCentro();
    int direita = calcularDistanciaDireita();
    reposicionaServoSonar();
    int maiorDistancia = 0;
    char melhorDistancia = '0';

    if (centro > direita && centro > esquerda)
        {
        melhorDistancia = 'c';
        maiorDistancia = centro;
        }
    else if (direita > centro && direita > esquerda)
        {
        melhorDistancia = 'd';
        maiorDistancia = direita;
        }
    else if (esquerda > centro && esquerda > direita)
        {
        melhorDistancia = 'e';
        maiorDistancia = esquerda;
        }
    if (maiorDistancia <= distanciaObstaculo)
        { //distância limite para parar o robô   
        moveBack();

        }
    reposicionaServoSonar();
    return melhorDistancia;

    }

void pensar()
    {
    servoRotate90();
    int distancia = readPing();
    if (distancia > distanciaObstaculo)
        {
        moveForward();
        __delay_ms(300);
        stopMove();

        }
    else
        {
        stopMove();
        char melhorDist = calculaMelhorDistancia();
        if (melhorDist == 'c')
            {
            moveForward();
            __delay_ms(300);
            stopMove();
            }
        else if (melhorDist == 'd')
            {
            moveBack();
            __delay_ms(150);
            moveRight();
            __delay_ms(650);
            stopMove();
            }
        else if (melhorDist == 'e')
            {
            moveBack();
            __delay_ms(150);
            moveLeft();
            __delay_ms(650);
            stopMove();
            }
        else
            {
            moveBack();
            __delay_ms(300);
            stopMove();
            }
        reposicionaServoSonar();
        }
    }

void reposicionaServoSonar()
    {
    servoRotate90();
    __delay_ms(200);
    } 
