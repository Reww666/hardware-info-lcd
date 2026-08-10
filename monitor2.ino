#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

String currentTitle = "";
String currentArtist = "";
unsigned long lastScroll = 0;
int scrollIndex = 0;
bool scrolling = false;

void setup() {
    Serial.begin(9600);
    lcd.init();
    lcd.backlight();
}

void loop() {
    // Traitement des données série
    if (Serial.available() > 0) {
        String line = Serial.readStringUntil('\n');

        if (line.startsWith("TITLE:")) {
            currentTitle = line.substring(6);
            scrollIndex = 0;      // Réinitialise le défilement
            scrolling = true;
        } else if (line.startsWith("ARTIST:")) {
            currentArtist = line.substring(7);
        }

        // Affichage initial
        if (currentTitle.length() > 0 && currentArtist.length() > 0) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(currentTitle);
            lcd.setCursor(0, 1);
            lcd.print(currentArtist);
        }
    }

    // Défilement du titre (carrousel)
    if (scrolling && millis() - lastScroll > 300) {
        lastScroll = millis();
        scrollTitle();
    }
}

String getScrollText(String text, int index, int maxWidth = 16) {
    String sep = "   "; // Séparateur (3 espaces)
    String scrollText = text + sep + text;
    int totalLen = scrollText.length();

    if (text.length() <= maxWidth) {
        // Pas de défilement nécessaire
        return text;
    }

    // Si l'index dépasse la fin, on recommence
    if (index >= totalLen - maxWidth) {
        index = 0;
    }

    return scrollText.substring(index, index + maxWidth);
}

void scrollTitle() {
    int max = 16;
    if (currentTitle.length() <= max) {
        lcd.setCursor(0, 0);
        lcd.print(currentTitle);
        return;
    }

    // Calcul du texte affiché
    String displayed = getScrollText(currentTitle, scrollIndex, max);
    lcd.setCursor(0, 0);
    lcd.print(displayed);

    scrollIndex++;
    if (scrollIndex >= currentTitle.length() + 3) {
        scrollIndex = 0; // Retour au début sans coupure
    }
}