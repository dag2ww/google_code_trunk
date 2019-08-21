#include <M5Stack.h> // Biblioteca do M5Stack
#include <HardwareSerial.h> //Biblioteca para comunicação Serial
#define TINY_GSM_MODEM_SIM800 // Definição do tipo de modem usado (SIM800L)
#include <TinyGsmClient.h> // Biblioteca GSM

// Variáveis do display M5Stack
#define WDT_SCR            320     // Tamanho
#define HGT_SCR            240     // Altura
#define WDT_SCR2           160     // Metade do tamanho
#define HGT_SCR2           120     // Metade da altura

// Celulares com permissões para enviar SMS
const int sizeListNumbers = 1;
const String numbers[sizeListNumbers] = {"+48693157686"};

// Objeto de comunicação serial do SIM800L
HardwareSerial SerialGSM(2);

// Velocidade da serial do SIM800L (A velocidade do monitor serial é de 115200)
const int BAUD_RATE = 9600;

// Pinos dos relés
const int relayPin1 = 21, relayPin2 = 22, relayPin3 = 19;

// Objeto com as funções GSM
TinyGsm modemGSM(SerialGSM);

// Altura da fonte (FreeSerif24pt7b) 24 + 4 de espaçamento
const int fontHeigth = 28;

// Assinatura da função "M5StackButtonsRead" (para evitar erros de compilação)
void M5StackButtonsRead(void *);

// Rodaremos duas tasks, onde ambas utilizam os pinos dos relés
// Essa flag indica se os pinos dos relés estão ocupados e evita que duas operações sejam feitas ao mesmo tempo
bool pinsBusy = false;
// Variáveis de referência contagem de tempo (millis), usados na nossa função de timeout
long prevMillisStartScreen, prevMillisReadSerial;
// Flags que indicam o início de contagem de tempo (millis), usados na nossa função de timeout
bool flagStartScreen = false, flagReadSerial = false;


// Função que verifica se o pino do relé passado por parâmetro está ocupado
// E evita que duas operações (uma de cada task) sejam feitas ao mesmo tempo
void pinsControl(int pin, int val)
{
  // Enquanto a flag estiver "true", não faz nada
  while(pinsBusy);

  // Seta a flag como true
  pinsBusy = true;
  // Executa o digitalWrite
  digitalWrite(pin, val);
  // Logo em seguida seta a flag como false
  pinsBusy = false;
}

// Exibe as labels "Relay 1", "Relay 2" e "Relay 3"
void showLabelButtons()
{
  String msg = "Relay 1";
  int spacing = 5;
  M5.Lcd.setFreeFont(&FreeSerif12pt7b);
  M5.Lcd.setCursor((WDT_SCR2/2 ) - (msg.length()*12 / 2) - spacing,HGT_SCR-20);       
  M5.Lcd.print(msg);

  msg = "Relay 2";
  M5.Lcd.setCursor(WDT_SCR2 - (msg.length()*12 / 2) + spacing*2, HGT_SCR-20);
  M5.Lcd.print(msg);

  msg = "Relay 3";
  M5.Lcd.setCursor(WDT_SCR2 + (WDT_SCR2/3) + spacing, HGT_SCR-20);
  M5.Lcd.print(msg);  
}

// Exibe no display a String "msg" e limpa o display caso o bool "clear" seja "true"
void showDisplay(String msg, bool clear)
{
  // Se a flag "clear" estiver como "true", resetamos o display
  if(clear)
  {
    M5.Lcd.clear();
    showLabelButtons();
    M5.Lcd.setFreeFont(&FreeSerif18pt7b);
    M5.Lcd.setCursor(0,fontHeigth);
  }
  
  // Exibimos a mensagem no display
  M5.Lcd.printf(msg.c_str());
  // Atualizamos o objeto M5 referente ao M5Stack
  M5.update();
}

// Inicia o monitor serial e a serial do modem GSM
void serialInit()
{
  Serial.begin(115200);
  SerialGSM.begin(BAUD_RATE);
  TinyGsmAutoBaud(SerialGSM);
}

// Configura o display com as cores de fundo, de texto e a fonte do texto
void displayInit()
{
  // Seta a tela de fundo como preta
  M5.Lcd.fillScreen(BLACK);
  // Seta a cor do texto como branca
  M5.Lcd.setTextColor(WHITE);
  // Seta a fonte do texto
  M5.Lcd.setFreeFont(&FreeSerif24pt7b);
}

// Reinicia o modem, exibindo no monitor serial e no display o sucesso ou falha
bool gsmRestart(bool restart)
{    
  // Exibições display e monitor serial
  showDisplay("Restarting modem...\n", true);
  Serial.println("Restarting modem...");

  // Se foi possível reiniciar o modem
  if(modemGSM.restart())
  {
    // Exibições display e monitor serial (sucesso)
    M5.Lcd.setCursor(0,0);
    showDisplay("Modem ", true);
    M5.Lcd.setTextColor(GREEN);
    showDisplay("OK\n",false);
    M5.Lcd.setTextColor(WHITE);
    Serial.println("Modem ok");
    return true;
  }
  
  // Exibições display e monitor serial (falha)
  showDisplay("Modem ", true);
  M5.Lcd.setTextColor(RED);
  showDisplay("Failed\n", false);
  M5.Lcd.setTextColor(WHITE);
  Serial.println("Modem failed"); 

  // Aguardamos 5 segundos e reiniciamos o ESP   
  if(restart)
  {
    delay(5000);      
    ESP.restart();
  }
  
  return false;  
}

// Aguarda que a mensagem "SMS Ready" seja recebida do modem em até 10 segundos, exibindo no monitor serial e no display o sucesso ou falha
bool waitSMSCallReady(bool setup)
{  
  // Exibições display e monitor serial
  Serial.println("Waiting SMS/Call");
  showDisplay("Waiting SMS/Call ", false);

  String msg = "";
  long prevMillis = millis();

  // Em até 10 segundos aguardamos a mensagem "SMS Ready"
  while(prevMillis + 10000 > millis() && msg.indexOf("SMS Ready")<0)
  {
    if(SerialGSM.available())
      msg = SerialGSM.readString();
  }
  
  // Se a mensagem foi recebida
  if(msg.indexOf("SMS Ready")>=0)
  {
    // Exibições display e monitor serial (sucesso)
    Serial.println(msg);
    M5.Lcd.setTextColor(GREEN);
    showDisplay("OK\n", false);
    M5.Lcd.setTextColor(WHITE);

    return true;
  }
  // Se a mensagem não foi recebida
  // Exibições display e monitor serial (falha)
  Serial.println("SMS/Call init failed");
  M5.Lcd.setTextColor(RED);
  showDisplay("Failed\n", false);
  M5.Lcd.setTextColor(WHITE); 

  // Se estamos ainda no setup, aguardamos 5 segundos e reiniciamos o ESP
  if(setup)
  {
    delay(3000);    
    ESP.restart();
  }  
  
  return false;
}

// Configuramos a mensagem SMS para modo texto no modem GSM
bool setSMSTextMode(bool restart)
{  
  String resp = "";

  // Enviamos um comando AT
  modemGSM.sendAT("+CMGF=1\r\n");
  // Aguardamos uma responsta em até 3 segundos
  modemGSM.waitResponse(3000, resp);
  
  // Exibição display
  showDisplay("SMS Text mode ", false);
  
  // Se a resposta conter um "OK"
  if(resp.indexOf("OK")>=0)
  {
    // Exibimos no display (sucesso)
    M5.Lcd.setTextColor(GREEN);
    showDisplay("OK\n", false);
    M5.Lcd.setTextColor(WHITE);
    Serial.println("SMS Text mode ok");

    return true;
  }
  // Se a resposta não conter um "OK"
  // Exibimos no display (falha)
  M5.Lcd.setTextColor(RED);
  showDisplay("Fail\n", false);
  M5.Lcd.setTextColor(WHITE);
  Serial.println("SMS Text mode failed");

  // Se a flag "restart" estiver como "true", aguardamos 5 segundos e reiniciamos o ESP
  if(restart)
  {
    delay(5000);        
    ESP.restart();
  }
  
  return false;
}

// Função que deleta todos os SMS
bool deleteALLSMS()
{  
  String resp = "";

  // Enviamos um comando AT 
  modemGSM.sendAT("+CMGD=1,4\r\n"); 
  // Aguardamos uma resposta em até 3 segundos
  modemGSM.waitResponse(3000, resp);
  
  // Se a resposta conter um "OK" retornamos "true", se não retornamos "false"
  return resp.indexOf("OK")>=0;
}

bool waitNetwork(bool restartESP)
{  
  // Exibições display e monitor serial
  Serial.println("Waiting network");
  showDisplay("Waiting network ", false);
  
  if(modemGSM.waitForNetwork())
  {
    // Exibições display e monitor serial (sucesso)
    Serial.println("OK");
    M5.Lcd.setTextColor(GREEN);
    showDisplay("OK\n", false);
    M5.Lcd.setTextColor(WHITE);

    return true;
  }
  
  // Exibições display e monitor serial (falha)
  Serial.println("Failed");
  M5.Lcd.setTextColor(RED);
  showDisplay("Fail\n", false);
  M5.Lcd.setTextColor(WHITE); 

  // Se estamos ainda no setup, aguardamos 5 segundos e reiniciamos o ESP
  if(setup)
  {
    delay(3000);    
    ESP.restart();
  }  
  
  return false;  
}

// Reiniciamos o modem
void modemRestart(bool restartESP)
{  
  // Reiniciamos o modem
  if(!gsmRestart(restartESP))  
    return;

  // Aguardamos a mensagem SMS Ready e Call Ready enviada pelo SIM800L
  if(!waitSMSCallReady(restartESP))
    return; 

  // Conectamos na rede gsm
  if(!waitNetwork(restartESP))
    return;   
  
  // Setamos SMS para modo texto
  if(!setSMSTextMode(restartESP))
    return;
  
  // É recomendável aguardar um tempo (aprox. 10s) antes de entrar no loop
  // Isso evita erros de sincronização na serial do GSM
  delay(10000);
}

// Iniciamos os pinos e a task que ficará varrendo os botões do M5Stack
// Os botões do M5Stack não precisam ser setados no pinMode
void inputOutputInit()
{
  pinMode(relayPin1, OUTPUT);
  pinMode(relayPin2, OUTPUT);
  pinMode(relayPin3, OUTPUT);
  // Digital write com flag (Os relés possuem lógica inversa)
  pinsControl(relayPin1, HIGH);
  pinsControl(relayPin2, HIGH);
  pinsControl(relayPin3, HIGH);
  
  // Iniciamos uma task que irá ler os botões
  xTaskCreatePinnedToCore(
  M5StackButtonsRead,   //Função que será executada
  "M5StackButtonsRead", //Nome da tarefa
  10000,     //Tamanho da pilha
  NULL,      //Parâmetro da tarefa (no caso não usamos)
  2,         //Prioridade da tarefa
  NULL,      //Caso queira manter uma referência para a tarefa que vai ser criada (no caso não precisamos)
  0);        //Número do core que será executada a tarefa (usamos o core 0 para o loop ficar livre com o core 1)
  
}

// Exibimos a mensagem "Waiting for messages" com uma linha grifando o texto
void showStartScreen()
{  
  showDisplay("", true);
  M5.Lcd.fillRect(10, HGT_SCR2, WDT_SCR-20, 1, TFT_WHITE);
  M5.Lcd.setCursor(5, HGT_SCR2-10);
  showDisplay("Waiting for messages", false);
}

void setup() 
{  
  // Iniciamos o Objeto, desabilitando o SD, que não será usado
  M5.begin(true,false,true); //LCD, SD, Serial  

  // Exibimos os textos "Relay 1", "Relay 2" e "Relay 3" 
  // Referentes aos botões do M5Stack
  showLabelButtons();
  // Setamos os relés (output) e criamos uma task de leitura dos botões
  inputOutputInit();
  // Iniciamos o monitor serial e modemGSM com suas respectivas velocidades
  serialInit();
  // Iniciamos o display  
  displayInit();
  // Reiniciamos o modem, informando que se for encontrado algum problema deve-se reiniciar o ESP
  modemRestart(true);
  // Exibe a tela inicial com a mensagem "Waiting for messages"
  showStartScreen();    

  // Setamos as flags de contagem de tempo como true
  flagStartScreen = flagReadSerial = true;
}


// Task que lê o botão a cada 10ms e controla os relés
void M5StackButtonsRead(void *p) 
{
  TickType_t taskDelay = 10 / portTICK_PERIOD_MS;

  //IMPORTANTE: SEMPRE DEIXAR UM LOOP COM UM DELAY PARA ALIMENTAR WATCHDOG
  while(true)
  {    
    // Se o primeiro botão "Relay 1" foi pressionado, modamos o estado do relé 1
    if(M5.BtnA.wasPressed())      
      pinsControl(relayPin1, !digitalRead(relayPin1));

    // Se o segundo botão "Relay 2" foi pressionado, modamos o estado do relé 2
    if(M5.BtnB.wasPressed())
      pinsControl(relayPin2, !digitalRead(relayPin2));
    
    // Se o terceiro botão "Relay 3" foi pressionado, modamos o estado do relé 3
    if(M5.BtnC.wasPressed())      
      pinsControl(relayPin3, !digitalRead(relayPin3));
    
    // Executamos um delay de 10ms, os delays executado nas xTasks são diferentes
    vTaskDelay(taskDelay);
    // Atualizamos o objeto M5 referente ao M5Stack
    M5.update();

    //IMPORTANTE: SEMPRE DEIXAR UM DELAY PARA ALIMENTAR WATCHDOG
  }  
}

// Verifica se o número é algum dos declarados pelo programa
bool numberIsValid(String number)
{
  // Percorre vetor com os números e verifica se é a String é igual
  for(int i = 0; i < sizeListNumbers; i++)
    if(numbers[i].equals(number))
      return true;

  return false;
}

// Executa uma ação de acordo com a variavel 'msg'
void newSMS(String number, String msg)
{    
  // "pinsControl" é uma função que é usada pela task core 0 (botão) e também pela task core 1 (loop) e evita que as duas acessem os pinos ao mesmo tempo
  
  if(msg.equalsIgnoreCase("relay 1 on"))
  {
    // Os reles possuem lógica inversa
    pinsControl(relayPin1, LOW);   
  }
  else
  if(msg.equalsIgnoreCase("relay 1 off")) 
  {
    pinsControl(relayPin1, HIGH);
  }
  else
  if(msg.equalsIgnoreCase("relay 2 on"))
  {
    pinsControl(relayPin2, LOW); 
  }
  else
  if(msg.equalsIgnoreCase("relay 2 off")) 
  {
    pinsControl(relayPin2, HIGH);
  }
  else
  if(msg.equalsIgnoreCase("relay 3 on"))
  {
    pinsControl(relayPin3, LOW); 
  }
  else
  if(msg.equalsIgnoreCase("relay 3 off")) 
  {
    pinsControl(relayPin3, HIGH);
  }
  else
  if(msg.equalsIgnoreCase("relays off"))
  {
    pinsControl(relayPin1, HIGH);
    pinsControl(relayPin2, HIGH);
    pinsControl(relayPin3, HIGH);
  }
  else
  if(msg.equalsIgnoreCase("relays on"))
  {
    pinsControl(relayPin1, LOW);
    pinsControl(relayPin2, LOW);
    pinsControl(relayPin3, LOW);
  }
  else // Faz com que o modem efetue uma ligação para o número que enviou o SMS
  if(msg.equalsIgnoreCase("call me"))
  {
    showDisplay("Calling...\n", true);
    if(modemGSM.callNumber(number))
    {
      showDisplay("OK\n", false);
      Serial.println("OK\n");
    }
    else
    {
      showDisplay("Failed\n", false);    
      Serial.println("Failed\n");
    }
    modemGSM.callHangup();
  }
  else // Se recebermos um SMS "status", o modem enviará uma resposta SMS com os status dos relés
  if(msg.equalsIgnoreCase("status"))
  {
    String status = "Relay 1: ";

    // Obtém os status do relé 1
    if(digitalRead(relayPin1) == LOW)
      status+="ON\n";
    else
      status+="OFF\n";

    status += "Relay 2: ";

    // Obtém os status do relé 2
    if(digitalRead(relayPin2) == LOW)
      status+="ON\n";
    else
      status+="OFF\n";

    status += "Relay 3: ";

    // Obtém os status do relé 3
    if(digitalRead(relayPin3) == LOW)
      status+="ON";
    else
      status+="OFF";

    // Exibições display e monitor serial
    showDisplay("Sending SMS...\n",true);
    Serial.println("Sending SMS...");
    
    // Envia SMS
    if(modemGSM.sendSMS(number,status))
    {
      showDisplay("OK", false);
      Serial.println("OK");
    }
    else
    {
      showDisplay("Failed", false);
      Serial.println("Failed");
    }       
  }
  else // Se recebermos um SMS "delete all", o modem excluirá todos os SMS (lidos e não lidos) e enviará uma resposta SMS se a exclusão foi ou não possível
  if(msg.equalsIgnoreCase("delete all"))
  {
    String respMsg;

    // Deleta todos os SMS
    if(deleteALLSMS())
      respMsg = "The messages have been deleted";
    else
      respMsg = "Failed to delete messages";
    
    // Exibições display e monitor serial
    showDisplay("Sending SMS...\n",true);
    Serial.println("Sending SMS...");

    // Envia SMS
    if(modemGSM.sendSMS(number, respMsg))
    {
      showDisplay("OK", false);
      Serial.println("OK");
    }
    else
    {
      showDisplay("Failed", false);
      Serial.println("Failed");
    }    
  }
}

// Enviamos um comando AT pedindo que o modem nos retorne uma lista de SMS ainda não lidos
String listSMSUnRead()
{
  String msg = "";  

  // Enviamos um comando AT
  modemGSM.sendAT("+CMGL=\"REC UNREAD\"\r\n"); 

  // Aguardamos uma resposta em até 5 segundos
  modemGSM.waitResponse(5000, msg);

  // Se não recebemos a resposta, aborta função
  if(msg.equals("")) 
    return "";

  //Serial.println("Response:["+msg+"]");
  // Se a resposta recebida for uma mensagem de Erro
  //Exemplo de mensagem:
  /*
  AT+CMGL="REC UNREAD"
  ERROR
  */
  if(msg.indexOf("AT+CMGL=\"REC UNREAD\"")>=0)
  {
    // Exibimos na serial
    Serial.println(msg);

    // Reiniciamos o modem
    // false = não reiniciará o ESP caso ocorra uma falha
    modemRestart(false);
    // Retornamos vazio
    return "";
  }

  // Se a mensagem possuir "OK", "SMS Ready" ou "Call Ready", abortamos a função pois não é o SMS em si
  if((msg.indexOf("OK")<=4 && msg.indexOf("OK")>=0) || (msg.indexOf("Call Ready")<=4 && msg.indexOf("Call Ready")>=0) || (msg.indexOf("SMS Ready")<=4 && msg.indexOf("SMS Ready")>=0))
    return "";

  // Exibimos a mensagem na serial
  Serial.println(msg);  

  // Retornamos a String msg
  return msg;
}

// A função "splitMsg" percorre a String "fullMsg" com as mensagens não lidas (Exemplo abaixo) e obtém os respectivos nºs e texto das mensagens
/*+CMGL: 1,"REC READ","+5518996478356","","18/12/07,08:57:56-08"
Relay 1 on

+CMGL: 2,"REC UNREAD","+5518996478356","","18/12/07,08:57:59-08"
Relay 2 on

OK*/
void splitMsg(String fullMsg)
{  
  String msg, number, aux;

  // Exibe na serial (debug)
  Serial.println("Full msg:["+fullMsg+"]"); 

  // Para retirarmos os \n do início do buffer, executamos este primeiro while
  // Enquanto o início da mensagem não for "+CMGL", pulamos uma linha
  while(!fullMsg.startsWith("+CMGL") && fullMsg.indexOf("\n")>=0)
    fullMsg = fullMsg.substring(fullMsg.indexOf("\n")+1);

  // Enquanto o início da mensagem não for "+CMGL", significa que é uma mensagem válida
  while(fullMsg.startsWith("+CMGL"))
  {
    // Obtém a pos da segunda vírgula
    int pos2ndComma = fullMsg.indexOf(",",fullMsg.indexOf(",")+1);    

    // Obtém substring após a segunda vírgula
    aux = fullMsg.substring(pos2ndComma+1);  
    // Obtém número que inicia após a primeira aspas e termina antes da segunda aspas
    number = aux.substring(1, aux.indexOf(",")-1);

    // Exibe o número de celular que nos enviou o SMS no monitor serial entre colchetes
    Serial.println("Number:["+number+"]");

    // Pula a primeira linha        
    fullMsg = aux.substring(aux.indexOf("\n")+1);
    // Obtém substring até o primeiro \n
    msg = fullMsg.substring(0, fullMsg.indexOf("\n")-1);

    // Exibe o texto da mensagem no monitor serial entre colchetes
    Serial.println("Msg:["+msg+"]");
    
    // Se o número for válido
    if(numberIsValid(number))
    {
      // Exibimos no display que um novo SMS chegou
      showDisplay("A new SMS Arrived!\n", true);
      showDisplay("\""+msg+"\"", false);
      // Chamamos a função "newSMS" que executará o comando de acordo com o texto guardado em "msg"
      newSMS(number, msg);
    }
    else
    {
      // Exibimos no display que o nº é inválido
      showDisplay("New SMS...\nInvalid number\n", true);
      Serial.println("New SMS...\nInvalid number");
    }

    // Pulamos duas linhas para obter a próxima mensagem
    fullMsg = fullMsg.substring(fullMsg.indexOf("\n", fullMsg.indexOf("\n")+1)+1);    
  }  
}

// Função que compara se o tempo foi atingido, sem que 'congele' a execução do loop
bool timeout(const int DELAY, long *previousMillis, bool *flag)
{
  if(*flag)
  {
    *previousMillis = millis();
    *flag = false;
  }

  if((*previousMillis + DELAY) < millis())
  {
    *flag = true;
    return true;
  }

  return false;
}

// Verificamos se existem mensagens não lidas
void verifySMSMessages()
{
  // Recebemos todas as mensagens não lidas no buffer "msg"
  String msg = listSMSUnRead();
      
  // Se existe pelo menos uma mensagem
  if(!msg.equals(""))
  {    
    // Chamamos a função que separa as mensagems e executa uma ação de acordo com o texto de cada mensagem
    splitMsg(msg);    
    // Setamos a variável de referencia de contagem de tempo com o tempo atual (millis) para que ela seja mostrada só após o timeout (5 segundos)
    prevMillisStartScreen = millis();
  }   
}

// OBS: Para atualizar os componentes do M5Stack é necessário chamar a função M5.update();
// Essa função já é chamada na task (core 0) 'M5StackButtonsRead', então não é necessário chamá-la novamente no loop
void loop()
{  
  // A cada 300ms verificamos se existem novos SMS
  if(modemGSM.getSimStatus() == SIM_READY && timeout(300, &prevMillisReadSerial, &flagReadSerial))
  {    
    // Printamos um ponto (debug) indicando a frequencia em que a função "verifySMSMessages" é chamada
    Serial.print(".");
    verifySMSMessages();   
  }

  // A cada 5 segundos atualizamos o display com a tela inicial
  if(timeout(5000, &prevMillisStartScreen, &flagStartScreen))
  {
    showStartScreen();   

    // Se o status do SIM não for "Ready" (pronto), reiniciamos o modem
    if(modemGSM.getSimStatus() != SIM_READY)
      modemRestart(false);
  }  
}
