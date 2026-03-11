#include <TFT_eSPI.h>
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();

// --- CONFIGURATION TROUVÉE ---
#define TX_PIN 8 // GPIO8 - TX vers ELM327
#define RX_PIN 9 // GPIO9 - RX depuis ELM327
#define OBD_BAUD 38400
#define OBD_CONFIG SERIAL_8N1

// --- COULEURS ---
#define BACKGROUND 0x0000
#define WHITE 0xFFFF
#define RED 0xF800
#define GREEN 0x07E0
#define YELLOW 0xFFE0
#define BLUE 0x001F
#define CYAN 0x07FF

void setup() {
 Serial.begin(115200);
 delay(1000);
 
 tft.init();
 tft.setRotation(0);
 tft.fillScreen(BACKGROUND);
 
 Serial.println("\n=== CONFIGURATION OBD TROUVÉE ===");
 Serial.println("Baud: 38400 8N1");
 Serial.println("TX: GPIO8, RX: GPIO9");
 
 // Afficher configuration
 tft.setTextColor(GREEN);
 tft.setTextSize(3);
 tft.setCursor(20, 20);
 tft.print("OBD OK!");
 
 tft.setTextSize(2);
 tft.setCursor(20, 60);
 tft.print("38400 8N1");
 
 tft.setTextSize(1);
 tft.setTextColor(WHITE);
 tft.setCursor(20, 90);
 tft.print("TX: GPIO8 -> OBD RX");
 tft.setCursor(20, 110);
 tft.print("RX: GPIO9 <- OBD TX");
 
 delay(2000);
 
 // Initialiser Serial1
 Serial1.begin(OBD_BAUD, OBD_CONFIG, RX_PIN, TX_PIN);
 delay(100);
 
 // Nettoyer buffer
 while(Serial1.available()) {
 Serial1.read();
 }
 
 // Tester la connexion
 testConnection();
}

void testConnection() {
 tft.fillRect(0, 130, 240, 110, BACKGROUND);
 
 // Test 1: ATZ (Reset)
 tft.setTextColor(CYAN);
 tft.setTextSize(2);
 tft.setCursor(20, 130);
 tft.print("1. ATZ...");
 
 Serial1.println("ATZ");
 delay(800);
 
 String response = readOBDResponse(2000);
 
 tft.setTextSize(1);
 tft.setCursor(20, 155);
 
 if(response.length() > 0) {
 tft.setTextColor(YELLOW);
 tft.print("Reponse: ");
 
 // Nettoyer l'affichage
 String display = cleanResponse(response);
 if(display.length() > 20) {
 tft.print(display.substring(0, 20));
 } else {
 tft.print(display);
 }
 
 Serial.print("ATZ -> ");
 Serial.println(response);
 } else {
 tft.setTextColor(RED);
 tft.print("Pas de reponse");
 Serial.println("ATZ -> No response");
 }
 
 delay(1500);
 
 // Test 2: ATI (Information)
 tft.fillRect(20, 170, 200, 40, BACKGROUND);
 tft.setTextColor(CYAN);
 tft.setTextSize(2);
 tft.setCursor(20, 170);
 tft.print("2. ATI...");
 
 Serial1.println("ATI");
 delay(500);
 
 response = readOBDResponse(1500);
 
 tft.setTextSize(1);
 tft.setCursor(20, 195);
 
 if(response.length() > 0) {
 tft.setTextColor(YELLOW);
 tft.print("Reponse: ");
 
 String display = cleanResponse(response);
 if(display.length() > 20) {
 tft.print(display.substring(0, 20));
 } else {
 tft.print(display);
 }
 
 Serial.print("ATI -> ");
 Serial.println(response);
 } else {
 tft.setTextColor(RED);
 tft.print("Pas de reponse");
 Serial.println("ATI -> No response");
 }
 
 delay(1500);
 
 // Test 3: ATRV (Tension batterie)
 tft.fillRect(20, 210, 200, 40, BACKGROUND);
 tft.setTextColor(CYAN);
 tft.setTextSize(2);
 tft.setCursor(20, 210);
 tft.print("3. ATRV...");
 
 Serial1.println("ATRV");
 delay(500);
 
 response = readOBDResponse(1500);
 
 tft.setTextSize(1);
 tft.setCursor(20, 235);
 
 if(response.length() > 0) {
 tft.setTextColor(YELLOW);
 tft.print("Reponse: ");
 
 String display = cleanResponse(response);
 if(display.length() > 20) {
 tft.print(display.substring(0, 20));
 } else {
 tft.print(display);
 }
 
 Serial.print("ATRV -> ");
 Serial.println(response);
 } else {
 tft.setTextColor(RED);
 tft.print("Pas de reponse");
 Serial.println("ATRV -> No response");
 }
 
 // Mode prêt
 delay(2000);
 tft.fillScreen(BACKGROUND);
 tft.setTextColor(GREEN);
 tft.setTextSize(3);
 tft.setCursor(30, 50);
 tft.print("PRET!");
 
 tft.setTextSize(2);
 tft.setTextColor(WHITE);
 tft.setCursor(30, 100);
 tft.print("Envoyez commandes");
 tft.setCursor(50, 130);
 tft.print("via Serial");
 
 Serial.println("\n=== MODE OBD ACTIF ===");
 Serial.println("Envoyez commandes OBD:");
 Serial.println("- ATZ (reset)");
 Serial.println("- ATI (info)");
 Serial.println("- ATRV (tension)");
 Serial.println("- ATSP0 (auto protocol)");
 Serial.println("- 0100 (test PID)");
}

String readOBDResponse(unsigned long timeout) {
 String response = "";
 unsigned long start = millis();
 
 while(millis() - start < timeout) {
 if(Serial1.available()) {
 char c = Serial1.read();
 response += c;
 if(c == '>') {
 break;
 }
 }
 }
 
 return response;
}

String cleanResponse(String input) {
 String output = "";
 for(int i = 0; i < input.length(); i++) {
 char c = input[i];
 if(c == '\r') {
 output += "\\r";
 } else if(c == '\n') {
 output += "\\n";
 } else if(c == '>') {
 output += ">";
 } else if(c >= 32 && c <= 126) {
 output += c;
 } else {
 output += "?";
 }
 }
 return output;
}

void loop() {
 // Mode commande manuelle
 if(Serial.available()) {
 String cmd = Serial.readStringUntil('\n');
 cmd.trim();
 
 if(cmd.length() > 0) {
 Serial.print(">>> ");
 Serial.println(cmd);
 
 // Envoyer à l'ELM327
 Serial1.println(cmd);
 delay(200);
 
 // Lire réponse
 String response = readOBDResponse(3000);
 Serial.print("<<< ");
 Serial.println(response);
 
 // Afficher sur écran
 tft.fillScreen(BACKGROUND);
 tft.setTextColor(CYAN);
 tft.setTextSize(2);
 tft.setCursor(10, 20);
 tft.print("Commande:");
 
 tft.setTextColor(WHITE);
 tft.setTextSize(3);
 tft.setCursor(10, 50);
 if(cmd.length() > 15) {
 tft.print(cmd.substring(0, 15));
 } else {
 tft.print(cmd);
 }
 
 tft.setTextColor(YELLOW);
 tft.setTextSize(2);
 tft.setCursor(10, 90);
 tft.print("Reponse:");
 
 tft.setTextSize(1);
 tft.setCursor(10, 120);
 
 String display = cleanResponse(response);
 // Afficher en plusieurs lignes si nécessaire
 int line = 0;
 for(int i = 0; i < display.length(); i += 30) {
 int endPos = (i + 30 < display.length()) ? i + 30 : display.length();
 tft.setCursor(10, 140 + line * 15);
 tft.print(display.substring(i, endPos));
 line++;
 if(line >= 5) break;
 }
 }
 }
 
 delay(10);
}
