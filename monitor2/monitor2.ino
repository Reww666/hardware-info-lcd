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

bool isPausedTitle = true;  // true = fixed display, false = scrolling
bool isPausedArtist = true;

bool atEndTitle = false;   // true = scrolling hast just finished (pause at the end)
bool atEndArtist = false;

const unsigned long pauseDuration = 4000; // 4 seconds pause
const unsigned long scrollSpeed = 350;    // Scroll speed

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
        line.trim(); // Remove any trainling \r or whitespace

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
            maxStepsArtist = currentArtist.length() - 16; 
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

// Common scrolling logic (used for both the title AND the artist)
// Cycle: pause at the beginning -> scroll -> pause at the end -> return to the beginning -> ...
void updateScroll(const String &text, bool scrolling, bool &isPaused,
                   unsigned long &lastAction, int &scrollIndex, int maxSteps,
                   bool &atEnd) {
    if (!scrolling || text.length() == 0) return;

    unsigned long now = millis();

    if (isPaused) {
        // Fixed phase (either at the beginning or at the end of scrolling)
        if (now - lastAction > pauseDuration) {
            if (atEnd) {
                // The end pause is over: return to the beginning
                scrollIndex = 0;
                atEnd = false;
                lastAction = now;
                display();
                // Stay paused (isPaused remains true) to indicate a
                // playback pause at the beginning before scrolling resumes.
            } else {
                // La pause de début est terminée : on lance le défilement
                isPaused = false;
                lastAction = now;
            }
        }
    } else {
        // The beginning pause is over: start scrolling
        if (now - lastAction > scrollSpeed) {
            lastAction = now;
            scrollIndex++;
            if (scrollIndex >= maxSteps) {
                scrollIndex = maxSteps;
                isPaused = true;
                atEnd = true;
                lastAction = now;
            }
            display(); // Redraw at every step, including the last frame
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
