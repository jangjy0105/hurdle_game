/*
 * 0528.c
 *
 * Created: 2021-05-28 오후 1:05:26
 * Author: 장재영
 */

#define F_CPU 16000000
 
#include <stdlib.h>
#include <mega128.h>
#include <delay.h>
 
#define CMD_WRITE   0xFC  //명령어쓰기 E=1, RW=0, RS=0
#define DATA_WRITE  0xFD  //데이터쓰기 E=1, RW=0, RS=1
#define LCD_EN      0x04  
 
int i;
int k = -1;
int count = 0;
int level = 9;
int life = 2;
unsigned char position[2] = {0xc0, 0x80};
 
unsigned char fnd_sel[4] = {0b00001000, 0b00000100, 0b00000010, 0b00000001};
unsigned char fnd_data[10] = {0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xd8, 0x80, 0x90};
 
void LCD_cmd_write(char cmd){
    PORTA = CMD_WRITE;
    PORTB = cmd;
    PORTA = PORTA^LCD_EN;
    delay_ms(2);
}
 
void LCD_data_write(char data){
    PORTA = DATA_WRITE;
    PORTB = data;
    PORTA = PORTA^LCD_EN;
    delay_ms(2);
}
 
void LCD_wr_string(char d_line, char *lcd_str){
    LCD_cmd_write(d_line);
    while(*lcd_str != '\0'){
        LCD_data_write(*lcd_str);
        lcd_str++;
    }
}
 
void FND_score(int score){
    int temp = score;
    unsigned char fnd[4];
 
   
    for(i = 0; i < 4; i++){
        fnd[i] = temp%10;
        temp = temp/10;
    }
    for(i = 0; i < 4; i++){
        PORTC = fnd_data[fnd[i]];
        PORTF = fnd_sel[i];
        delay_ms(5);
    }
}
 
void FND_life(void){
    PORTF = fnd_sel[1];
    PORTC = fnd_data[life];
}
 
char generate_hurdle(char *line1, char *line2){
        if(rand()%level == 0 && *(line2+15) == ' ' && *(line2+14) == ' ' && *(line1+14) == ' ' && *(line1+13) == ' ') return '*'; 
        else return ' ';
}
 
char generate_life(char hurdle){
    if(rand()%200 == 0 && life < 8) return '+';
    else return hurdle;
}
 
void LCD_display(char *line1, char *line2){
    LCD_wr_string(position[1] ,line1);
    LCD_wr_string(position[0] ,line2);
    LCD_wr_string(position[k%2],"o");
    LCD_wr_string(position[(k+1)%2]," ");
}
 
void get_life(char life1, char life2){
    if((life1 == '+' && k%2 == 1) || (life2 == '+' && k%2 == 0)){
        life++;
        FND_life();
        for(i = 0; i < 5; i++){
            PORTE = 0x01;
            delay_ms(2);
            PORTE = 0x02;
            delay_ms(2);  
        }
    }    
}
 
void bump(char *line1,char *line2, int position){  
    int score;
    if(*line1 == '*' && k%2 == position){
        life--;
        FND_life();
        for(i = 0; i < 250; i++){
            PORTE = 0x01;
            delay_ms(2);
            PORTE = 0x02;
            delay_ms(2);  
        } 
        k = position;
        if (life == 0){
            for(i = 0; i < 16; i++){
                *(line1+i) = ' ';
                *(line2+i) = ' ';
            }
            LCD_cmd_write(0x01);
            LCD_wr_string(0x80,"   GAME OVER");
            score = count;
            count = 0;
            level = 9;
            life = 2;
            k = -1;
            while(k == -1){FND_score(score);} 
        }
    }    
}
 
void move_line(char *line1, char *line2){
    for(i = 0; i < 15; i++){
        *(line1+i) = *(line1+i+1);
        *(line2+i) = *(line2+i+1);
    }  
}
 
void level_up(){
    if(count%100 == 0 && level != 2){
        level--;
    }
}
 
interrupt [EXT_INT0] void ext_int0_isr(void){
    #asm("cli");   //interrupt disable
    k++;
    #asm("sei"); //interrupt enable
} 
 
void init_LCD(void){
    delay_ms(15);        //15msec 이상 시간지연
    LCD_cmd_write(0x38); //기능셋(데이터버스 8비트, 라인수:2줄)
 
    LCD_cmd_write(0x01);  //화면 지우기
    LCD_cmd_write(0x06);  //엔트리모드셋
    LCD_cmd_write(0x0C);  //표시 On
}
 
void exint0_setting(void){
    EICRA = 0x02;
    EIMSK = 0x01;
} 
 
void init_system(void){
    DDRA = 0xff;
    DDRB = 0xff;
    DDRC = 0xff;
    DDRD = 0x00;
    DDRE = 0xff;
    DDRF = 0xff;
    PORTA = 0xff;   
    PORTB = 0xff;
    PORTC = 0xff;
    PORTE = 0x00;
    PORTF = 0x00;
}
 
void main(void)
{   
    char line1[17] = "                "; 
    char line2[17] = "                ";    
    init_system();
    init_LCD();
    exint0_setting();
                           
    #asm("sei"); //interrupt enable      
                           
    while (1){
        while(k == -1){
            LCD_wr_string(0x80,"  HURDLE GAME");
        }     
        
        FND_life();
                 
        line1[15] = generate_hurdle(line1, line2);
        line2[15] = generate_hurdle(line2, line1);
        
        line1[15] = generate_life(line1[15]);
        line2[15] = generate_life(line2[15]);
        
        LCD_display(line1, line2); 
        
        get_life(line1[0], line2[0]);
        
        bump(line1, line2, 1);
        bump(line2, line1, 0);  
        
        move_line(line1, line2);
        
        count++;
      
        level_up();

        delay_ms(50);
    }
}
