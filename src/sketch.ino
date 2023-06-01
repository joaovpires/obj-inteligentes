//Incluir bibliotecas
#include <EspMQTTClient.h>

// CONFIG REDE WI-FI
char SSIDName[] = "Wokwi-GUEST"; 
char SSIDPass[] = "";

// DEFINIÇÕES DE PINOS
#define pinEcho 18
#define pinTrig 4
#define pinLED 13

// CONEXÃO COM O BROKER MQTT
char BrokerURL[] = "broker.hivemq.com"; 
char BrokerUserName[] = ""; 
char BrokerPassword[] = ""; 
char MQTTClientName[] = "mqtt-mack-pub";
int BrokerPort = 1883; //porta do broker MQTT

// TOPICO DO BROKER A SER CONECTADO
String TopicoPrefixo = "TESTMACK32151608"; 
String Topico_01 = TopicoPrefixo+"/Sonar";

// INICIALIZA O CLIENTE MQTT
EspMQTTClient clienteMQTT(SSIDName, SSIDPass, BrokerURL, BrokerUserName, BrokerPassword, MQTTClientName, BrokerPort);

// DECLARAÇÃO DE FUNÇÕES
void enviaPulso();
void medeDistancia();

// DECLARAÇÃO DE VARIÁVEIS PARA O SENSOR
volatile unsigned long inicioPulso = 0;
volatile float distancia = 0;
volatile int modo = -1;

void onConnectionEstablished() {
}

// IMPLEMENTO DE FUNÇÕES
void medeDistancia(){
  switch (modo) {
    case 0: {
        inicioPulso = micros();
        modo = 1;
        break;
      }
    case 1: {
        distancia = (float)(micros() - inicioPulso) / 58.3; // distancia em CM
        inicioPulso = 0;
        modo = -1;
        break;
      }
  }
}

void enviaPulso(){
  // ENVIA O SINAL PARA O MÓDULO INICIAR O FUNCIONAMENTO
  digitalWrite(pinTrig, HIGH);
  // AGUARDAR 10 uS PARA GARANTIR QUE O MÓDULO VAI INICIAR O ENVIO
  delayMicroseconds(10);
  // DESLIGA A PORTA PARA FICAR PRONTO PARA PROXIMA MEDIÇÃO
  digitalWrite(pinTrig, LOW);
  // INDICA O MODO DE FUNCIONAMENTO (AGUARDAR PULSO)
  modo = 0;
}

// SETUP
void setup() {
  Serial.begin(9600);

  pinMode(pinEcho, INPUT);
  pinMode(pinTrig, OUTPUT);
  digitalWrite(pinTrig, LOW);
  pinMode(pinLED, OUTPUT); // Configura o pino do LED como saída

  // CONFIGURA A INTERRUPÇÃO
  attachInterrupt(digitalPinToInterrupt(pinEcho), medeDistancia, CHANGE);

  clienteMQTT.enableDebuggingMessages(); // HABILITA AS MENSAGENS NO DEBUG DO CONSOLE
}

// LOOP
void loop() {
  if (clienteMQTT.isWifiConnected() == 1) {
    Serial.println("Conectado ao WiFi!");
  } else {
    Serial.println("Nao conectado ao WiFi!");
  }

  if (clienteMQTT.isMqttConnected() == 1) {
    Serial.println("Conectado ao broker MQTT!");
  } else {
    Serial.println("Nao conectado ao broker MQTT!");
  }
  clienteMQTT.loop(); //funcao necessaria para manter a conexao com o broker MQTT ativa e coletar as mensagens recebidas

  Serial.println("Nome do cliente: " + String(clienteMQTT.getMqttClientName())
    + " / Broker MQTT: " + String(clienteMQTT.getMqttServerIp())
    + " / Porta: " + String(clienteMQTT.getMqttServerPort())
  );

  // ENVIA O COMANDO PARA O MÓDULO LER A DISTANCIA
  enviaPulso();
  // A RESPOSTA DA DISTANCIA VEM POR INTERRUPÇÃO, SÓ PRECISA ESPERAR ALGUNS MILISSEGUNDOS
  
  delay(2000); // TEMPO DE RESPOSTA APÓS A LEITURA

  if (distancia < 100 && distancia > 3) { // INTERVALO ESCOLHIDO DE DISTÂNCIA PARA DETECÇÃO
    digitalWrite(pinLED, HIGH); // Ligar o LED
    Serial.print("Entrou no raio de distancia! ");
    clienteMQTT.publish(Topico_01, "Entrou no raio de distância! Valor detectado: " + String(distancia) + " cm");
  } else {
    digitalWrite(pinLED, LOW); // Desligar o LED
  }
}
