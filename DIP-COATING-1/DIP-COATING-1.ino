
// Coloca no pino 6 a frequencia de 440Hz por 200millisegundos
//  tone(6, 440, 200);
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <max6675.h>
#include <PID_v1.h>
extern volatile unsigned long timer0_millis; // zerar finção millis()
unsigned long lastTempUpdate, windowStartTime;

// Menu
#define pino_botao_ok A1
#define pino_botao_dir A2
#define pino_botao_esq A3

// Mecanismo de mergulho
#define pin_altura_inicial 2  // fim de curso da altura 
#define pin_step_mp 3         
#define pin_direcao_mp 4
#define pin_enable_mp 10
#define fim_curso_baker_aberto 5
#define fim_curso_baker_fechado 6
#define pin_motor_baker_a 7
#define pin_motor_baker_b 8

// Forno

#define WINDOW_SIZE        2000
#define MINIMUM_RELAY_TIME 500
#define RELAY_PIN          3
#define TEMP_READ_DELAY    100
#define KP                 15
#define KI                 10
#define KD                 10


// Variaveis fixas
int etapa_do_ciclo;
int altura_inicial_stado;
long tmp_altura_inicial;
int fim_de_curso_aberto_stado;
int fim_de_curso_fechado_stado;
long tmp_FC_fechado;
long tmp_FC_aberto;
long Tempo_decorrido;

// Variaveis menu

int menu_print=1;
int variavel_menu=0;
long tempo_menu;
long tempo_menu_dir;
long tempo_menu_esq;
long tempo_menu_max=0;
int botao_menu_stado=0;
int botao_dir_stado=0;
int botao_esq_stado=0;
int estagio_menu =0;
int variavel_menu_display = 0;

int inf_print_menu=100;
int inf_menu_estagio_2=0;


// Variaveis mutaveis do ciclo (definir valores defolt)
int velocidade_backer_decida;
int velocidade_backer_subida;
int altura_backer;
long tempo_backer; 
int velocidade_forno_decida;
int altura_forno;
long tempo_forno;
int velocidade_forno_subida;
long tempo_entre_ciclo;
int temperatura_forno;


LiquidCrystal_I2C lcd(0x27,16,2);

void AbreBaker(){
  digitalWrite(pin_motor_baker_a,LOW);
  digitalWrite(pin_motor_baker_b,HIGH);
}

void FechaBaker(){
  digitalWrite(pin_motor_baker_a,HIGH);
  digitalWrite(pin_motor_baker_b,LOW);
}

void ParaBaker(){
  digitalWrite(pin_motor_baker_a,HIGH);
  digitalWrite(pin_motor_baker_b,HIGH);
}

void AlturaInicial(){
  if(digitalRead(pin_altura_inicial)==LOW && altura_inicial_stado==0 ){
    tmp_altura_inicial=millis();
    altura_inicial_stado=2;
  }
  if(digitalRead(pin_altura_inicial)==HIGH ){
   altura_inicial_stado=0;
  }
  if(digitalRead(pin_altura_inicial)==LOW && altura_inicial_stado==2 && millis()-tmp_altura_inicial>=100){
    altura_inicial_stado=1;
  }
}

void FimDeCurso(){
  if(digitalRead(fim_curso_baker_fechado)==HIGH && fim_de_curso_fechado_stado==0 ){
    tmp_FC_fechado = millis();
    fim_de_curso_fechado_stado=2;
  }
  if(digitalRead(fim_curso_baker_fechado)==LOW ){
   fim_de_curso_fechado_stado=0;
  }
  if(digitalRead(fim_curso_baker_fechado)==HIGH && fim_de_curso_fechado_stado==2 && millis()-tmp_FC_fechado>=150){
    fim_de_curso_fechado_stado=1;
  }

  
  if(digitalRead(fim_curso_baker_aberto)==HIGH && fim_de_curso_aberto_stado==0 ){
    tmp_FC_aberto = millis();
    fim_de_curso_aberto_stado=2;
  }
  if(digitalRead(fim_curso_baker_aberto)==LOW ){
   fim_de_curso_aberto_stado=0;
  }
  if(digitalRead(fim_curso_baker_aberto)==HIGH && fim_de_curso_aberto_stado==2 && millis()-tmp_FC_aberto>=150){
    fim_de_curso_aberto_stado=1;
  }
}

void setup() {
   Wire.begin();
  Serial.begin(9600);
  
  pinMode (pin_altura_inicial, INPUT);
  pinMode (pin_step_mp, OUTPUT);
  pinMode(pin_direcao_mp,OUTPUT); 
  pinMode(pin_enable_mp, OUTPUT);
  pinMode(fim_curso_baker_aberto, INPUT);
  pinMode(fim_curso_baker_fechado, INPUT);
  pinMode(pin_motor_baker_a,OUTPUT); 
  pinMode(pin_motor_baker_b, OUTPUT);



  
  etapa_do_ciclo  =   -3;
  altura_inicial_stado=0;
  ParaBaker();
  digitalWrite(pin_enable_mp,HIGH);

  lcd.init();  
  lcd.setBacklight(HIGH);
  lcd.clear();
          lcd.setCursor(0,0);
          lcd.print(estagio_menu);
  while (etapa_do_ciclo!=0){
  Menu();
  

  }

          
  velocidade_backer_decida= 1;//velocidade de cm/s
  altura_backer=5;//em cm a partir da base
  tempo_backer= 30; // em segundos
  velocidade_backer_subida= 2;//velocidade de cm/s
  velocidade_forno_decida= 5;//velocidade de cm/s
  altura_forno=-15;// em cm para baixo da base
  tempo_forno=400;      // em segundos
  velocidade_forno_subida= 5;//velocidade de cm/s
  tempo_entre_ciclo= 30; // em segundos
  temperatura_forno=100;// em Celcius
  
  while (etapa_do_ciclo!=0){
    if(etapa_do_ciclo==-3){
      AlturaInicial();
      if(altura_inicial_stado==1){
        etapa_do_ciclo=-2;
        noTone(pin_step_mp);
        digitalWrite(pin_enable_mp,HIGH); // rodar o tone e verificar o fim de curso , quando acionado dar um notone
      }
      else{
        digitalWrite(pin_enable_mp,LOW);
        digitalWrite(pin_direcao_mp,LOW);
        tone(pin_step_mp,1000,200);
      }
    } 
    if(etapa_do_ciclo==-2){
      digitalWrite(pin_enable_mp,LOW);
      digitalWrite(pin_direcao_mp,HIGH);
      tone(pin_step_mp,1000,400);
      delay(800);
      etapa_do_ciclo=-1;
    }
    if(etapa_do_ciclo==-1){
      FimDeCurso();
      if(fim_de_curso_fechado_stado==1){
        etapa_do_ciclo=0;
        ParaBaker();
      }
      else{
        FechaBaker();
      }
    }
 
  }
  // definir se vai ser o manual ou pelo app
  etapa_do_ciclo=-2;
  while (etapa_do_ciclo!=0){
    if(etapa_do_ciclo==-2){
      digitalWrite(pin_enable_mp,LOW);
      digitalWrite(pin_direcao_mp,HIGH);
      tone(pin_step_mp,1000,400);
      delay(800);
      etapa_do_ciclo=-1;
    }
    if(etapa_do_ciclo==-1){
      FimDeCurso();
      if(fim_de_curso_fechado_stado==1){
        etapa_do_ciclo=0;
        ParaBaker();
      }
      else{
        FechaBaker();
      }
    }
    
  }

  
  
  
}
//ESTADO INICIAL -----------------------------------

void loop(){
  Serial.print(etapa_do_ciclo);

  if(etapa_do_ciclo==0){
    //Seleção das configuraçoes iniciais: tempo de forno, temperatura...
    Menu();
    // condicão de temperatura
  }
  if(etapa_do_ciclo==1){
    // abrir backer
  }
  if(etapa_do_ciclo==2){
    // Decer Filme
  }
  if(etapa_do_ciclo==3){
    // tempo de mergulho backer
  }
  if(etapa_do_ciclo==4){
    // Subir Filme
  }
  if(etapa_do_ciclo==5){
    // Recolher Backer
  }
  if(etapa_do_ciclo==6){
    // Decer filme para o forno
  }
  if(etapa_do_ciclo==7){
    // Tempo dentro do forno
  }
  if(etapa_do_ciclo==8){
    // Subir Filme do forno
  }
  if(etapa_do_ciclo==10){
    // tempo de descanso do forno
  }
  if(etapa_do_ciclo==11){
    // Configuração do final de ciclo
  }
  if(etapa_do_ciclo==12){
    // Configuração final de todos os filmes
  }

  // Controle PID da Temperatura



  // Imprime as informaçoes de temperatura

  
}

void Menu(){
  TempoMax();
if(estagio_menu==0){    
    if(digitalRead(pino_botao_ok) == HIGH && botao_menu_stado==0){
      botao_menu_stado=1; 
      tempo_menu=millis();
      tempo_menu_max=millis();
    }  
    if(digitalRead(pino_botao_ok) == HIGH && botao_menu_stado==1 && millis()-tempo_menu>=1851){
      botao_menu_stado=0;
      estagio_menu=100;
    }
    if(digitalRead(pino_botao_ok) == LOW && botao_menu_stado==1){
      botao_menu_stado=0; 
      
    } 
  }
  
  if(estagio_menu==100){
    
    if(botao_menu_stado==0){
      // PRINT MENU
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("     MENU");
      tempo_menu=millis();
      tempo_menu_max=millis();
      botao_menu_stado=1;
      }
      if(botao_menu_stado==1 && millis()-tempo_menu>=900 &&digitalRead(pino_botao_ok) == LOW ){
        botao_menu_stado=0;
        estagio_menu=1;
        variavel_menu=0;
        inf_menu_estagio_2=0;
        menu_print=1;
      }
    }


  
  // Tela de seleção : Menu ; Iniciar; Visualizar.
  if(estagio_menu==1){
    if(digitalRead(pino_botao_dir) == HIGH && botao_dir_stado==0){
      botao_dir_stado=1;
      tempo_menu_dir=millis();
    }
    if(digitalRead(pino_botao_dir) == LOW && botao_dir_stado==1){  
      if(millis()-tempo_menu_dir>=280 ){
        variavel_menu=(variavel_menu)%2+1;// Condiç-ao de contorno:::
        tempo_menu_max=millis();
        botao_dir_stado=0;
      }
    }

  
    if(digitalRead(pino_botao_esq) == HIGH && botao_esq_stado==0){
      botao_esq_stado=1;
      tempo_menu_dir=millis();
    }
    if(digitalRead(pino_botao_esq) == LOW && botao_esq_stado==1){  
      if(millis()-tempo_menu_esq>=280 ){
        if(variavel_menu==0){
          variavel_menu=2;
        }
        else{
          variavel_menu=(variavel_menu-1)%2; // Aplicar condição de contorno
        }
        tempo_menu_max=millis();
        botao_esq_stado=0;
      }
    }
    

    if(inf_print_menu!=variavel_menu){
      inf_print_menu=variavel_menu;
        // Mostra o nome da opção do estagio 1 do menu
      lcd.setCursor(0,0);
      lcd.print("                ");
      lcd.setCursor(0,0);
      switch(variavel_menu){
          case 0:{
            lcd.setCursor(3,0);
            lcd.print("Iniciar");
          }
          break;
          case 1:{
            lcd.setCursor(6,0);
            lcd.print("Menu");// COLOCAR A TEMPERATURA
          }
          break;
          case 2:{
            lcd.setCursor(0,0);
            lcd.print("Visualizar Inf.");
          }
      }
    }
    if(digitalRead(pino_botao_ok) == HIGH && botao_menu_stado==0){
      botao_menu_stado=1;
      tempo_menu=millis();
    }
    if(digitalRead(pino_botao_ok) == LOW && botao_menu_stado==1){  
      if(millis()-tempo_menu>=280){
        estagio_menu = 2;
        inf_print_menu=100;
        tempo_menu_max=millis();
        botao_menu_stado=0;
      }
    }
  }
  
  if(estagio_menu==2){
    
    switch(variavel_menu){
      case 0:{
        //Iniciar
        etapa_do_ciclo=1;
        if (menu_print!=0){
          Tempo_decorrido=millis();
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("LOGO INICIANDO");
          menu_print=0;
        }
        if(millis()-Tempo_decorrido>=3000){
          etapa_do_ciclo=1;
          Tempo_decorrido=millis();
        }
        
      }
      break;
      case 1:estagio_menu=3;
      break;
      case 2:MostraVariaveis();
      break;
     
    }
  }
}

void TempoMax(){
  if(millis()-tempo_menu_max>=10000){
      estagio_menu=0;
      variavel_menu=0;
      variavel_menu_display =0;
      lcd.clear();
    }
}


void MostraVariaveis(){
  
   if(digitalRead(pino_botao_dir) == HIGH && botao_dir_stado==0){
      botao_dir_stado=1;
      tempo_menu_dir=millis();
    }
    if(digitalRead(pino_botao_dir) == LOW && botao_dir_stado==1){  
      if(millis()-tempo_menu_dir>=280 ){
        variavel_menu_display=(variavel_menu_display)%9+1;// Condiç-ao de contorno:::
        tempo_menu_max=millis();
        botao_dir_stado=0;
      }
    }

  
    if(digitalRead(pino_botao_esq) == HIGH && botao_esq_stado==0){
      botao_esq_stado=1;
      tempo_menu_dir=millis();
    }
    if(digitalRead(pino_botao_esq) == LOW && botao_esq_stado==1){  
      if(millis()-tempo_menu_esq>=280 ){
        if(variavel_menu_display==0){
          variavel_menu_display=9;
        }
        else{
          variavel_menu_display=(variavel_menu_display-1)%9; // Aplicar condição de contorno
        }
        tempo_menu_max=millis();
        botao_esq_stado=0;
      }
    }

    if(inf_print_menu!=variavel_menu_display){
      inf_print_menu=variavel_menu_display;
        // Mostra o nome da opção do estagio 1 do menu
      lcd.setCursor(0,0);
      lcd.print("                ");
      lcd.setCursor(0,0);
      switch(variavel_menu_display){
          case 0:{
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Velocidade");
            lcd.setCursor(0,1);
            lcd.print("backer DOWN cm/s");
            lcd.setCursor(11,1);
            lcd.print(velocidade_backer_decida,1);
          }
          break;
          case 1:{
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Velocidade");
            lcd.setCursor(0,1);
            lcd.print("backer UP   cm/s");
            lcd.setCursor(11,1);
            lcd.print(velocidade_backer_subida,1);
          }
          break;
          case 2:{
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Velocidade");
            lcd.setCursor(0,1);
            lcd.print("forno DOWN cm/s");
            lcd.setCursor(11,1);
            lcd.print(velocidade_forno_decida,1);
          }
          break;
          case 3:{
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Velocidade");
            lcd.setCursor(0,1);
            lcd.print("forno UP   cm/s");
            lcd.setCursor(11,1);
            lcd.print(velocidade_forno_subida,1);
          }
          break;
          
          case 4:{
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Tempo de ");
            lcd.setCursor(0,1);
            lcd.print("mergulho   min");
            lcd.setCursor(11,0);
            if(tempo_backer/60<10){
              lcd.print("0");
              lcd.print(tempo_backer/60);
            }
            else{
              lcd.print(tempo_backer/60);
            }
            lcd.print(":");
            if(tempo_backer%60<10){
              lcd.print("0");
              lcd.print(tempo_backer%60);
            }
            else{
              lcd.print(tempo_backer%60);
            }
          }
          break;
          case 5:{
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Tempo de ");
            lcd.setCursor(0,1);
            lcd.print("forno      min");
            lcd.setCursor(11,0);
            
            if(tempo_forno/60<10){
              lcd.print("0");
              lcd.print(tempo_forno/60);
            }
            else{
              lcd.print(tempo_forno/60);
            }
            lcd.print(":");
            if(tempo_forno%60<10){
              lcd.print("0");
              lcd.print(tempo_forno%60);
            }
            else{
              lcd.print(tempo_forno%60);
            }
          }
          break;
          case 6:{
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Tempo  ");
            lcd.setCursor(0,1);
            lcd.print("entre ciclo  min");
            lcd.setCursor(11,0);
            
             if(tempo_entre_ciclo/60<10){
              lcd.print("0");
              lcd.print(tempo_entre_ciclo/60);
            }
            else{
              lcd.print(tempo_entre_ciclo/60);
            }
            lcd.print(":");
            if(tempo_entre_ciclo%60<10){
              lcd.print("0");
              lcd.print(tempo_entre_ciclo%60);
            }
            else{
              lcd.print(tempo_entre_ciclo%60);
            }
          }
          break;
          case 7:{
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Altura de");
            lcd.setCursor(0,1);
            lcd.print("Mergulho   cm");
            lcd.setCursor(11,0);
            lcd.print(altura_backer,1);
          }
          break;
          case 8:{
            lcd.setCursor(0,0);
            lcd.print("Altura do");
            lcd.setCursor(0,1);
            lcd.print("forno      cm");
            lcd.setCursor(11,0);
            lcd.print(altura_forno,1);
          }
          break;
          case 9:{
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Temperatura");
            lcd.setCursor(0,1);
            lcd.print("do forno    °C");
            lcd.setCursor(11,0);
            lcd.print(temperatura_forno,1);
          }
          break;
      }
    }
    if(digitalRead(pino_botao_ok) == HIGH && botao_menu_stado==0){
      botao_menu_stado=1;
      tempo_menu=millis();
    }
    if(digitalRead(pino_botao_ok) == LOW && botao_menu_stado==1){  
      if(millis()-tempo_menu>=280){
        estagio_menu = 1;//2
        inf_print_menu=100;
        tempo_menu_max=millis();
        botao_menu_stado=0;
        variavel_menu_display =0;
        lcd.clear();
      }
    }
      
}


 
    


void software_Reset() // Restarts program from beginning but does not reset the peripherals and registers
{
asm volatile ("  jmp 0");  
}



  
