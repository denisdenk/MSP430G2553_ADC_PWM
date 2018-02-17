#include <msp430.h>

typedef unsigned char     uint8_t;
typedef unsigned short    uint16_t;
typedef unsigned int      uint32_t;
typedef unsigned long int uint64_t;

long ADC_value = 0;
long PWM = 0;

/* Прототипы функций */
void ConfigureADC(void);
void ConfigurePWM(void);
long map(long x, long in_min, long in_max, long out_min, long out_max);

void main(void)
{
	{
		WDTCTL = WDTPW + WDTHOLD;		// Stop WDT

		BCSCTL1 = CALBC1_1MHZ;			// Set range   DCOCTL = CALDCO_1MHZ;
		BCSCTL2 &= ~(DIVS_3);			// SMCLK = DCO = 1MHz

		P1SEL |= BIT3;				// Вход АЦП P1.3
		P1DIR |= BIT2;				// P1.2 на выход
		P1SEL |= BIT2;            		// P1.2 to TA0.1

		ConfigureADC();				// Инициализация АЦП
		ConfigurePWM();				// инициализация PWM

		__enable_interrupt();			// Активация прерываний

		while(1)
		{
			__delay_cycles(1000);				// Wait for ADC Ref to settle
			ADC10CTL0 |= ENC + ADC10SC;
			__bis_SR_register(CPUOFF + GIE);

			ADC_value = ADC10MEM;
			PWM = map(ADC_value, 0, 1024, 0, 1000);
			CCR1 = PWM;                			// CCR1 PWM duty cycle (0 -1000)
		}
	}
}

// ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR (void)
{
	__bic_SR_register_on_exit(CPUOFF);        // Return to active mode 
}

/* Инициализация АЦП */
void ConfigureADC(void)
{
	ADC10CTL1 = INCH_3 + ADC10DIV_3 ;         		// Channel 3, ADC10CLK/3
	ADC10CTL0 = SREF_0 + ADC10SHT_3 + ADC10ON + ADC10IE;  	// Vcc & Vss as reference, Sample and hold for 64 Clock cycles, ADC on, ADC interrupt enable
	ADC10AE0 |= BIT3;                         		// ADC input enable P1.3
}
/* Инициализация ШИМ */
void ConfigurePWM(void)
{
	CCR0 = 1000-1;             // PWM Period
	CCTL1 = OUTMOD_7;          // CCR1 reset/set
	TACTL = TASSEL_2 + MC_1;   // SMCLK, up mode
}

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


