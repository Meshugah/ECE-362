/*
************************************************************************
 ECE 362 - Mini-Project C Source File - Spring 2016
***********************************************************************
	 	   			 		  			 		  		
 Team ID: < 3 >

 Project Name: < Lazer Engraver >

 Team Members:

   
   - Software Leader: < Vignesh Karthikeyan >      Signature: _____Vignesh Karthikeyan_________________

   - Interface Leader: < Greg Macon >     Signature: ______Greg Macon________________

   - Peripheral Leader: < Yizhou Zhang >    Signature: ____Yizhou Zhang__________________

   - Doc Leader: < Tasnova Khandker >      Signature: _____Tasnova Khandker_________________
   

 Academic Honesty Statement:  In signing above, we hereby certify that we 
 are the individuals who created this HC(S)12 source file and that we have
 not copied the work of any other student (past or present) while completing 
 it. We understand that if we fail to honor this agreement, we will receive 
 a grade of ZERO and be subject to possible disciplinary action.

***********************************************************************

 The objective of this Mini-Project is to  < Engrave letters of the alphabet using a laser >


***********************************************************************

 List of project-specific success criteria (functionality that will be
 demonstrated):

 1. Push Buttons being pressed and receieved by one ATD pin 

 2. PWM intensity control of laser

 3. SPI control of LCD

 4. TIM to stop laser from running too long

 5.

***********************************************************************

  Date code started: < 4/21/16 >

  Update history (add an entry every time a significant change is made):

  Date: < 4/23/16 >  Name: < Greg Macon >   Update: < Basic Program Functionality >

  Date: < 4/25/16 >  Name: < Vignesh Karthikeyan >   Update: < Letter Subroutines >

  Date: < 4/27/16 >  Name: < Greg Macon/Vignesh Karthikeyan >   Update: < Final Touches >


***********************************************************************
*/

 #include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include <mc9s12c32.h>


#define ON 1
#define OFF 0

/* All funtions after main should be initialized here */

char inchar(void);
void outchar(char);
void fdisp(void);
void shiftout(char);
void lcdwait(void);
void send_byte(char);
void send_i(char);
void chgline(char);
void print_c(char);
void pmsglcd(char[]);

void dispwait(void);
void timprint(int,int);
//MOTOR FUNCTIONS
void turn(int, int); //motor driver
void move(int, int, int);
void all_reset();
void laser(int);

//LETTER FUNCTIONS
void prntA(void);
void prntB(void);
void prntC(void);
void prntD(void);
void prntE(void);
void prntF(void);
void prntG(void);
void prntH(void);
void prntI(void);
void prntJ(void);
void prntK(void);
void prntL(void);
void prntM(void);
void prntN(void);
void prntO(void);
void prntP(void);
void prntQ(void);
void prntR(void);
void prntS(void);
void prntT(void);
void prntU(void);
void prntV(void);
void prntW(void);
void prntX(void);
void prntY(void);
void prntZ(void);



/*  Variable declarations */           
char leftpb = 0;  // left pushbutton flag
char rghtpb = 0;  // right pushbutton flag
char prevpb = 0;  // previous pushbutton state 
char preva = 0;   // previous select button state (ATD)
char select = 0;  // which button is pushed
char startup = 1; // whether program is in startup mode(1 when in startup mode)     //what can i do with this?
char arr[16] = {0};// character select    
int index = 0;    // the position in the array/(+49) which character you are selecting
int index2 = 0;   // 10s digit
int i = 0;        //??
int count = 0;    //??
int print_ctrl = 0;
int stor = 0;


int buffsize = 0; //stores number of character that are going to be printed
int THRESH1; 
int THRESH2;
int THRESH3;
int THRESH4;
int THRESH5;
long FCONST;        
//Subroutine          SCALE!!!!                                                                    SCALE
long scale = 20;  //total scale of all the variables
int letter_counter = 0;  

//timer
long long timcnt = 0;
int second;
long timhold;
int digit = 0;

/* LCD COMMUNICATION BIT MASKS */
#define RS 0x04 // RS pin mask (PTT[2])
#define RW 0x08 // R/W pin mask (PTT[3])
#define LCDCLK 0x10 // LCD EN/CLK pin mask (PTT[4])

/* LCD INSTRUCTION CHARACTERS */
#define LCDON 0x0F // LCD initialization command
#define LCDCLR 0x01 // LCD clear display command
#define TWOLINE 0x38 // LCD 2-line enable command
#define CURMOV 0xFE // LCD cursor move instruction
#define LINE1 0x80 // LCD line 1 cursor position
#define LINE2 0xC0 // LCD line 2 cursor position

int tc_values[3] = {300, 150, 75};
char tc_index = 0;
char pwm_index = 0;
long o; 


int sample_value = 0;
int saw_tooth = 0;
/*  
***********************************************************************
 Initializations
***********************************************************************
*/

void  initializations(void) {

    THRESH1 = 0x30;//0x30;
    THRESH2 =0x50;// 0x60;
    THRESH3 =0x60; //0x90;
    THRESH4 = 0x90;//0xC0;
    THRESH5 = 0xC0;//0xF0;
    FCONST = 8000;                //SPEED!!! lower is faster                               SPEEEDDDD

/* Set the PLL speed (bus clock = 24 MHz) */
  CLKSEL = CLKSEL & 0x80; //; disengage PLL from system
  PLLCTL = PLLCTL | 0x40; //; turn on PLL
  SYNR = 0x02;            //; set PLL multiplier
  REFDV = 0;              //; set PLL divider
  while (!(CRGFLG & 0x08)){  }
  CLKSEL = CLKSEL | 0x80; //; engage PLL


/* Disable watchdog timer (COPCTL register) */
  COPCTL = 0x40   ; //COP off; RTI and COP stopped in BDM-mode

/* Initialize asynchronous serial port (SCI) for 9600 baud, no interrupts */
  SCIBDH =  0x00; //set baud rate to 9600
  SCIBDL =  0x9C; //24,000,000 / 16 / 156 = 9600 (approx)  
  SCICR1 =  0x00; //$9C = 156
  SCICR2 =  0x0C; //initialize SCI for program-driven operation
  DDRB   =  0x10; //set PB4 for output mode
  PORTB  =  0x10; //assert DTR pin on COM port

         
/* Add additional port pin initializations here */
  DDRT = 0xFF;
  DDRM = 0xFF;
  DDRAD = 0x00;
  ATDDIEN = 0xC0;
  
// Turn on my ATD
  ATDCTL2 = 0x80;
  ATDCTL3 = 0x10;
  ATDCTL4 = 0x85;

/* Initialize the SPI to 6 Mbs */
  SPIBR_SPR0 = 0;
  SPIBR_SPR1 = 0;
  SPIBR_SPR2 = 0;
  
  SPIBR_SPPR0 = 1;
  SPIBR_SPPR1 = 0;
  SPIBR_SPPR2 = 0;
  
  SPICR1 = 0x50;
  SPICR2 = 0;
      
/* Initialize digital I/O port pins */


/* Initialize the LCD
     - pull LCDCLK high (idle)
     - pull R/W' low (write state)
     - turn on LCD (LCDON instruction)
     - enable two-line mode (TWOLINE instruction)
     - clear LCD (LCDCLR instruction)
     - wait for 2ms so that the LCD can wake up     
*/
  for(o = 0;o<100000;o++);
  PTT_PTT4 = 1;
  PTT_PTT3 = 0;
  send_i(LCDON);
  send_i(TWOLINE);
  send_i(LCDCLR);
  lcdwait();    //do i need this?
  
  
/* Initialize RTI for 2.048 ms interrupt rate */ 
  RTICTL = 0x1F;
  CRGINT = 0x80;
  
/* Initialize TIM Ch 7 (TC7) for periodic interrupts every 1.000 ms
     - enable timer subsystem
     - set channel 7 for output compare
     - set appropriate pre-scale factor and enable counter reset after OC7
     - set up channel 7 to generate 1 ms interrupt rate
     - initially disable TIM Ch 7 interrupts      
*/
/*
  TSCR1_TEN = 1;
  TIOS_IOS7 = 1;
  TSCR2_PR0 = 0;
  TSCR2_PR1 = 0;
  TSCR2_PR2 = 1;
  TC7 = 1500;
  TSCR2_TCRE = 1;
  TIE_C7I = 0;
 */
 
  TSCR1 = 0x80;
  TSCR2 = 0x0C;
  TIOS = 0x80;
  TIE = 0x80;
  TC7 = 150;
   
  /* PWM STUFFS */
  PWMPOL = 0x00;  //might need to change 
  PWMCTL = 0x00;  //this is good no concatenate set
  PWMCAE = 0x00;  //this is good                                                          ////dty is strength
  PWMPER0 = 100; //probs good , period is 100
  PWMDTY0 = 5;//could make 10 would probably fix vertical //if ^ thats good this is good , dty is 20 
  PWMCLK = 0x00;  //this is good 
  PWMPRCLK = 0x00;// this is good
  MODRR = 0x01;   // 
  PWME = 0x00; //this is good    (thats what we want before turn on)
  
  
  
 EnableInterrupts;

  
 all_reset();
                  
  
  
      //Ground is up
      //Rightmost is down
      //left one from that is select
      //next is erase
      //left most is start
  
  //initialize CHAR SELECT
  arr[0] = 'A';           //Sets first character to appear on lcd, when it does, to be A
  
  send_i(LCDCLR);
  chgline(LINE1);
  pmsglcd("Select Char: ");     //LINE1 : Select char: 1
  print_c(index + 48);
  print_c(index + 49);          //Prints which index its on "Char 1?"
  chgline(LINE2);                
  print_c(arr[index]);          //LINE2 : shows which character 

  second = 0;
}



/*     
***********************************************************************
 Main
***********************************************************************
*/

void main(void) { 
    
      
    
  
  
  DisableInterrupts;
initializations();     



  for(;;) {
    
    if(startup == 1) 
        {
      //if the program is in startup mode
          /*
          if up button is pushed(select == 1)
          set select to zero(reset button)
          cycle through characters upward 
          */                             
          if(select == 1)//if up button is pressed
              {  
                select = 0;
                arr[index]++;
                if(arr[index] == '[')  //character after capital letters
                    {  
                      arr[index] = 'A';     //rollover characters
                   }
            
                send_i(LCDCLR);
                chgline(LINE1);
                pmsglcd("Select Char: ");
            
                if(index > 9)
                    {
                      index2 = 1;
                    }
                else
                    {
                      index2 = 0;
                    }
            
                print_c(index2 + 48);
                index2 = (index + 1)%10;
                print_c(index2 + 48); // index + 49 = 1
                chgline(LINE2);
           
                i = 0; //space printer
                while(arr[i] != 0)
                  {
                    print_c(arr[i]);
                    i++;
                  }
                i = 0; 
           
               }
  
      /* 
      if down button is pushed
        set select to zero
        cycle through characters downward
      */
          if(select == 2)
              {  //if down button is pressed
                select = 0;
                arr[index]--;
            
                if(arr[index] == '@')
                   {    //character below capital letters
                      arr[index] = 'Z';       //rollover characters
                   }
            
                send_i(LCDCLR);
                chgline(LINE1);
                pmsglcd("Select Char: ");
         
               if(index > 9)
                    {
                      index2 = 1;
                    } 
                else
                    {
                      index2 = 0;
                    }
                print_c(index2 + 48);
                index2 = (index + 1)%10;
                print_c(index2 + 48);
                chgline(LINE2);
           
                i = 0; //Space printer
                while(arr[i] != 0)
                    {
                      print_c(arr[i]);
                      i++;
                    }   
               i = 0;
              }
        
      /*
      if select button is pushed
        set select to zero
        store current character into an array 
        move cursor to the next position
        if number of characters == 16
          print warning   
      */    
          if(select == 3)
              {  //if select button is pressed
                select = 0;
                if(buffsize < 15) 
                    {
                      buffsize++;
                      index++;
                      arr[index] = 'A';
                      send_i(LCDCLR);
                      chgline(LINE1);
                      pmsglcd("Select Char: ");
                    
                      if(index >= 9)
                          {
                            index2 = 1;
                          } 
                      else  
                          {
                            index2 = 0;
                          }
                      print_c(index2 + 48);
                      index2 = (index + 1)%10;
                      print_c(index2 + 48);
                      chgline(LINE2);
            
                      i = 0;   //Space printer
                      while(arr[i] != 0)
                          {
                            print_c(arr[i]);
                            i++;
                          }   
                      i = 0;
         
                    }
        
                else 
                    {
                      chgline(LINE1);
                      send_i(LCDCLR);
                     pmsglcd("Too many letters");  
                    }
          
              }
      
      /*
      if backspace button is pushed
      set select to zero
      erase last character from screen and array
      */
     
          if(select == 4)
              {  //if backspace button is pressed
                select = 0;   
                if(buffsize > 0) 
                    {     
                      buffsize--;
                      arr[index] = 0;
                      index--;
                      send_i(LCDCLR);
                      chgline(LINE1);
                      pmsglcd("Select Char: ");
                      if(index >= 9)
                          {
                            index2 = 1;
                          } 
                      else
                          {
                            index2 = 0;
                          }
                      print_c(index2 + 48);
                      index2 = (index + 1)%10;
                      print_c(index2 + 48);
                      chgline(LINE2);
             
                    i = 0; //Space printer
                    while(arr[i] != 0)
                        {
                          print_c(arr[i]);
                          i++;
                        }   
                    i = 0;
         
          
                    }
                else 
                    {
                      chgline(LINE1);
                      send_i(LCDCLR);
                      pmsglcd("None selected");
                      chgline(LINE2);
                      pmsglcd("Select Char: 01");
                    }
        
              }
      /*
      if start button is pushed
        set select to zero
        print("Are you sure?")
        if you are sure
          start cutting code
          startup = 0
        else
          reset letters
      */       
          if(select == 5)
              {  //if start button is pressed
                select = 0;
                send_i(LCDCLR);
                chgline(LINE1);
                pmsglcd("Ready to print?:");
                chgline(LINE2);
                while(arr[i] != 0)
                    {
                      print_c(arr[i]);
                      i++;
                    }
                i = 1;
                count = 0;
                while(i!=0)
                    {
                      if(select == 4) 
                          { //if chosen to clear
                            select = 0;
                            arr[index] = 'A';
                            send_i(LCDCLR);
                            chgline(LINE1);
                            pmsglcd("Select Char: ");
                            if(index >= 9)
                                {
                                  index2 = 1;
                                } 
                            else
                                {
                                 index2 = 0;
                                }
                            print_c(index2 + 48);
                            index2 = (index + 1)%10;
                            print_c(index2 + 48);
                            chgline(LINE2);
             
                            i = 0;  //Space printer
                            while(arr[i] != 0)
                                {
                                  print_c(arr[i]);
                                  i++;
                                }   
                            i = 0;                 
                          }
                      else if (select == 5)
                          {
                            select = 0;

                            i= 0;
                            //timer printer
                            //dispwait();
                            timcnt = 0;
                            startup = 0;
                            
                          }
                    }
              }
           }
    if(startup == 0)// End of Startup Script     
        {  //This is where we start printing. when done, set startup to 1 to restart the printing, to anything else to stop             
          for(print_ctrl = 0; (arr[print_ctrl] != 0) && (print_ctrl < 16); print_ctrl++) 
              {
                stor = print_ctrl;
                send_i(LCDCLR);
                chgline(LINE1);
                pmsglcd("Printing: ");
                print_c(arr[print_ctrl]);
                /*if(index > 9)
                {
                  index2 = 1;
                } 
                else
                {
                  index2 = 0;
                }
                print_c(index2 + 48);
                index2 = (index + 1)%10;
                print_c(index2 + 48);*/
                chgline(LINE2);
          
                i = 0; //Space printer
                while(arr[i] != 0)
                {
                  print_c(arr[i]);
                  i++;
                }   
                i = 0;
                if(arr[print_ctrl] == 'A')
                    {
                      prntA();
                    }
                     
                if(arr[print_ctrl] == 'B')
                    {
                      prntB();
                    }
                  
                if(arr[print_ctrl] == 'C')  
                    {    
                      prntC();
                    }
                
                if(arr[print_ctrl] == 'D')
                    {
                      prntD();
                    }
                if(arr[print_ctrl] == 'E')
                    {
                      prntE();
                    }
                if(arr[print_ctrl] == 'F')
                    {    
                      prntF();
                    }
                if(arr[print_ctrl] == 'G')
                    {
                      prntG();
                    }
                if(arr[print_ctrl] == 'H')
                    {
                      prntH();
                    }
                if(arr[print_ctrl] == 'I')
                    {    
                      prntI();
                    }
                if(arr[print_ctrl] == 'J')
                    {
                      prntJ();
                    }
                if(arr[print_ctrl] == 'K')
                    {
                      prntK();
                    }
                if(arr[print_ctrl] == 'L')
                    {    
                      prntL();
                    }
                if(arr[print_ctrl] == 'M')
                    {          
                      prntM();
                    }
                if(arr[print_ctrl] == 'N')
                    {
                      prntN();
                    }
                if(arr[print_ctrl] == 'O')
                    {
                      prntO();
                    }
                if(arr[print_ctrl] == 'P')
                    {   
                      prntP();
                    }
                if(arr[print_ctrl] == 'Q')
                    {
                      prntQ();
                    }
                if(arr[print_ctrl] == 'R')
                    {
                      prntR();
                    }
                if(arr[print_ctrl] == 'S')
                    {   
                      prntS();
                    }
                if(arr[print_ctrl] == 'T')
                    {
                      prntT();
                    }
                if(arr[print_ctrl] == 'U')
                    {
                      prntU();
                    }
                if(arr[print_ctrl] == 'V')
                    {   
                      prntV();
                    }
                if(arr[print_ctrl] == 'W')
                    {
                      prntW();
                    }
                if(arr[print_ctrl] == 'X')
                    {
                      prntX();
                    }
                if(arr[print_ctrl] == 'Y')
                    {  
                      prntY();
                    }
                if(arr[print_ctrl] == 'Z')  
                    {   
                      prntZ();
                    }
                
            
             
              }
              
              all_reset();
               arr[0] = 'A';           //Sets first character to appear on lcd, when it does, to be A
  
                send_i(LCDCLR);
                chgline(LINE1);
                pmsglcd("Select Char: ");     //LINE1 : Select char: 1
                print_c(index + 48);
                print_c(index + 49);          //Prints which index its on "Char 1?"
                chgline(LINE2);                
                print_c(arr[index]);          //LINE2 : shows which character 

                second = 0;
             //wher do i put this? //timer display
             /* send_i(LCDCLR);
              chgline(LINE1 );
              pmsglcd("Time taken: ");
              timhold = timcnt;
              
              //fix for varible length
              digit = second/10;
              print_c(digit + 48);
              //timprint(digit + 48, LINE1 + 0x0A);
              digit = (second) % 10;
              print_c(digit + 48);

              //timprint(digit+48, LINE1+0x0B);
              //digit = timhold % 10;
              //timprint(digit+48, LINE1+0x0C);
               */
              startup = 1;//idles
              timcnt = 0;
              second = 0;


                            
       
        }
        
        //and these
     
     
     
        
            
     
    if(startup == 2)
    {
   //do nothing 
     PWME = 0x01;
  
    }
  }  
  
}  
   


  //main is good

/*
***********************************************************************
 RTI interrupt service routine: RTI_ISR
  Initialized for 2.048 ms interrupt rate
  Samples state of pushbuttons (PAD7 = left, PAD6 = right)
  If change in state from "high" to "low" detected, set pushbutton flag
     leftpb (for PAD7 H -> L), rghtpb (for PAD6 H -> L)
     Recall that pushbuttons are momentary contact closures to ground
***********************************************************************
*/

interrupt 7 void RTI_ISR(void)
{
  // clear RTI interrupt flag
  CRGFLG = CRGFLG | 0x80; 

    if(prevpb && !PORTAD0_PTAD6) 
  {
     prevpb = 0;
     rghtpb = 1;
  }
  else if(prevpb && !PORTAD0_PTAD7)
  {
     prevpb = 0;
     leftpb = 1;
  }
  else 
  {
     prevpb = PORTAD0_PTAD7 && PORTAD0_PTAD6;
  }
  
}

/*
***********************************************************************
  TIM interrupt service routine
  used to initiate ATD samples (on Ch 0 and Ch 1)     
***********************************************************************
*/

interrupt 15 void TIM_ISR(void)
{

  // clear TIM CH 7 interrupt flag 
  TFLG1 = TFLG1 | 0x80; 

  ATDCTL5 = 0x10;
  
  timcnt++;
  if(timcnt > 25000){
    timcnt = 0;
    second++;
  }
  
  while(ATDSTAT0 & 0x80 != 0x80);//work on thresholds
  
  if(preva && ATDDR0H < THRESH1) {   //lowest threshold and button was unpressed before
       select = 1;     //select button
       preva = 0;       //makes sure button isn't pressed 2x

  }
  else if(preva && ATDDR0H < THRESH2){
       select = 4;     //up
       preva = 0;
  }
  else if(preva && ATDDR0H < THRESH3){
       select = 3;
       preva = 0;
  }
  else if(preva && ATDDR0H < THRESH4){
       select = 2;
       preva = 0;
  }
  else if(preva && ATDDR0H < THRESH5){
       select = 5;
       preva = 0;
  }
  else 
  {
     //prevpb = PORTAD0_PTAD7 && PORTAD0_PTAD6;
     if(ATDDR0H < THRESH5) { //if a button is pressed preva = 0 else 1
      preva = 0;
     } else{
      preva = 1;
     }
  }
 
  
}



/*
***********************************************************************
  shiftout: Transmits the character x to external shift 
            register using the SPI.  It should shift MSB first.  
             
            MISO = PM[4]
            SCK  = PM[5]
***********************************************************************
*/
 
void shiftout(char x)

{
 
  // read the SPTEF bit, continue if bit is 1
  // write data to SPI data register
  // wait for 30 cycles for SPI data to shift out 
  while(!SPISR_SPTEF);
  
  SPIDR = x;
  
  asm
  {
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
  }    
}

/*
***********************************************************************
  lcdwait: Delay for approx 2 ms
***********************************************************************
*/

void lcdwait()
{
  
  asm 
  {
        pshx
        ldx #16000
    start:
        dbne x, start
        pulx
  } 

}


/*
***********************************************************************
  dispwait: Delay for timer message to be displayed
***********************************************************************
*/

void dispwait()
{
  int t;
  for(t = 0; t<10000;t++) {
    
    asm 
    {
          pshx
          ldx #16000
      start:
          dbne x, start
          pulx
    } 
  }
}

/*
*********************************************************************** 
  send_byte: writes character x to the LCD
***********************************************************************
*/

void send_byte(char x)
{
     // shift out character
     // pulse LCD clock line low->high->low
     // wait 2 ms for LCD to process data
     shiftout(x);
     PTT_PTT4 = 0;
     PTT_PTT4 = 1;
     PTT_PTT4 = 0;
     lcdwait();
}

/*
***********************************************************************
  send_i: Sends instruction byte x to LCD  
***********************************************************************
*/

void send_i(char x)
{
      // set the register select line low (instruction data)
      // send byte
      PTT_PTT2 = 0;
      send_byte(x);
}

/*
***********************************************************************
  chgline: Move LCD cursor to position x
  NOTE: Cursor positions are encoded in the LINE1/LINE2 variables
***********************************************************************
*/

void chgline(char x)
{
    send_i(CURMOV);
    send_i(x);
}

/*
***********************************************************************
  print_c: Print (single) character x on LCD            
***********************************************************************
*/
 
void print_c(char x)
{
    PTT_PTT2 = 1;
    send_byte(x);
}



 /***********************************************************************                       
;  timprint: Print character passed in register A on LCD ,            
;***********************************************************************/
void timprint(int c, int pos)
{
  chgline(pos);
  send_byte(c);
}





/*
***********************************************************************
  pmsglcd: print character string str[] on LCD
***********************************************************************
*/

void pmsglcd(char str[])
{
    int i = 0;
    
    while(str[i] !='\0')
    {
      print_c(str[i++]);
    }
}







//SUBROUTINE PART




/*
***********************************************************************
 Name:         printselect
 Description:  displays the select letter prompt 
 Example:      printselect();
***********************************************************************

void printselect(){
  
  send_i(LCDCLR);
  chgline(LINE1);
  pmsglcd("Select Char: ");
        
  if(index > 9)
  {
    index2 = 1;
  } 
  else
  {
    index2 = 0;
  }
  print_c(index2 + 48);
  index2 = (index + 1)%10;
  print_c(index2 + 48);
  chgline(LINE2);
          
  i = 0; //Space printer
  while(arr[i] != 0)
  {
    print_c(arr[i]);
  i++;
  }   
  i = 0;
}

*/   



/*
***********************************************************************
 Name:         turn
 Description:  turns the motors(forwards and back, x and y)
 Example:      outchar(1, -1);
***********************************************************************
*/

void turn(int x, int y) {
  long i;
  int j = PWME;
  if(second > 90){
    PWME = 0;
    send_i(LCDCLR);
    chgline(LINE1 );
    pmsglcd("Wait for");
    chgline(LINE2 );
    pmsglcd("Laser cooldown");
    while(second < 120);
    second = 0;
    send_i(LCDCLR);
    chgline(LINE1);
    pmsglcd("Printing:");
    chgline(LINE2 );
    i = 0;
    while(arr[i] != 0)
    {
      print_c(arr[i]);
      i++;
    }   
    i = 0;
  } 
  
  
  
  
  
   
  if(select==5) {
    PWME = 0;
    select = 0;
    send_i(LCDCLR);
    chgline(LINE1 );
    pmsglcd("Paused...");
    while(select != 5){
      if(select >0 && select < 5){
        select = 0;
      }
    }
    select = 0;
    send_i(LCDCLR);
    chgline(LINE1);
    pmsglcd("Printing:");
   print_c(arr[stor]);
  }
  
  
  
  
  
  
  
  
  PWME = j;
    
  if(x > 0) {
    PTT_PTT5 = 0;
    for(i = 0; i < FCONST; i++);
    PTT_PTT1 = 0;
    for(i = 0; i < FCONST; i++);    
    PTT_PTT5 = 1;
    for(i = 0; i < FCONST; i++);    
    PTT_PTT1 = 1;
    for(i = 0; i < FCONST; i++);
  }
    
  if(x < 0) {
    PTT_PTT1 = 0;
    for(i = 0; i < FCONST; i++);
    PTT_PTT5 = 0;
    for(i = 0; i < FCONST; i++);    
    PTT_PTT1 = 1;
    for(i = 0; i < FCONST; i++);    
    PTT_PTT5 = 1;
    for(i = 0; i < FCONST; i++);
  }
  
   if(y < 0) {
    PTT_PTT6 = 0;
    for(i = 0; i < FCONST; i++);
    PTT_PTT7 = 0;
    for(i = 0; i < FCONST; i++);    
    PTT_PTT6 = 1;
    for(i = 0; i < FCONST; i++);    
    PTT_PTT7 = 1;
    for(i = 0; i < FCONST; i++);
   }
    
  if(y > 0) {
    PTT_PTT7 = 0;
    for(i = 0; i < FCONST; i++);
    PTT_PTT6 = 0;
    for(i = 0; i < FCONST; i++);    
    PTT_PTT7 = 1;
    for(i = 0; i < FCONST; i++);    
    PTT_PTT6 = 1;
    for(i = 0; i < FCONST; i++);
  }
  return;
}
 
 

/*
***********************************************************************
 Name:         move
 Description:  turns the motors(3 param: passes x and y to turn(), dist variable for distance, scale as a global variable)
 Example:      move(1, -1, 6);
***********************************************************************
*/

 
  void move(int x, int y, int space) 
  {
    int j;
    for(j = 0; j < (space * scale); j++)
      { 
      turn(x,y);
      }
    return;
  }
 

/*
***********************************************************************
 Name:         all_reset
 Description:  moves back laser pointer to lower left corner of the printer line, takes in print_control to move back
***********************************************************************
*/

 void all_reset() 
 {
    laser(OFF);
    send_i(LCDCLR);
    chgline(LINE1);
    pmsglcd("Resetting X");
    while(leftpb == 0) {
      turn(-1,0);
    }
    move(1,0,5);
    send_i(LCDCLR);
    chgline(LINE1);
    pmsglcd("Resetting Y");
    while(rghtpb == 0) {
      turn(0,1);
    }
        
    move(0,-1,16);
    leftpb = 0;
    rghtpb = 0;
    
    return;
 }
 
/*
***********************************************************************
 Name:         laser
 Description:  turns laser ON/OFF
***********************************************************************
*/

void laser(int power)
{
  if(power == OFF){
   PWME = 0x00;
  }
  if(power == ON){
   PWME = 0x01;
  }
return;
}


//LETTER FUNCTIONS
void prntA()
{
  laser(OFF);
  
  //start position
  move(1,0,1); 
  
  //all outer
  laser(ON);
  move(0,1,7);
  move(1,1,3); 
  move(1,0,2);
  move(1,-1,3);
  move(0,-1,7);
  move(-1,0,2);
  move(0,1,4);
  move(-1,0,4);
  move(0,-1,4);
  move(-1,0,2);
  laser(OFF);
  
  //move to inside
  move(1,0,2);
  move(0,1,6);

  
  //inside
  laser(ON);
  move(0,1,1);
  move(1,1,1);
  move(1,0,2);
  move(1,-1,1);
  move(0,-1,1);
  move(-1,0,4);
  laser(OFF);
  
  //move to final
  move(0,-1,6); 
  move(1,0,6);
  
  return;
}

void prntB()
{
  laser(OFF);

  //start position
  move(1,0,1); 
  
  //all outer
  laser(ON);
  move(0,1,10);
  move(1,0,6);
  move(1,-1,2);
  move(0,-1,2);
  move(-1,-1,1);
  move(1,-1,1);
  move(0,-1,2);
  move(-1,-1,2);
  move(-1,0,6);
  laser(OFF);
  
  //move to inner down
  move(1,0,2);
  move(0,1,2);
  
  //inner shape
  laser(ON);
  move(1,0,3);
  move(1,1,1);
  move(-1,1,1);
  move(-1,0,3);
  move(0,-1,2);
  laser(OFF);
  
  //move to inner up
  move(0,1,4);
  
  //inner shape
  laser(ON);
  move(1,0,3);
  move(1,1,1);
  move(-1,1,1);
  move(-1,0,3);
  move(0,-1,2);
  laser(OFF);
  
  //move to final
  move(0,-1,6);
  move(1,0,7);
  
  return;
}


void prntC()
{
  laser(OFF);

  //start position
  move(1,0,4); 
  
  //all outer
  laser(ON);
  move(1,0,5);
  move(0,1,2);
  move(-1,0,4);
  move(-1,1,2);
  move(0,1,2);
  move(1,1,2);
  move(1,0,4);
  move(0,1,2);
  move(-1,0,5);
  move(-1,-1,3);
  move(0,-1,4);
  move(1,-1,3);
  laser(OFF);
  
  //move to final
  move(1,0,6);

  return;
}

void prntD()
{
  laser(OFF);

  //start position
  move(1,0,1);
  
  //all outer
  laser(ON);
  move(0,1,10);
  move(1,0,5);
  move(1,-1,3);
  move(0,-1,4);
  move(-1,-1,3);
  move(-1,0,5);
  laser(OFF);
  
  //move to inner
  move(1,0,2);
  move(0,1,2);
  
  //inner shape
  laser(ON);
  move(1,0,2);
  move(1,1,2);
  move(0,1,2);
  move(-1,1,2);
  move(-1,0,2);
  move(0,-1,6);
  laser(OFF);
  
  //move to final
  move(0,-1,2);
  move(1,0,7);
  
 
  return;
}


void prntE()
{
 
//start position
move(1,0,1);

//outer
laser(ON);
move(0,1,10);
move(1,0,8);
move(0,-1,2);
move(-1,0,6);
move(0,-1,2);
move(1,0,6);
move(0,-1,2);
move(-1,0,6);
move(0,-1,2);

move(1,0,6);
move(0,-1,2);
move(-1,0,8);
laser(OFF);
//end poistion
move(1,0,9); 
  
  return;
}
void prntF()
{
   //start position
move(1,0,1);

//outer
laser(ON);
move(0,1,10);
move(1,0,8);
move(0,-1,2);
move(-1,0,6);
move(0,-1,2);
move(1,0,6);
move(0,-1,2);
move(-1,0,6);
move(0,-1,4);
move(-1,0,2);
laser(OFF);

//end poistion
move(1,0,9);
  return;
}

void prntG()
{
     //start position
move(1,0,3);

//Outer
laser(ON);
move(-1,1,2);
move(0,1,6);
move(1,1,2);
move(1,0,4);
move(1,-1,2);
move(0,-1,2);
move(-1,0,2);
move(0,1,2);
move(-1,0,4);
move(0,-1,6);
move(1,0,4);
move(0,1,1);
move(-1,0,2);
move(0,1,2);
move(1,0,4);
move(0,-1,3);
move(-1,-1,2);
move(-1,0,4);
laser(OFF);

//end position
move(1,0,7);

  
  return;
}
void prntH()
{
  move(1,0,1);

//Outer
laser(ON);
move(0,1,10);
move(1,0,2);
move(0,-1,4);
move(1,0,4);
move(0,1,4);
move(1,0,2);
move(0,-1,10);
move(-1,0,2);
move(0,1,4);
move(-1,0,4);
move(0,-1,4);
move(-1,0,2);
laser(OFF);

//end position
move(1,0,9);
 
  return;
}

void prntI()
{
  //start position
move(1,0,1);

//Outer
laser(ON);
move(1,0,8);
move(0,1,2);
move(-1,0,3);
move(0,1,6);
move(1,0,3);
move(0,1,2);
move(-1,0,8);
move(0,-1,2);
move(1,0,3);
move(0,-1,6);
move(-1,0,3);
move(0,-1,2);
laser(OFF);

//end position
move(1,0,9);
  return;
}

void prntJ()
{
  //start position
move(1,0,2);

//Outer 
laser(ON);
move(-1,1,1);
move(0,1,2);
move(1,0,2);
move(0,-1,1);
move(1,0,1);
move(0,1,6);
move(-1,0,3);
move(0,1,2);
move(1,0,8);
move(0,-1,2);
move(-1,0,3);
move(0,-1,8);
move(-1,0,4);
laser(OFF);

//end position
move(1,0,8);
  
  return;
}


void prntK()
{
 //start position
move(1,0,1);

//Outer
laser(ON);
move(1,0,2);
move(0,1,3);
move(1,-1,3);
move(1,0,3);
move(-1,1,5);
move(1,1,5);
move(-1,0,3);
move(-1,-1,3);
move(0,1,3);
move(-1,0,2);
move(0,-1,10);
laser(OFF);

//end poistion
move(1,0,9); 
  return;
}


void prntL()
{
  //start position
move(1,0,1);

//Outer
laser(ON);
move(1,0,8);
move(0,1,2);
move(-1,0,6);
move(0,1,8);
move(-1,0,2);
move(0,-1,10);
laser(OFF);

//end position
move(1,0,9);
  return;
}


void prntM()
{
  //start position
move(1,0,1);

//Outer
laser(ON);
move(1,0,2);
move(0,1,7);
move(1,-1,2);
move(1,1,2);
move(0,-1,7);
move(1,0,2);
move(0,1,10);
move(-1,0,2);
move(-1,-1,2);
move(-1,1,2);
move(-1,0,2);
move(0,-1,10);
laser(OFF);

//end position
move(1,0,9);
  return;
}

void prntN()
{
  //start position
move(1,0,1);

//Outer
laser(ON);
move(1,0,2);
move(0,1,6);
move(1,-1,4);
move(0,-1,2);
move(1,0,2);
move(0,1,10);
move(-1,0,2);
move(0,-1,6);
move(-1,1,4);
move(0,1,2);
move(-1,0,2);
move(0,-1,10);
laser(OFF);

//end position
move(1,0,9);
  return;
}
void prntO()
{
   //start position
move(1,0,3);

//outer
laser(ON);
move(1,0,4);
move(1,1,2);
move(0,1,6);
move(-1,1,2);
move(-1,0,4);
move(-1,-1,2);
move(0,-1,6);
move(1,-1,2);
laser(OFF);

//move to inner
move(0,1,3);

//inner
laser(ON);
move(1,-1,1);
move(1,0,2);
move(1,1,1);
move(0,1,4);
move(-1,1,1);
move(-1,0,2);
move(-1,-1,1);
move(0,-1,4);
laser(OFF);

//final position
move(0,-1,3);
move(1,0,7);
  return;
}
void prntP()
{
  //start position
move(1,0,1);

//outer
laser(ON);
move(0,1,10);
move(1,0,6);
move(1,-1,2);
move(0,-1,2);
move(-1,-1,2);
move(-1,0,4);
move(0,-1,4);
move(-1,0,2);
laser(OFF);

//move to inner
move(1,0,2);
move(0,1,6);

//inner
laser(ON);
move(0,1,2);
move(1,0,3);
move(1,-1,1);
move(-1,-1,1);
move(-1,0,3);
laser(OFF);

//final position
move(0,-1,6);
move(1,0,7);

  return;
}
void prntQ()
{
 //start position
 move(1,0,3);
 
 //outer
 laser(ON);
 move(1,0,3);
 move(1,1,1);
 move(1,-1,1);
 move(1,1,1);
 move(-1,1,1);
 move(0,1,5);
 move(-1,1,2);
 move(-1,0,4);
 move(-1,-1,2);
 move(0,-1,6);
 move(1,-1,2);
 laser(OFF);
 
 //move to inner
 move(0,1,2);
 
 //inner
 laser(ON);
 move(1,-1,1);
 move(1,0,2);
 move(-1,1,1);
 move(1,1,1);
 move(1,-1,1);
 move(0,1,4);
 move(-1,1,1);
 move(-1,0,2);
 move(-1,-1,1);
 move(0,-1,4);
 laser(OFF);
 
 //end position
 move(0,-1,3);
 move(1,0,7);
   

  return;
}

void prntR()
{
       //start position
move(1,0,1);

//outer
laser(ON);
move(0,1,10);
move(1,0,6);
move(1,-1,2);
move(0,-1,2);
move(-1,-1,2);
move(-1,0,2);
move(1,-1,3);
move(-1,0,3);
move(-1,1,2);
move(0,-1,2);
move(-1,0,2);
laser(OFF);

//move to inner
move(1,0,2);
move(0,1,6);

//inner
laser(ON);
move(0,1,2);
move(1,0,3);
move(1,-1,1);
move(-1,-1,1);
move(-1,0,3);
laser(OFF);

//final position
move(0,-1,6);
move(1,0,7);

  return;
}
void prntS()
{
   //start position
  move(1,0,3);
  
  //outer
  laser(ON);
  move(1,0,4);
  move(1,1,2);
  move(0,1,2);
  move(-1,1,2);
  move(-1,0,4);
  move(0,1,2);
  move(1,0,4);
  move(0,-1,1);
  move(1,0,2);
  move(0,1,1);
  move(-1,1,2);
  move(-1,0,4);
  move(-1,-1,2);
  move(0,-1,2);
  move(1,-1,2);
  move(1,0,4);
  move(0,-1,2);
  move(-1,0,4);
  move(0,1,1);
  move(-1,0,2);
  move(0,-1,1);
  move(1,-1,2);
  laser(OFF);
  
  //final position
  move(1,0,7);
  
  return;
}
void prntT()
{
  move(1,0,4);

//outer
laser(ON);
move(0,1,8);
move(-1,0,3);
move(0,1,2);
move(1,0,8);
move(0,-1,2);
move(-1,0,3);
move(0,-1,8);
move(-1,0,2);
laser(OFF);

//final position
move(1,0,6);
  return;
}
void prntU()
{
  //start position
move(1,0,3);

//Outer
laser(ON);
move(1,0,4);
move(1,1,2);
move(0,1,8);
move(-1,0,2);
move(0,-1,7);
move(-1,-1,1);
move(-1,0,2);
move(-1,1,1);
move(0,1,7);
move(-1,0,2);
move(0,-1,8);
move(1,-1,2);
laser(OFF);

//final position
move(1,0,7);

  return;
}
void prntV()
{
 //start position
move(1,0,4);

//outer
laser(ON);
move(-1,1,1);
move(0,1,2);
move(-1,1,1);
move(0,1,2);
move(-1,1,1);
move(0,1,3);

move(1,0,2);
move(0,-1,2);
move(1,-1,1);
move(0,-1,3);
move(1,-1,1);
move(1,1,1);
move(0,1,3);
move(1,1,1);
move(0,1,2);
move(1,0,2);
move(0,-1,3);
move(-1,-1,1);
move(0,-1,2);
move(-1,-1,1);
move(0,-1,2);
move(-1,-1,1);
move(-1,0,2);
laser(OFF);

//End position
move(1,0,6);
 
  return;
}
void prntW()
{
  //start position
move(1,0,3);

//outer
laser(ON);
move(-1,1,2);
move(0,1,8);
move(1,0,2);
move(0,-1,7);
move(1,1,2);
move(1,-1,2);
move(0,1,7);
move(1,0,2);
move(0,-1,8);
move(-1,-1,2);
move(-1,1,2);
move(-1,-1,2);
laser(OFF);

//end position
move(1,0,7);
  return;
}
void prntX()
{
  //start position
  move(1,0,1);
  
  //outer
  laser(ON);
  move(1,0,2);
  move(0,1,2);
  move(1,1,2);
  move(1,-1,2);
  move(0,-1,2);
  move(1,0,2);
  move(0,1,2);
  move(-1,1,4);
  move(1,1,4);
  move(0,1,2);
  move(-1,0,2);
  move(0,-1,2);
  move(-1,-1,2);
  move(-1,1,2);
  move(0,1,2);
  move(-1,0,2);
  move(0,-1,2);
  move(1,-1,4);
  move(-1,-1,4);
  move(0,-1,2);
  laser(OFF);
  
  //end position
  move(1,0,9);  

return;
}
void prntY()
{
  //start position
move(1,0,4);

//outer
laser(ON);
move(1,0,2);
move(0,1,5);
move(1,1,3);
move(0,1,2);
move(-1,0,1);
move(-1,-1,3);
move(-1,1,3);
move(-1,0,1);
move(0,-1,2);
move(1,-1,3);
move(0,-1,5);
laser(OFF);

//end poistion
move(1,0,4);
  return;
}
void prntZ()
{
 //start position
move(1,0,1);

//outer
laser(ON);
move(1,0,8);
move(0,1,2);
move(-1,0,6);
move(1,1,6);
move(0,1,2);
move(-1,0,8);
move(0,-1,2);
move(1,0,6);
move(-1,-1,6);
move(0,-1,2);
laser(OFF);

//end position;
move(1,0,9);
 
  return;
}