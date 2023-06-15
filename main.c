

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

//PAME MPROSTA H ANAPODA
#define FORWARD  1
#define REVERSE -1
//sSTHN ARXH  PAME MPROSTA
volatile int direction = FORWARD;

//GIA KATHE KATASTASH ORIZOYME ENAN ARITHMO
#define MOVING 1
#define TURN_LEFT 2
#define TURN_RIGHT 3
#define TURN_180 4

//sSTHN ARXH EIMASTE SE KINHSH
int state = MOVING;	


int turns_left = 0;
int turns_right = 0;

#define TURN_LEFT_TIME  2//(2000000/1024)*0,5 = 9765
#define TURN_RIGTH_TIME 2//(2000000/1024)*0,5 = 9765
#define TURN_180_TIME TURN_LEFT*2

#define T1 2//20000000/1024 * 3 =  58593
#define T2 2//20000000/1024 * 2 =  39062
int timer; //METABLHTH POY MAS EGLOVIZEI STIS WHILE OSO O METRHTHS METRAEI
int adcInt;//AYTH H METABLHTH XREIAZETE AN DOSEI O  PLAINOS AISTHTHRAS PIDAEI THN ADC TOY MPROSTA
int switchInt;//AYTH H METABLHTH XREIAZETE AN KANOYME PERISTROFH 180 NA BGOYME APO THN IF(STATE ==MONING )
//=======================SINARTISEIS============================//

//ARXIKOPOIHSI METRHTH
void timer_set(int time){
	//Clear TCA0 counter
	TCA0.SINGLE.CNT = 0;
	//When the counter reached the limit an interrupt will be occur22
	TCA0.SINGLE.CMP0 = time;
	//Enable TCA0 counting
	TCA0.SINGLE.CTRLA |= 0x01;
	//Enable interrupt on CMP0
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP0_bm;
}

//ARXIKOPOIHSI ADC STO ANALOGO MODE
void ADC_SET(char mode){
	timer = 1 ;
	
	if(mode=='f'){//DEXIS AISTHITIRAS
		ADC0.CTRLA |= ADC_FREERUN_bm;
		ADC0.WINHT |= 200;
		ADC0.CTRLE =ADC_WINCM1_bm; // RES > THR
	}
	else{ //MPROSTA AISTHITIRAS
		ADC0.CTRLA  &=~(ADC_FREERUN_bm | ADC_ENABLE_bm);//ADC single conv;
		ADC0.WINLT  |=10;
		ADC0.CTRLE  =ADC_WINCM0_bm;//0x02;RES < THR
	}		
}

//GIA SWAP TON STROFON SE PERIPTOSH PERISTROFIS 180 MOIRES
void swap(int *left,int *right){
	int temp = *left;
	*left=*right;
	*right=temp;
}

//===================================================//
int main() {
	
	//METAVLITES XRISIMES GIA ARXIKOPOIHSI ADC STO ANALIOGO MODE
	char f = 'f';
	char s = 's';
	
	// PIN3 LEFT turn, PIN2 MOVING, PIN1 RIGHT turn
	PORTD.DIR |= (PIN0_bm|PIN1_bm|PIN2_bm);

	//SET ALL PIN OFF
	PORTD.OUT |= (PIN0_bm|PIN1_bm|PIN2_bm);
	
    //SWITCH GIA PERISTROFH
	PORTF.PIN5CTRL |= (PORT_PULLUPEN_bm | PORT_ISC_FALLING_gc);

    //PRESCALER GIA DIERESI SIXNOTHTAS
	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc;
	TCA0.SINGLE.CTRLB = 0;
	
	//ADC0.CTRLB |= ADC_SAMPNUM_ACC2_gc;
	// Initialize the ADC PRAGMATA POY EINAI KOINA KAI STA DYO MODE
	ADC0.MUXPOS |= ADC_MUXPOS_AIN7_gc;
	ADC0.DBGCTRL |= ADC_DBGRUN_bm;
	ADC0.INTCTRL |= ADC_WCMP_bm;
	
	sei();
	while(1) {
		
		//EXO OLOKLHROSEI KYKLO OTAN PAO FORWARD
		if(direction==FORWARD)
		if ((turns_left - turns_right == 4) & (turns_right >=2))//
		break;
		
		//EXO GYRISEI APO EKEI POY HRTHA
		if (direction == REVERSE)
		if (turns_right < 0  && turns_left == 0)
			break;

		//EDO MPENO OTAN KINOYME
		if (state == MOVING){
			
			adcInt=0;
			
			//LED : 101
			PORTD.OUTCLR = PIN1_bm;
			PORTD.OUTSET = PIN2_bm|PIN0_bm;
			
			while (state == MOVING){
				// Start Conversion
				ADC_SET(f);//ARXIKOPIHSI SE FREERUNING MODE
				timer_set(T1);
				ADC0.CTRLA |= ADC_ENABLE_bm;
				ADC0.COMMAND |= ADC_STCONV_bm;
				
				//EDO MENOYME XRONO T1
				while(timer==1);
				
				if(adcInt==0 && switchInt==0){
					ADC_SET(s);//ARXIKOPIHSI SE SINGLE CONV
					timer_set(T2);
					ADC0.CTRLA |= ADC_ENABLE_bm;
					ADC0.COMMAND |= ADC_STCONV_bm;
					
				while(timer==1);//EDO MENOYME XRONO T2
				}
			}
		}

		//EDO MPAENO OTAN STRIVO ARISTERA
		if (state == TURN_LEFT) {
			PORTF.PIN5CTRL =0x00;
			turns_left +=direction;	//METRAME TIS  ERES
			//LED 011
			PORTD.OUTCLR = PIN2_bm;
			PORTD.OUTSET = PIN1_bm|PIN0_bm;
			
			timer_set(TURN_LEFT_TIME);//KALO SYNARTHSH ARXIKOPOIHSHS METRHTH
			while (state == TURN_LEFT);//MENOYME EDO MEXRI NAN TELEIVSH O XRONOS STROFHS
			PORTF.PIN5CTRL |= (PORT_PULLUPEN_bm | PORT_ISC_FALLING_gc);
		}

		//OTAN STRVOYME DEXIA
		if (state == TURN_RIGHT) {
			PORTF.PIN5CTRL =0x00; 
			turns_right +=direction;	//METRAME TIS STROFES DEXIES
			//LED 110
			PORTD.OUTCLR = PIN0_bm;
			PORTD.OUTSET = PIN2_bm|PIN1_bm;
			
			timer_set(TURN_RIGTH_TIME);//KALO SYNARTHSH ARXIKOPOIHSHS METRHTH
			while (state == TURN_RIGHT);//MENOYME EDO MEXRI NAN TELEIVSH O XRONOS STROFH
			PORTF.PIN5CTRL |= (PORT_PULLUPEN_bm | PORT_ISC_FALLING_gc);
		}
		
		
		
		if (state == TURN_180) {
			
			swap(&turns_left,&turns_right);
			//left_turns += direction;
		
			if (direction == FORWARD) 
				direction = REVERSE;
		    else 
			direction = FORWARD;
			
			//ANABOYME KAI TA TREIA LED
			PORTD.OUTCLR = PIN2_bm|PIN1_bm|PIN0_bm;
			timer_set(TURN_180_TIME);
			while (state == TURN_180);
			switchInt=0;
			timer=1;
			
		}
		
		
		
	}
}


//================ISR=======================================//

ISR(TCA0_CMP0_vect) {

	// DISABLE TCA0
	TCA0.SINGLE.CTRLA &= ~(0x01);
	int intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS = intflags;
	timer = 0;
	state = MOVING;//AFOY EXEI TELEIVSEI H STROFH PAME SE STATE = MOVING
}


ISR(ADC0_WCOMP_vect) {	//interrupt για τον ADC
	
	// DISABLE ADC0
	ADC0.CTRLA &= ~(ADC_ENABLE_bm);
	
	int intflags = ADC0.INTFLAGS;
	timer=0;
	
	//THETO THN SIMEA GIA NA XERO OTI EXV KANEI INTERRUPT
	//MOY XREIAZETE STHN IF(STATE==MOVING)
	adcInt=1;
	ADC0.INTFLAGS = intflags;
	if(ADC0.CTRLA & ADC_FREERUN_bm){//AN FREE RUN MODE
	  if(direction==FORWARD)	
	      state = TURN_RIGHT;
		  else state=TURN_LEFT;
     	}
    else {  //AN SINGLE CONV MODE
	   if(direction==FORWARD)
		    state=TURN_LEFT;
		else state=TURN_RIGHT;	
	   }
}

ISR(PORTF_PORT_vect) {

	// Clear the interrupt flag
	int intflags = PORTF.INTFLAGS;
	PORTF.INTFLAGS = intflags;	
	
    //THA KANOYTME PERISTROFI
	state = TURN_180;	
	timer=0;
	switchInt=1;
	
	}



