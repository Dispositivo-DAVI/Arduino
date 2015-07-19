/*
|**********************|$ PROJETO DAVI - Dispositivo de Apoio a Deficiêntes Visuais $|*******************************|
|                                                                                                                    |
|********************************************************************************************************************|
|                                                                                                                    |
| @Software: DAVI                                                                                                    |
|                                                                                                                    |
|====================================================================================================================|
| @Version: 0.6                                                                                                      |
|====================================================================================================================|
| @Date: 31 de agosto de 2014                                                                                        | 
|====================================================================================================================|
| @Developer: Equipe DAVI.                                                                                           |
|====================================================================================================================|
| @Site: --=<| http://www.clubedosgeeks.com.br |>=--                                                                 |
|        --=<| http://www.growupti.com.br |>=--                                                                      |
|====================================================================================================================|
| @Licença Apache - Versão 2.0, janeiro de 2004                                                                      |
|        --=<| http://www.apache.org/licenses/ |>=--                                                                 |
|                                                                                                                    |
|====================================================================================================================|
|ATENÇÃO! Nunca use carregador externo ou conecte o dispositivo a porta USB, quando o mesmo estiver (Ligado), usando |
|a bateria externa..                                                                                                 | 
|====================================================================================================================|
*/

/*Importa as bibliotecas..*/
#include "ThreadController.h"
#include "Thread.h"
#include "NewPing.h"
#include "LiquidCrystal_I2C.h"
#include "Wire.h"

/*Define a distância(centimetro) máxima do sensor ultrasonico em que será realizado o ping..*/
#define ultraSonicHCSR04_MAX_DISTANCE 300 

#define ultraSonic_1_HCSR04_TRIGGER_PIN  8 /*Define o Trigger do sensor ultrasonico, configura em qual pino(porta digital) do Arduino acionar a onda..*/
#define ultraSonic_1_HCSR04_ECHO_PIN     9 /*Define o Echo do sensor ultrasonico, configura em qual pino(porta digital) do Arduino ecoa a onda..*/ 

#define ultraSonic_2_HCSR04_TRIGGER_PIN  6
#define ultraSonic_2_HCSR04_ECHO_PIN     7

#define ultraSonic_3_HCSR04_TRIGGER_PIN  4
#define ultraSonic_3_HCSR04_ECHO_PIN     5

#define ultraSonic_4_HCSRO4_TRIGGER_PIN  2
#define ultraSonic_4_HCSR04_ECHO_PIN     3

/*Cria objetos sonar do tipo NewPing que recebe como parametro as configurações dos pinos(portas digitais) e distancia máxima de alcanse.*/ 
NewPing sonar1(ultraSonic_1_HCSR04_TRIGGER_PIN, ultraSonic_1_HCSR04_ECHO_PIN, ultraSonicHCSR04_MAX_DISTANCE); 
NewPing sonar2(ultraSonic_2_HCSR04_TRIGGER_PIN, ultraSonic_2_HCSR04_ECHO_PIN, ultraSonicHCSR04_MAX_DISTANCE);
NewPing sonar3(ultraSonic_3_HCSR04_TRIGGER_PIN, ultraSonic_3_HCSR04_ECHO_PIN, ultraSonicHCSR04_MAX_DISTANCE);
NewPing sonar4(ultraSonic_4_HCSRO4_TRIGGER_PIN, ultraSonic_4_HCSR04_ECHO_PIN, ultraSonicHCSR04_MAX_DISTANCE);

/*Essas variáveis seram usadas para armazenar os valores que seram convertidos em centimetros.. */
unsigned int distanciaSensor1CM;
unsigned int distanciaSensor2CM;
unsigned int distanciaSensor3CM;
unsigned int distanciaSensor4CM;

/*Declara uma thread "raiz ou mãe" um controle que agrupa as thread's filhas..*/
ThreadController cpu;

/*Declara os objetos ultrasônico do tipo Thread's..*/
Thread threadUltraSonic1;
Thread threadUltraSonic2;
Thread threadUltraSonic3;
Thread threadUltraSonic4;

/*Declara thread LCD monitor e Serial monitor..*/
Thread threadMonitorLCD;
Thread threadMonitorSerial;

/*Declara thread*/
Thread threadSpeekStartSystem;
Thread threadSpeekNivelEndBateria;

/**/
Thread threadCont;

/*Declaração das constantes referentes aos pinos digitais que configura onde os motores de alerta vibratorio seram conectados na placa arduino.*/
const int sparkFun1 = 10;
const int sparkFun2 = 11;
const int sparkFun3 = 12;
const int sparkFun4 = 13;


/*Configura LCD - cria um objeto lcd e passa por parametro sua configuração setando, o endereço 0x27 
e definindo suas caracteristicas que contem 16 caracteres e 2 linhas no display do LCD..*/
LiquidCrystal_I2C lcd(0x27,16,2);

/*Define pino de saida(conexão para a speak)..*/
unsigned int speakPin = 8;

/*Variavel de controle para reprodoção do som da speak..*/
int contLoopSpeakStart = 0;

/**/
int contLoopSpeakEnd = 0;

/*------------------------------------------------------------------------------------------------------------------------------------------|
|=============================================================== ($ METODOS $) +============================================================|
|------------------------------------------------------------------------------------------------------------------------------------------*/

/*
|=============================================================================================|
| Método setup - É chamado apenas uma vez, no start da aplicação e realiza ou prepara as       |
| configurações iniciais do programa..                                                        |
|=============================================================================================|
*/
void setup() {
  Serial.begin(9600); /*Configura um serial monitor para exibir os resutados dos pings..*/ 

  /*Define pino(barramento) de saida(conexao) do speek..*/
  pinMode(speakPin, OUTPUT);

  /*Configura os pinos onde estaram conectados os atuadores de alerta vibratorio como saída.*/
  pinMode(sparkFun1,OUTPUT); 
  pinMode(sparkFun2,OUTPUT);
  pinMode(sparkFun3,OUTPUT);
  pinMode(sparkFun4,OUTPUT);

  /*Configura o Monitor LCD*/
  lcd.init(); /*Inicializa o LCD*/
  lcd.backlight(); /*Liga o LED do LCD*/

  /*Configura as Threads dos Objetos..*/
  threadUltraSonic1.setInterval(33); /*Define o tempo em que irá ser executada a thread..*/
  threadUltraSonic1.onRun(echoCheckUltraSonic1); /*Define qual função irá ser execultada pela thread..*/

  threadUltraSonic2.setInterval(33);
  threadUltraSonic2.onRun(echoCheckUltraSonic2);

  threadUltraSonic3.setInterval(33);
  threadUltraSonic3.onRun(echoCheckUltraSonic3);

  threadUltraSonic4.setInterval(33);
  threadUltraSonic4.onRun(echoCheckUltraSonic4);

  /*Configura a Thread do Monitor Serial*/
  threadMonitorSerial.setInterval(50);
  threadMonitorSerial.onRun(monitorSerial);

  /*Configura a Thread do Monitor LCD*/
  threadMonitorLCD.setInterval(50);
  threadMonitorLCD.onRun(monitorLCD);

  threadSpeekStartSystem.setInterval(0);
  threadSpeekStartSystem.onRun(speakStartSystem);

  threadSpeekNivelEndBateria.setInterval(10000);
  threadSpeekNivelEndBateria.onRun(speakNivelEndBateria);

  /*Adiciona as thread's filhas a thread raiz ou mãe..*/
  cpu.add(&threadSpeekStartSystem);
  cpu.add(&threadSpeekNivelEndBateria);

  cpu.add(&threadUltraSonic1);
  cpu.add(&threadUltraSonic2);
  cpu.add(&threadUltraSonic3);
  cpu.add(&threadUltraSonic4);

  cpu.add(&threadMonitorSerial);
  //cpu.add(&threadMonitorLCD);
  /*Obs.: & significa que vc declarou as threads como referência.. e não com ponteiro..*/
}

/*
|===============================================================================================|
| Método loop - Considerado como o coração do programa, estará sendo executado permanentemente  |
| enquanto o dispositivo estiver ligado (energizado)..                                          |
|===============================================================================================|
*/
void loop() {
  cpu.run(); /*Start a thread raiz..*/
}

/*
|================================================================================================|
| Método echoCheckUltraSonic - Verifica se ouve echo, ou seja, qual retorno do ping realizado    |
| pelo sensor ultrasônico, apos isso realiza a conversão dos dados obtidos para centimetro e     |
| chama o metodo fuzzySparkFun que controla os sensores atuadores de alerta vibratorio..         |
|================================================================================================|
*/
void echoCheckUltraSonic1(){
  /*Variável usada para receber o valor do ping(onda) em microssegundos..*/
  unsigned int distanciaSensorUS1 = sonar1.ping();/*Envia ping(onda) e obtem ping(onda) em microssegundos (uS)..*/
  distanciaSensor1CM = distanciaSensorUS1 / US_ROUNDTRIP_CM;/*converte o ping(onda) de microssegundos para centimetros..*/
  fuzzySparkFun1();/*chama o método que contem a logica fuzzy..*/
}

void echoCheckUltraSonic2(){
  unsigned int distanciaSensorUS2 = sonar2.ping();
  distanciaSensor2CM = distanciaSensorUS2 / US_ROUNDTRIP_CM;
  fuzzySparkFun2();
}

void echoCheckUltraSonic3(){
  unsigned int distanciaSensorUS3 = sonar3.ping();
  distanciaSensor3CM = distanciaSensorUS3 / US_ROUNDTRIP_CM;
  fuzzySparkFun3();
}

void echoCheckUltraSonic4(){
  unsigned int distanciaSensorUS4 = sonar4.ping();
  distanciaSensor4CM = distanciaSensorUS4 / US_ROUNDTRIP_CM;
  fuzzySparkFun4();
}

/*
|=============================================================================================|
| Método fuzzySparkFun - Possue toda a logica fuzzy usada para desliga ou liga o atuador de   |
| alerta vibratório corespondente ao sensor ultrasonico, adequando a intencidade de vibração  |
| de acordo com a distância encontrada pelo sensor..                                          |
|=============================================================================================|
*/
void fuzzySparkFun1(){
  digitalWrite(sparkFun1, LOW);

  /*Condição para ligar sparkFun..*/
  if (distanciaSensor1CM > 0 && distanciaSensor1CM <= 50) {
    digitalWrite(sparkFun1,HIGH);
  }
  if (distanciaSensor1CM > 50 && distanciaSensor1CM <= 100) {
    digitalWrite(sparkFun1,HIGH);
    delay(100);
    digitalWrite(sparkFun1, LOW);
  }
}

void fuzzySparkFun2(){
  digitalWrite(sparkFun2, LOW);

  /*Condição para ligar sparkFun..*/
  if (distanciaSensor2CM > 0 && distanciaSensor2CM <= 50) {
    digitalWrite(sparkFun2,HIGH);
  }
  if (distanciaSensor2CM > 50 && distanciaSensor2CM <= 100) {
    digitalWrite(sparkFun2,HIGH);
    delay(100);
    digitalWrite(sparkFun2, LOW);
  }
}

void fuzzySparkFun3(){
  digitalWrite(sparkFun3 ,LOW);

  /*Condição para ligar sparkFun..*/
  if (distanciaSensor3CM > 0 && distanciaSensor3CM <= 50) {
    digitalWrite(sparkFun3,HIGH);
  }
  if (distanciaSensor3CM > 50 && distanciaSensor3CM <= 100) {
    digitalWrite(sparkFun3,HIGH);
    delay(100);
    digitalWrite(sparkFun3, LOW);
  }
}

void fuzzySparkFun4(){
  digitalWrite(sparkFun4 ,LOW);

  /*Condição para ligar sparkFun..*/
  if (distanciaSensor4CM > 0 && distanciaSensor4CM <= 50) {
    digitalWrite(sparkFun4,HIGH);
  }
  if (distanciaSensor4CM > 50 && distanciaSensor4CM <= 100) {
    digitalWrite(sparkFun4,HIGH);
    delay(100);
    digitalWrite(sparkFun4, LOW);
  }
}

/*
|==============================================================================================|
| lerVcc - Ler voltagem                                                                        |
|==============================================================================================|
*/
long lerVcc() {
  long result;
  /*Leia referente a 1.1V AVcc*/
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); /*Espera por Vref para resolver..*/
  ADCSRA |= _BV(ADSC); /*Converte*/
  
  while (bit_is_set(ADCSRA,ADSC));
  
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; /*Volta a calcular AVcc em mV*/ 
  
  return result; /*O retorno será em millivolts. Ex.: 5000 = 5V, 3300 = 3.3V.*/
}

/*
|==============================================================================================|
| speakStartSystem - Emite Som(bip) para informar que o sistema foi iniciado, esse método é    |
| chamado por uma thread constantemente, porem so execultado apenas uma vez..                  |
|==============================================================================================|
*/
void speakStartSystem(){
  /*loop*/
  while(contLoopSpeakStart <= 1000){
    digitalWrite(speakPin, HIGH); /*Liga pino..*/
    analogWrite(speakPin, 255); /*Onda quadrada positiva..*/
    delayMicroseconds(192); /*192uS*/

    analogWrite(speakPin, 0); /*Onda quadrada neutra..*/
    delayMicroseconds(192); /*192uS*/
    digitalWrite(speakPin, LOW);/*Desliga pino..*/
    
    contLoopSpeakStart++;/*incremento*/
  }
}

/*
|=============================================================================================|
| Método speakNivelEndBateria - Verifica o valor da tensão de saida, e se estiver abaixo do   |
| permitido, exibe um alerta sonoro(LOOM) informando que deve recarregar a bateira..          |
| Deve-se obedece com caltela os niveis de tensão de acordo com a placa arduino e a bateria   |
| usada..                                                                                     |
|=============================================================================================|
*/
void speakNivelEndBateria(){
  if((lerVcc()) <= 4962){//4962 || 4982 || 5000
    /*loop*/
    while(contLoopSpeakEnd <= 1000){
      digitalWrite(speakPin, HIGH); /*Liga pino..*/
      analogWrite(speakPin, 255); /*Onda quadrada positiva..*/
      delayMicroseconds(192); /*192uS*/

      analogWrite(speakPin, 0); /*Onda quadrada neutra..*/
      delayMicroseconds(1920); /*1920uS*/
      digitalWrite(speakPin, LOW);/*Desliga pino..*/
      
      contLoopSpeakEnd++;/*incremento*/
    }
  }  
  contLoopSpeakEnd = 0;
}

/*
|=============================================================================================|
| Método monitorSerial - Imprime no monitor Serial os valores das distância encontradas       |
| pelos sensores ultrasonico..                                                                |
|=============================================================================================|
*/
void monitorSerial(){
  /*Exibe distancias dos Sensores..*/
  Serial.print("Ping1: ");
  Serial.print(distanciaSensor1CM);
  Serial.println("cm");

  Serial.print("Ping2: ");
  Serial.print(distanciaSensor2CM);
  Serial.println("cm");

  Serial.print("Ping3: ");
  Serial.print(distanciaSensor3CM);
  Serial.println("cm");

  Serial.print("Ping4: ");
  Serial.print(distanciaSensor4CM);
  Serial.println("cm");

  /*Exibe valores da voltagem da bateria..*/
  Serial.print( lerVcc(), DEC );
  Serial.println(" Vcc");
}

/*
|=============================================================================================|
| Método monitorLCD - Imprime no monitor LCD os valores das distância encontradas             |
| pelos sensores ultrasonico..                                                                | 
| OBS.: Quando esse méto é usado impacta no funcionamento(performace) do sistema atrazando as |
| leituras os alertas.. em fim todo o sistema, usar apenas para verifica leituras..           |
|=============================================================================================|
*/
void monitorLCD(){
  /*Exibe Distâncias dos sensores..*/
  lcd.setCursor(0,0); /*Define que escreva na linha 1 do lcd..*/
  lcd.print("Sensor 1: ");
  lcd.setCursor(0,1); /*Define que escreva na linha 2 do lcd..*/
  lcd.print(distanciaSensor1CM);
  lcd.print(" cm");
  delay(500);

  lcd.setCursor(0,0);
  lcd.print("Sensor 2: ");
  lcd.setCursor(0,1);
  lcd.print(distanciaSensor2CM);
  lcd.print(" cm");
  delay(500);

  lcd.setCursor(0,0);
  lcd.print("Sensor 3: ");
  lcd.setCursor(0,1);
  lcd.print(distanciaSensor3CM);
  lcd.print(" cm");
  delay(500);

  lcd.setCursor(0,0);
  lcd.print("Sensor 4: ");
  lcd.setCursor(0,1);
  lcd.print(distanciaSensor4CM);
  lcd.print(" cm");
  delay(500);

  /*Exibe valor da Voltagem..*/
  lcd.setCursor(0,0); /*Define que escreva na linha 1 do lcd..*/
  lcd.print("Tensao :");
  lcd.setCursor(0,1); /*Define que escreva na linha 2 do lcd..*/
  lcd.print(lerVcc(), DEC);
  lcd.print(" Vcc");
  delay(500);
}

