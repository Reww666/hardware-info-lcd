#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

String currentTitle = "";
String currentArtist = "";

unsigned long lastActionTitle = 0;
unsigned long lastActionArtist = 0;

int scrollIndexTitle = 0;
int scrollIndexArtist = 0;
bool scrollingTitle = false;
bool scrollingArtist = false;

bool isPausedTitle = true;  // true = affichage fixe, false = défilement
bool isPausedArtist = true;

bool atEndTitle = false;   // true = on vient de finir le défilement (en pause à la fin)
bool atEndArtist = false;

const unsigned long pauseDuration = 4000; // 4 secondes de pause
const unsigned long scrollSpeed = 350;    // vitesse de défilement

int maxStepsTitle = 0;
int maxStepsArtist = 0;

void setup() {
    Serial.begin(9600);
    lcd.init();
    lcd.backlight();
}

void loop() {
    if (Serial.available() > 0) {
        String line = Serial.readStringUntil('\n');
        line.trim(); // enlève un éventuel \r ou espace résiduel

        if (line.startsWith("TITLE:")) {
            currentTitle = line.substring(6);
            scrollIndexTitle = 0;
            scrollingTitle = true;
            maxStepsTitle = currentTitle.length() - 16;
            if (maxStepsTitle < 0) maxStepsTitle = 0;
            isPausedTitle = true;
            atEndTitle = false;
            lastActionTitle = millis();
            display();
        } else if (line.startsWith("ARTIST:")) {
            currentArtist = line.substring(7);
            scrollIndexArtist = 0;
            scrollingArtist = true;
            maxStepsArtist = currentArtist.length() - 16;   // <-- bug fixé : était absent
            if (maxStepsArtist < 0) maxStepsArtist = 0;
            isPausedArtist = true;
            atEndArtist = false;
            lastActionArtist = millis();
            display();
        }
    }

    updateScroll(currentTitle, scrollingTitle, isPausedTitle, lastActionTitle,
                 scrollIndexTitle, maxStepsTitle, atEndTitle);

    updateScroll(currentArtist, scrollingArtist, isPausedArtist, lastActionArtist,
                 scrollIndexArtist, maxStepsArtist, atEndArtist);
}

// Logique de défilement commune (utilisée pour le titre ET l'artiste)
// Cycle : pause au début -> défilement -> pause à la fin -> retour au début -> ...
void updateScroll(const String &text, bool scrolling, bool &isPaused,
                   unsigned long &lastAction, int &scrollIndex, int maxSteps,
                   bool &atEnd) {
    if (!scrolling || text.length() == 0) return;

    unsigned long now = millis();

    if (isPaused) {
        // Phase fixe (soit au début, soit à la fin du défilement)
        if (now - lastAction > pauseDuration) {
            if (atEnd) {
                // La pause de fin est terminée : on revient au début
                scrollIndex = 0;
                atEnd = false;
                lastAction = now;
                display();
                // On reste en pause (isPaused reste true) pour marquer une
                // pause de lecture au début avant de repartir en défilement.
            } else {
                // La pause de début est terminée : on lance le défilement
                isPaused = false;
                lastAction = now;
            }
        }
    } else {
        // Phase défilement
        if (now - lastAction > scrollSpeed) {
            lastAction = now;
            scrollIndex++;
            if (scrollIndex >= maxSteps) {
                scrollIndex = maxSteps;
                isPaused = true;
                atEnd = true;
                lastAction = now;
            }
            display(); // redessine à chaque pas, y compris la dernière frame
        }
    }
}

void display() {
    lcd.clear();
    lcd.setCursor(0, 0);
    if (currentTitle.length() > 16) {
        lcd.print(currentTitle.substring(scrollIndexTitle, scrollIndexTitle + 16));
    } else {
        lcd.print(currentTitle);
    }
    lcd.setCursor(0, 1);
    if (currentArtist.length() > 16) {
        lcd.print(currentArtist.substring(scrollIndexArtist, scrollIndexArtist + 16));
    } else {
        lcd.print(currentArtist);
    }
}
