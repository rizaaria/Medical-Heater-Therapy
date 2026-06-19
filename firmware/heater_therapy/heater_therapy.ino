// ======================================================
// HEATER THERAPY FINAL VERSION
// ======================================================
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include <OneWire.h>
#include <DallasTemperature.h>

MCUFRIEND_kbv tft;

// ================= PIN =================

#define HEATER_PIN     10
#define BUZZER_PIN     11
#define LED_PIN        13
#define ONE_WIRE_BUS   12

// ================= SENSOR =================

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// ================= TOUCH =================

#define MINPRESSURE 200
#define MAXPRESSURE 1000

const int XP = 6, XM = A2, YP = A1, YM = 7;

const int TS_LEFT = 212;
const int TS_RT   = 889;
const int TS_TOP  = 951;
const int TS_BOT  = 230;

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

// ================= COLOR =================

#define BLACK       0x0000
#define WHITE       0xFFFF
#define RED         0xF800
#define GREEN       0x07E0
#define BLUE        0x001F
#define CYAN        0x07FF
#define YELLOW      0xFFE0
#define ORANGE      0xFD20
#define NAVY        0x000F
#define DARKGREY    0x4208
#define LIGHTGREY   0xC618
#define BG          0x18E3
#define CARD        0x2965

// ================= SYSTEM =================

bool heaterOn = false;
bool heaterOutput = false;

bool timerRunning = false;
bool finished = false;

bool overheat = false;
bool overheatBeeped = false;

bool cycleMode = false;

bool lastTouchState = false;

// ================= TIMER =================

int timeOptions[3] = {5,10,20};

int timeIndex = 0;
int setTime = 5;

unsigned long startMillis = 0;
unsigned long lastDisplay = 0;
unsigned long lastTouchTime = 0;

// ================= HEATER CYCLE =================

unsigned long heaterMillis = 0;

bool heaterCycleState = true;

// ================= TEMPERATURE =================

float filteredTemp = 25;

#define TEMP_OFFSET        2.0

#define TARGET_TEMP        39
#define OVERHEAT_TEMP      50

// ======================================================
// TOUCH
// ======================================================

bool getTouch(int &x, int &y)
{
    TSPoint p = ts.getPoint();

    pinMode(YP, OUTPUT);
    pinMode(XM, OUTPUT);

    digitalWrite(YP, HIGH);
    digitalWrite(XM, HIGH);

    if (p.z < MINPRESSURE || p.z > MAXPRESSURE)
        return false;

    x = map(p.y, TS_TOP, TS_BOT, 0, tft.width());
    y = map(p.x, TS_RT, TS_LEFT, 0, tft.height());

    return true;
}

// ======================================================
// BUZZER
// ======================================================

void beepShort(int duration)
{
    digitalWrite(BUZZER_PIN,HIGH);
    delay(duration);
    digitalWrite(BUZZER_PIN,LOW);
}

void beepStart()
{
    beepShort(120);
}

void beepStop()
{
    beepShort(120);

    delay(120);

    beepShort(120);
}

void beepFinish()
{
    for(int i=0;i<3;i++)
    {
        beepShort(150);

        delay(150);
    }
}

void beepOverheat()
{
    digitalWrite(BUZZER_PIN,HIGH);

    delay(1200);

    digitalWrite(BUZZER_PIN,LOW);
}

// ======================================================
// SPLASH
// ======================================================

void showSplash()
{
    tft.fillScreen(BLACK);

    tft.fillRoundRect(20,25,280,190,12,NAVY);

    tft.setTextColor(CYAN);
    tft.setTextSize(3);

    tft.setCursor(45,45);
    tft.print("HEATER THERAPY");

    tft.drawFastHLine(40,80,240,LIGHTGREY);

    tft.setTextSize(2);
    tft.setTextColor(WHITE);

    tft.setCursor(60,110);
    tft.print("Amalia Stefhanny");

    tft.setCursor(48,140);
    tft.print("P2.20.40.1.23.003");

    tft.setTextSize(1);
    tft.setTextColor(LIGHTGREY);

    tft.setCursor(100,200);
    tft.print("Teknik Elektromedik");

    delay(3000);
}

// ======================================================
// UI
// ======================================================

void drawUI()
{
    tft.fillScreen(BG);

    tft.fillRoundRect(10,10,300,40,10,NAVY);

    tft.setTextColor(WHITE);
    tft.setTextSize(3);

    tft.setCursor(55,18);
    tft.print("HEATER THERAPY");

    // TIMER CARD
    tft.fillRoundRect(10,60,170,90,10,CARD);

    tft.setTextSize(2);
    tft.setTextColor(CYAN);

    tft.setCursor(28,70);
    tft.print("THERAPY TIME");

    // BUTTON -
    tft.fillRoundRect(20,105,40,30,6,DARKGREY);

    tft.setTextColor(WHITE);
    tft.setTextSize(3);

    tft.setCursor(32,108);
    tft.print("-");

    // BUTTON +
    tft.fillRoundRect(130,105,40,30,6,DARKGREY);

    tft.setCursor(142,108);
    tft.print("+");

    // START CARD
    tft.fillRoundRect(190,60,120,90,10,CARD);

    // STATUS CARD
    tft.fillRoundRect(10,160,300,80,10,CARD);
}

// ======================================================
// START BUTTON
// ======================================================

void drawStartButton()
{
    uint16_t color =
        heaterOn ? RED : GREEN;

    tft.fillRoundRect(205,95,90,40,8,color);

    tft.setTextColor(WHITE);
    tft.setTextSize(2);

    if(heaterOn)
    {
        tft.setCursor(225,108);
        tft.print("STOP");
    }
    else
    {
        tft.setCursor(220,108);
        tft.print("START");
    }
}

// ======================================================
// START HEATER
// ======================================================

void startHeater()
{
    if(filteredTemp >= OVERHEAT_TEMP)
    {
        beepOverheat();
        return;
    }

    heaterOn = true;
    heaterOutput = true;

    finished = false;

    overheat = false;
    overheatBeeped = false;

    cycleMode = false;

    timerRunning = false;

    startMillis = millis();

    heaterMillis = millis();

    heaterCycleState = true;

    // soft start
    analogWrite(HEATER_PIN,180);

    delay(500);

    analogWrite(HEATER_PIN,255);

    digitalWrite(LED_PIN,HIGH);

    drawStartButton();

    beepStart();
}

// ======================================================
// STOP HEATER
// ======================================================

void stopHeater()
{
    heaterOn = false;

    heaterOutput = false;

    timerRunning = false;

    analogWrite(HEATER_PIN,0);

    digitalWrite(LED_PIN,LOW);

    drawStartButton();
}

// ======================================================
// CONTROL
// ======================================================

void controlLogic(float t1)
{
    // =====================================
    // SENSOR ERROR
    // =====================================

    if(t1 == DEVICE_DISCONNECTED_C ||
       t1 < -20 ||
       t1 > 100)
    {
        stopHeater();

        overheat = true;

        return;
    }

    // =====================================
    // OVERHEAT
    // =====================================

    if(t1 >= OVERHEAT_TEMP)
    {
        stopHeater();

        overheat = true;

        if(!overheatBeeped)
        {
            beepOverheat();

            overheatBeeped = true;
        }

        return;
    }

    // =====================================
    // RESET OVERHEAT
    // =====================================

    if(overheat && t1 < 40)
    {
        overheat = false;

        overheatBeeped = false;
    }

    // =====================================
    // START TIMER
    // =====================================

    if(heaterOn &&
       !timerRunning &&
       t1 >= TARGET_TEMP)
    {
        timerRunning = true;

        startMillis = millis();
    }

    // =====================================
    // MASUK MODE CYCLING
    // =====================================

    if(heaterOn &&
       !cycleMode &&
       t1 >= TARGET_TEMP)
    {
        cycleMode = true;

        heaterMillis = millis();
    }

    // =====================================
    // FULL HEATING MODE
    // =====================================

    if(heaterOn && !cycleMode)
    {
        analogWrite(HEATER_PIN,255);

        heaterOutput = true;
    }

    // =====================================
    // CYCLING MODE
    // =====================================

    if(heaterOn && cycleMode)
    {
        unsigned long elapsed =
            millis() - heaterMillis;

        // ON 3 detik
        if(heaterCycleState)
        {
            analogWrite(HEATER_PIN,255);

            heaterOutput = true;

            if(elapsed >= 2000)
            {
                heaterCycleState = false;

                heaterMillis = millis();
            }
        }

        // OFF 10 detik
        else
        {
            analogWrite(HEATER_PIN,0);

            heaterOutput = false;

            if(elapsed >= 7500)
            {
                heaterCycleState = true;

                heaterMillis = millis();
            }
        }
    }
}

// ======================================================
// TIMER
// ======================================================

void updateTimer()
{
    if(timerRunning)
    {
        unsigned long elapsed =
            (millis() - startMillis)/1000;

        int remain =
            (setTime * 60) - elapsed;

        if(remain <= 0)
        {
            stopHeater();

            finished = true;

            beepFinish();
        }
    }
}

// ======================================================
// DISPLAY
// ======================================================

void updateDisplay(float t1, float t2)
{
    // TIMER DISPLAY
    tft.fillRect(68,100,55,30,CARD);

    tft.setTextColor(WHITE);
    tft.setTextSize(3);

    if(setTime == 5)
        tft.setCursor(84,105);
    else
        tft.setCursor(74,105);

    tft.print(setTime);

    tft.setTextSize(1);
    tft.print(" MIN");

    // TEMPERATURE
    tft.fillRect(20,170,260,20,CARD);

    tft.setTextSize(2);
    tft.setTextColor(WHITE);

    tft.setCursor(20,170);
    tft.print("H:");
    tft.print(t1,1);
    tft.print((char)247);
    tft.print("C");

    tft.setCursor(145,170);
    tft.print("B:");
    tft.print(t2,1);
    tft.print((char)247);
    tft.print("C");

    // HEATER STATUS
    tft.setCursor(240,170);

    if(heaterOutput)
    {
        tft.setTextColor(RED);
        tft.print("ON ");
    }
    else
    {
        tft.setTextColor(GREEN);
        tft.print("OFF");
    }

    // PROGRESS BAR
    int barWidth = map(t1,25,50,0,260);

    if(barWidth < 0) barWidth = 0;
    if(barWidth > 260) barWidth = 260;

    tft.fillRect(20,198,260,15,CARD);

    tft.drawRoundRect(20,198,260,15,4,WHITE);

    uint16_t barColor = GREEN;

    if(t1 >= 37)
        barColor = ORANGE;

    if(t1 >= 45)
        barColor = RED;

    tft.fillRoundRect(20,198,barWidth,15,4,barColor);

    // STATUS
    tft.fillRect(20,220,260,15,CARD);

    tft.setCursor(20,220);

    if(overheat)
    {
        tft.setTextColor(RED);
        tft.print("STATUS : OVERHEAT");
    }
    else if(finished)
    {
        tft.setTextColor(YELLOW);
        tft.print("STATUS : DONE");
    }
    else if(cycleMode)
    {
        tft.setTextColor(GREEN);
        tft.print("STATUS : THERAPY");
    }
    else if(heaterOn)
    {
        tft.setTextColor(ORANGE);
        tft.print("STATUS : HEATING");
    }
    else
    {
        tft.setTextColor(WHITE);
        tft.print("STATUS : READY");
    }

    // COUNTDOWN
    if(timerRunning)
    {
        unsigned long elapsed =
            (millis() - startMillis)/1000;

        int remain =
            (setTime * 60) - elapsed;

        int min = remain / 60;
        int sec = remain % 60;

        char buf[10];

        sprintf(buf,"%02d:%02d",min,sec);

        tft.fillRect(205,68,90,20,CARD);

        tft.setTextColor(CYAN);
        tft.setTextSize(2);

        tft.setCursor(220,70);
        tft.print(buf);
    }
    else
    {
        tft.fillRect(205,68,90,20,CARD);

        tft.setTextColor(LIGHTGREY);
        tft.setTextSize(2);

        tft.setCursor(225,70);
        tft.print("--:--");
    }
}

// ======================================================
// TOUCH
// ======================================================

void handleTouch(int tx, int ty)
{
    if(!heaterOn)
    {
        // BUTTON -
        if(tx>=20 && tx<=60 &&
           ty>=105 && ty<=135)
        {
            if(timeIndex > 0)
            {
                timeIndex--;

                setTime =
                    timeOptions[timeIndex];
            }
        }

        // BUTTON +
        if(tx>=130 && tx<=170 &&
           ty>=105 && ty<=135)
        {
            if(timeIndex < 2)
            {
                timeIndex++;

                setTime =
                    timeOptions[timeIndex];
            }
        }
    }

    // START STOP
    if(tx>=205 && tx<=295 &&
       ty>=95 && ty<=135)
    {
        if(!heaterOn)
        {
            startHeater();
        }
        else
        {
            stopHeater();

            beepStop();
        }
    }
}

// ======================================================
// SETUP
// ======================================================

void setup()
{
    Serial.begin(115200);

    pinMode(HEATER_PIN,OUTPUT);
    pinMode(BUZZER_PIN,OUTPUT);
    pinMode(LED_PIN,OUTPUT);

    digitalWrite(HEATER_PIN,LOW);
    digitalWrite(BUZZER_PIN,LOW);
    digitalWrite(LED_PIN,LOW);

    sensors.begin();

    uint16_t ID = tft.readID();

    if(ID == 0xD3D3)
        ID = 0x9486;

    tft.begin(ID);

    tft.setRotation(1);

    showSplash();

    drawUI();

    drawStartButton();
}

// ======================================================
// LOOP
// ======================================================

void loop()
{
    int tx, ty;

    // =====================================
    // SENSOR READ
    // =====================================

    sensors.requestTemperatures();

    float rawT1 =
        sensors.getTempCByIndex(0);

    float rawT2 =
        sensors.getTempCByIndex(1);

    // FILTER
    if(rawT1 > -20 && rawT1 < 100)
    {
        filteredTemp =
            (filteredTemp * 0.7) +
            (rawT1 * 0.3);
    }

    float t1 =
        filteredTemp + TEMP_OFFSET;

    float t2 = rawT2;

    if(rawT2 == DEVICE_DISCONNECTED_C)
    {
        t2 = 0;
    }

    // =====================================
    // TOUCH
    // =====================================

    bool currentTouch =
        getTouch(tx, ty);

    if(currentTouch &&
       !lastTouchState &&
       millis() - lastTouchTime > 250)
    {
        handleTouch(tx, ty);

        lastTouchTime = millis();
    }

    lastTouchState = currentTouch;

    // =====================================
    // SYSTEM
    // =====================================

    controlLogic(t1);

    updateTimer();

    // =====================================
    // LED STATUS
    // =====================================

    static unsigned long ledMillis = 0;
    static bool ledBlink = false;

    if(overheat)
    {
        if(millis() - ledMillis > 150)
        {
            ledBlink = !ledBlink;

            digitalWrite(LED_PIN,ledBlink);

            ledMillis = millis();
        }
    }
    else if(heaterOn && !heaterOutput)
    {
        if(millis() - ledMillis > 500)
        {
            ledBlink = !ledBlink;

            digitalWrite(LED_PIN,ledBlink);

            ledMillis = millis();
        }
    }
    else if(heaterOutput)
    {
        digitalWrite(LED_PIN,HIGH);
    }
    else
    {
        digitalWrite(LED_PIN,LOW);
    }

    // =====================================
    // DISPLAY
    // =====================================

    if(millis() - lastDisplay > 500)
    {
        updateDisplay(t1,t2);

        lastDisplay = millis();
    }
}