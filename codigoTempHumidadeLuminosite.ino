#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BMP280.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"

// Definições de sensores e display
#define LED 2
#define LIGHT_SENSOR_PIN 34
#define DHTPIN 4
#define DHTTYPE DHT22
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_BMP280 bmp;
DHT dht(DHTPIN, DHTTYPE);

// Credenciais de Wi-Fi e MQTT
const char* ssid = "aula";
const char* password = "12345678";
long timezone = -3;
byte daysavetime = 1;
const char* mqtt_server = "10.128.77.242";
const char* mqtt_topic = "sensores/dadosTrabalho2342345234523452345234523452345234";
const char* mqtt_topic_control = "sensores/ativacaoTrabalho2342345234523452345234523452345234";
// config para pegar hora
//  timeClient.update();
// timeClient.setTimeOffset(-10800);
//  Serial.println(timeClient.getFormattedTime());
//
//
// Inicialização WiFi e MQTT
WiFiClient espClient;
PubSubClient client(espClient);

//WiFiUDP ntpUDP;
//NTPClient timeClient(ntpUDP);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando-se a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  pinMode(LED,OUTPUT);
  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    Serial.println("Callback acionado!");
    Serial.print("Tópico: ");
    Serial.println(topic);
    Serial.print("Payload: ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    realizarAcao();

    Serial.println();
}
void reconnect() {
 
    while (!client.connected()) {
        Serial.print("Tentando conectar ao MQTT...");
        if (client.connect("ESP32Client")) {
            Serial.println("Conectado ao MQTT Broker!");
            // Inscreva-se no tópico de controle
            if (client.subscribe(mqtt_topic_control)) {
                Serial.println("Inscrito no tópico de controle: " + String(mqtt_topic_control));
            } else {
                Serial.println("Falha ao se inscrever no tópico de controle!");
            }
        } else {
            Serial.print("Falha na conexão, rc=");
            Serial.print(client.state());
            Serial.println(" tentando novamente em 5 segundos...");
            delay(5000);
        }
    }
  
}
void createDir(fs::FS &fs, const char *path) {
  Serial.printf("Creating Dir: %s\n", path);
  if (fs.mkdir(path)) {
    Serial.println("Dir created");
  } else {
    Serial.println("mkdir failed");
  }
}

void writeFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void realizarAcao() {
  Serial.println("Ação ativada! Executando...");
  delay(500);
  digitalWrite(LED,HIGH);
  
  delay(5000);
  digitalWrite(LED,LOW);
}

void pararAcao() {
  Serial.println("Ação desativada!");
 
}
char dirName[20];
char filePath[40];

void setup() {
  Serial.begin(9600);
  #ifdef REASSIGN_PINS
  SPI.begin(sck, miso, mosi, cs);
  if (!SD.begin(cs)) {
  #else
  if (!SD.begin()) {
  #endif
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }
  Serial.println(F("DHTxx test!"));
  dht.begin();
  
  //timeClient.begin();
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  Serial.println(F("BMP280 test"));
  unsigned status;
  //status = bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID);
  status = bmp.begin(0x76);
  if (!status) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    Serial.print("SensorID was: 0x"); Serial.println(bmp.sensorID(),16);
    Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    Serial.print("        ID of 0x60 represents a BME 280.\n");
    Serial.print("        ID of 0x61 represents a BME 680.\n");
    while (1) delay(10);
  }
 
  display.display();
  delay(2000); // Pause for 2 seconds
  display.clearDisplay();
   bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
  setup_wifi();
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
 

  client.setServer(mqtt_server, 1883);
  client.setCallback(mqtt_callback);
  reconnect();

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Contacting Time Server");
  configTime(3600 * timezone, daysavetime * 3600, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org");
  struct tm tmstruct;
  delay(2000);
  tmstruct.tm_year = 0;
  getLocalTime(&tmstruct, 5000);
  snprintf(dirName, sizeof(dirName), "/%04d-%02d-%02d_%02d-%02d", (tmstruct.tm_year) + 1900, (tmstruct.tm_mon) + 1, tmstruct.tm_mday, tmstruct.tm_hour, tmstruct.tm_min);
  Serial.printf(
    "\nNow is : %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct.tm_year) + 1900, (tmstruct.tm_mon) + 1, tmstruct.tm_mday, tmstruct.tm_hour, tmstruct.tm_min,
    tmstruct.tm_sec
  );
  snprintf(filePath, sizeof(filePath), "%s/dados.txt", dirName);

  createDir(SD, dirName);
  Serial.println("");
  SD.open(filePath, FILE_WRITE );

}

void loop() {
 // timeClient.update();
  client.loop(); 

  if (!client.connected()) {
    reconnect();
  }
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 // client.loop(); // Mantém a conexão MQTT viv
  //delay(10000);
  int analogValue = analogRead(LIGHT_SENSOR_PIN);
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);
  float p = bmp.readPressure() / 101325;
  float m = bmp.readAltitude(1013.25);

  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Falha ao ler do sensor DHT!"));
    return;
  }

  float hic = dht.computeHeatIndex(t, h, false);
  
  Serial.print("Luminosidade = ");
  Serial.println(analogValue);
  
  String luz = "";
  if (analogValue < 40) {
    luz = "dark";
  } else if (analogValue < 800) {
    luz = "noite";
  } else if (analogValue < 2000) {
    luz = "sombra";
  } else if (analogValue < 3200) {
    luz = "dia";
  } else {
    luz = "sol";
  }

  Serial.print(F("umidade: "));
  Serial.println(h);
  Serial.print(F("%  temperatura: "));
  Serial.println(t);
  Serial.print(F("°C  sensação térmica: "));
  Serial.print(hic);
  Serial.println(F("°C "));
  Serial.print(p);
  Serial.println(" atm");
  Serial.print(m);
  Serial.println(" m");

 
  String payload = String("{\"Luminosidade\": ") + analogValue +
                   ", \"Umidade\": " + h +
                   ", \"Temperatura\": " + t +
                   ", \"Pressão\": " + p +
                   ", \"altitude\": " + m +
                   ", \"SensacaoTermica\": " + hic + "}";
  
  if (client.publish(mqtt_topic, payload.c_str())) {
    Serial.println("Dados enviados com sucesso: " + payload);
  } else {
    Serial.println("Falha ao enviar dados MQTT");
    reconnect();
  }
  static unsigned long lastDisplaySwitch = 0;
  static bool showPageOne = true;
  
  if (millis() - lastDisplaySwitch > 5000) {
    showPageOne = !showPageOne;
    lastDisplaySwitch = millis();
  }

  if (showPageOne) {
    testscrolltext(luz, h, t, hic);
  } else {
    displayPageTwo(p, m);
  }

  configTime(3600 * timezone, daysavetime * 3600, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org");
  struct tm tmstruct;
  delay(2000);
  tmstruct.tm_year = 0;
  getLocalTime(&tmstruct, 5000);



  
  
  Serial.println("Começando intervalo de tempo de 3 minutos");
  delay(2000);
  Serial.println("terminando intervalo de tempo de 3 minutos");
}

void testscrolltext(String luz, float h, float t, float hic) {
 display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10, 0);
  
  display.print("L:");
  display.println(luz);
  display.print("H: ");
  display.print(h);
  display.println("%");
  display.print("T: ");
  display.print(t);
  display.println("C");
  display.print("ST:");
  display.print(hic);
  display.println("C");

  display.display();
}
void displayPageTwo(float p, float m) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10, 0);
  
  display.print("Pressao: ");
  display.print(p);
  display.println("atm");
  display.println("   Alt: ");
  display.print(m);
  display.println(" m");

  display.display();
}