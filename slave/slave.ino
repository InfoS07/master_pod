#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "ESP32-Access-Point";
const char* password = "12345678";
const int BUTTON_PIN = 5;
const unsigned int udpPort = 1234;  // Port utilisé pour les communications UDP
const IPAddress multicast_ip(239, 0, 0, 1);  // Adresse IP de multicast de l'ESP32
int led = 4;

WiFiUDP udp;

void setup() {
    pinMode(led, OUTPUT);
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    Serial.print("Connecting to ");
    Serial.print(ssid);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    // Rejoindre le groupe multicast
    if (!udp.beginMulticast(WiFi.localIP(), multicast_ip, udpPort)) {
        Serial.println("Failed to join multicast group");
        while (1) {
            delay(1000);
        }
    }
}

void loop() {
    int packetSize = udp.parsePacket();
    if (packetSize) {
        char incomingPacket[255];
        int len = udp.read(incomingPacket, 255);
        if (len > 0) {
            incomingPacket[len] = 0;
            Serial.printf("Received multicast message: %s\n", incomingPacket);
            if(String(incomingPacket) == "1"){
                digitalWrite(led, HIGH);
            }
            if(String(incomingPacket) == "a"){
                digitalWrite(led, LOW);
            }
            
        }
    }

    // Ajoutez ici votre logique existante pour envoyer des messages par UDP
    int buttonState = digitalRead(BUTTON_PIN);
    if (buttonState == 0) {
        udp.beginPacketMulticast(multicast_ip, udpPort, WiFi.localIP());
        udp.write("z");
        udp.endPacket();
        Serial.print("Sent multicast message: ");
        Serial.println("Sent multicast message: y");

        //digitalWrite(led, LOW);

        delay(400);
    }
}
