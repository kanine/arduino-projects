# ESP32 Arduino Reference

Target: ESP32 WROOM / Dev Module — 3.3 V logic, Xtensa LX6 dual-core, Arduino-ESP32 core.

**Source:** [ESP32 Datasheet (Espressif)](https://documentation.espressif.com/esp32_datasheet_en.pdf)
**Pinout reference:** [ESP32-WROOM-32 Pinout — Last Minute Engineers](https://lastminuteengineers.com/esp32-wroom-32-pinout-reference/)
**Board photo:** [docs/images/elegoo-esp32-wroom.jpg](images/elegoo-esp32-wroom.jpg)

---

## Elegoo ESP32 WROOM Board Labels

The Elegoo ESP32 WROOM dev board silkscreen uses a `D`-prefix for GPIO pins. The mapping is **1:1** — `D`-number equals GPIO number.

| Board label | GPIO | Key function |
|---|---|---|
| D2 | GPIO2 | Onboard blue LED (active HIGH) |
| D4 | GPIO4 | General I/O |
| D5 | GPIO5 | VSPI CS / strapping pin |
| D12 | GPIO12 | Strapping — must be LOW at boot |
| D13 | GPIO13 | HSPI MOSI |
| D14 | GPIO14 | HSPI CLK |
| D15 | GPIO15 | Strapping / HSPI CS |
| D18 | GPIO18 | VSPI CLK |
| D19 | GPIO19 | VSPI MISO |
| D21 | GPIO21 | **I2C SDA (default)** |
| D22 | GPIO22 | **I2C SCL (default)** |
| D23 | GPIO23 | VSPI MOSI |
| D25 | GPIO25 | DAC1 / ADC2_CH8 |
| D26 | GPIO26 | DAC2 / ADC2_CH9 |
| D27 | GPIO27 | ADC2_CH7 |
| D32 | GPIO32 | ADC1_CH4 / Touch9 |
| D33 | GPIO33 | ADC1_CH5 / Touch8 |
| D34 | GPIO34 | ADC1_CH6 — input only |
| D35 | GPIO35 | ADC1_CH7 — input only |
| RX2 | GPIO16 | UART2 RX |
| TX2 | GPIO17 | UART2 TX |
| RX0 | GPIO3 | UART0 RX (USB serial) |
| TX0 | GPIO1 | UART0 TX (USB serial) |
| VN | GPIO39 | ADC1_CH3 — input only |
| VP | GPIO36 | ADC1_CH0 — input only |
| 3V3 | — | 3.3 V regulated output |
| VIN | — | 5–12 V input |
| GND | — | Ground |
| EN | — | Chip enable (active HIGH) |

> GPIO6–11 are not broken out — they are internally connected to SPI flash.

---

## Hardware Specs

| Feature | Value |
|---|---|
| CPU | Dual-core Xtensa LX6, up to 240 MHz |
| SRAM | 520 KB |
| ROM | 448 KB |
| Flash | 4 MB (typical on WROOM modules) |
| Logic voltage | 3.3 V (not 5 V tolerant) |
| Wi-Fi | 802.11 b/g/n, 2.4 GHz |
| Bluetooth | Classic BT 4.2 + BLE |
| ADC | 18 channels (12-bit), two units: ADC1 and ADC2 |
| DAC | 2 channels (8-bit), GPIO25 and GPIO26 |
| PWM (LEDC) | 16 channels |
| UART | 3 hardware ports |
| SPI | 4 buses (VSPI / HSPI user-accessible) |
| I2C | 2 buses |
| Touch | 10 capacitive touch pins |
| Serial baud | Up to 5 Mbps |

---

## GPIO Pinout

> All GPIOs are **3.3 V** — do not connect directly to 5 V signals.
> Internal pull resistors are ~45 kΩ.

| GPIO | I | O | Pull | ADC | DAC | Touch | Notes |
|------|---|---|------|-----|-----|-------|-------|
| 0 | ✓ | ✓ | U/D | ADC2_CH1 | — | T1 | **Strapping** — pulled up; LOW = flash mode |
| 1 | ✓ | ✓ | U/D | — | — | — | UART0 TX; debug output at boot |
| 2 | ✓ | ✓ | U/D | ADC2_CH2 | — | T2 | **Strapping** — on-board LED; must be LOW to flash |
| 3 | ✓ | ✓ | U/D | — | — | — | UART0 RX; HIGH at boot |
| 4 | ✓ | ✓ | U/D | ADC2_CH0 | — | T0 | |
| 5 | ✓ | ✓ | U/D | — | — | — | **Strapping** — SPI CS for VSPI; PWM at boot |
| 6–11 | — | — | — | — | — | — | **Do not use** — connected to SPI flash/PSRAM |
| 12 | ✓ | ✓ | U/D | ADC2_CH5 | — | T5 | **Strapping** — boot fails if pulled HIGH at reset |
| 13 | ✓ | ✓ | U/D | ADC2_CH4 | — | T4 | |
| 14 | ✓ | ✓ | U/D | ADC2_CH6 | — | T6 | PWM at boot |
| 15 | ✓ | ✓ | U/D | ADC2_CH3 | — | T3 | **Strapping** — PWM at boot |
| 16 | ✓ | ✓ | U/D | — | — | — | UART2 RX (default) |
| 17 | ✓ | ✓ | U/D | — | — | — | UART2 TX (default) |
| 18 | ✓ | ✓ | U/D | — | — | — | VSPI CLK |
| 19 | ✓ | ✓ | U/D | — | — | — | VSPI MISO |
| 21 | ✓ | ✓ | U/D | — | — | — | I2C SDA (default) |
| 22 | ✓ | ✓ | U/D | — | — | — | I2C SCL (default) |
| 23 | ✓ | ✓ | U/D | — | — | — | VSPI MOSI |
| 25 | ✓ | ✓ | U/D | ADC2_CH8 | DAC1 | — | |
| 26 | ✓ | ✓ | U/D | ADC2_CH9 | DAC2 | — | |
| 27 | ✓ | ✓ | U/D | ADC2_CH7 | — | T7 | |
| 32 | ✓ | ✓ | U/D | ADC1_CH4 | — | T9 | |
| 33 | ✓ | ✓ | U/D | ADC1_CH5 | — | T8 | |
| 34 | ✓ | — | none | ADC1_CH6 | — | — | Input-only; no pull resistors |
| 35 | ✓ | — | none | ADC1_CH7 | — | — | Input-only; no pull resistors |
| 36 | ✓ | — | none | ADC1_CH0 | — | — | Input-only; no pull resistors |
| 39 | ✓ | — | none | ADC1_CH3 | — | — | Input-only; no pull resistors |

I = input capable, O = output capable, Pull = U/D means hardware pull-up and pull-down available.

### Strapping pins summary

These pins are sampled at reset to configure boot mode. Avoid driving them to unexpected states during startup.

| GPIO | Concern |
|---|---|
| 0 | LOW = UART download mode; HIGH = normal boot |
| 2 | Must be floating or LOW during flashing |
| 5 | Controls SDIO slave timing |
| 12 | HIGH at boot causes 1.8 V flash voltage — board will not start |
| 15 | Controls boot log output |

---

## GPIO API

```cpp
pinMode(pin, mode);         // INPUT, OUTPUT, INPUT_PULLUP, INPUT_PULLDOWN
digitalWrite(pin, HIGH/LOW);
int v = digitalRead(pin);

// Interrupts
attachInterrupt(pin, handler, mode);  // mode: RISING, FALLING, CHANGE, ONLOW, ONHIGH
detachInterrupt(pin);
```

---

## ADC

### Units and pin mapping

| Unit | GPIOs | WiFi safe? |
|---|---|---|
| ADC1 | 32, 33, 34, 35, 36, 39 | Yes |
| ADC2 | 0, 2, 4, 12, 13, 14, 15, 25, 26, 27 | **No — unusable while WiFi is active** |

**Use ADC1 pins (32–39) for analog reads in WiFi sketches.**

### Attenuation and voltage range (ESP32)

| Constant | Input range |
|---|---|
| `ADC_ATTEN_DB_0` | 100 mV – 950 mV |
| `ADC_ATTEN_DB_2_5` | 100 mV – 1250 mV |
| `ADC_ATTEN_DB_6` | 150 mV – 1750 mV |
| `ADC_ATTEN_DB_11` | 150 mV – 3100 mV |

Default attenuation is `ADC_ATTEN_DB_11` (0–3.1 V range).

### API

```cpp
analogReadResolution(12);                  // 9–12 bits; default 12 (0–4095)
analogSetAttenuation(ADC_ATTEN_DB_11);    // all ADC pins
analogSetPinAttenuation(pin, ADC_ATTEN_DB_6); // single pin

int raw = analogRead(pin);                 // returns 0–4095 at 12-bit
int mV  = analogReadMilliVolts(pin);       // calibrated millivolts
```

### Caveats

- **ADC2 is blocked while WiFi radio is active.** `analogRead()` on ADC2 pins returns garbage.
- Avoid interrupts on GPIO36 and GPIO39 when using ADC or WiFi with sleep mode.
- ADC readings are non-linear near rail voltages; use `analogReadMilliVolts()` with calibration for accuracy.

---

## DAC

Two 8-bit DAC channels. Output range 0–3.3 V.

| Channel | GPIO |
|---|---|
| DAC1 | 25 |
| DAC2 | 26 |

```cpp
#include <driver/dac.h>
dacWrite(25, 128);   // 0–255 maps to 0–3.3 V
```

---

## PWM (LEDC)

ESP32 has 16 independent PWM channels, any GPIO (except input-only 34–39).

### Frequency vs resolution trade-off

`freq × (2^resolution) ≤ 80 MHz (APB clock)`

| Frequency | Max resolution |
|---|---|
| 1 kHz | 16 bit |
| 5 kHz | 13 bit |
| 10 kHz | 12 bit |
| 40 kHz | 10 bit |

### API (Arduino-ESP32 3.x)

```cpp
// Attach pin — channel auto-assigned
ledcAttach(pin, freq, resolution);         // e.g. ledcAttach(5, 1000, 12)
ledcWrite(pin, duty);                      // duty 0 to (2^resolution - 1)
ledcReadFreq(pin);
ledcChangeFrequency(pin, freq, resolution);
ledcDetach(pin);

// Arduino-compatible shim (0–255)
analogWrite(pin, value);

// Tones
ledcWriteTone(pin, frequency);             // 50% duty at freq Hz
ledcWriteNote(pin, NOTE_C, 4);            // musical note, octave 4

// Hardware fades
ledcFade(pin, start_duty, end_duty, fade_time_ms);
ledcFadeWithInterrupt(pin, start, end, time_ms, callback);
```

---

## I2C

Default pins: **SDA = GPIO21, SCL = GPIO22**. Any GPIO can be remapped.

```cpp
#include <Wire.h>

// Default pins
Wire.begin();

// Custom pins (call before begin)
Wire.setPins(sdaPin, sclPin);
Wire.begin();

// With frequency
Wire.begin(sdaPin, sclPin, 400000);   // 400 kHz fast mode

// Slave mode
Wire.begin(slaveAddr, sdaPin, sclPin, 0);
Wire.onReceive([](int n){ /* read n bytes */ });
Wire.onRequest([]{ /* write response */ });

Wire.setClock(100000);    // 100 kHz standard / 400 kHz fast
Wire.setTimeOut(50);      // ms; default 50
```

---

## SPI

Two user-accessible buses. Pins can be remapped.

| Bus | CS | CLK | MOSI | MISO |
|---|---|---|---|---|
| VSPI (SPI3) | 5 | 18 | 23 | 19 |
| HSPI (SPI2) | 15 | 14 | 13 | 12 |

```cpp
#include <SPI.h>

SPI.begin();                              // VSPI defaults
SPI.begin(clk, miso, mosi, cs);          // custom pins

SPIClass vspi(VSPI);
vspi.begin(18, 19, 23, 5);

SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
SPI.transfer(byte);
SPI.endTransaction();
```

---

## UART / Serial

Three hardware UART ports. Pins are remappable.

| Port | Default TX | Default RX | Notes |
|---|---|---|---|
| UART0 (Serial) | GPIO1 | GPIO3 | USB-serial; used for flashing and Serial monitor |
| UART1 (Serial1) | GPIO10 | GPIO9 | Overlaps flash pins — **remap before use** |
| UART2 (Serial2) | GPIO17 | GPIO16 | General use |

```cpp
Serial.begin(115200);                     // UART0 (USB)
Serial2.begin(9600);                      // UART2 defaults (GPIO16/17)
Serial2.begin(9600, SERIAL_8N1, rxPin, txPin);  // custom pins
```

Remap UART1 away from flash pins:
```cpp
Serial1.begin(9600, SERIAL_8N1, 13, 12); // rx=13, tx=12
```

---

## Wi-Fi

```cpp
#include <WiFi.h>

// Station mode
WiFi.begin(ssid, password);
while (WiFi.status() != WL_CONNECTED) delay(500);
Serial.println(WiFi.localIP());

// Access point mode
WiFi.softAP(ssid, password);

// Event callbacks run on a separate FreeRTOS task — keep them thread-safe
WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info){ ... });

// Static IP (call before begin)
WiFi.config(localIP, gateway, subnet, dns);
WiFi.setHostname("my-esp32");
WiFi.setAutoReconnect(true);
```

**ADC2 is unavailable while WiFi is active.** Switch to ADC1 pins (GPIO32–39) for analog reads in WiFi sketches.

---

## Deep Sleep

Program execution restarts from `setup()` after wake. Use `RTC_DATA_ATTR` to persist variables across sleep cycles.

```cpp
#include <esp_sleep.h>

RTC_DATA_ATTR int bootCount = 0;   // survives deep sleep

// Timer wake-up
esp_sleep_enable_timer_wakeup(10 * 1000000ULL);  // 10 seconds (microseconds)
esp_deep_sleep_start();

// External wake-up — single pin (RTC GPIO only: 0,2,4,12–15,25–27,32–39)
esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 1);     // wake on HIGH

// External wake-up — multiple pins
uint64_t mask = (1ULL << 32) | (1ULL << 33);
esp_sleep_enable_ext1_wakeup_io(mask, ESP_EXT1_WAKEUP_ANY_HIGH);
esp_deep_sleep_start();

// Determine wake reason
esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
// ESP_SLEEP_WAKEUP_TIMER, ESP_SLEEP_WAKEUP_EXT0, etc.
```

Non-RTC GPIO pins power off during deep sleep. If you need a pin held HIGH/LOW during sleep, configure it with the HOLD feature or use an RTC-capable GPIO.

---

## Common Pitfalls

| Pitfall | Detail |
|---|---|
| 5 V on GPIO | ESP32 is 3.3 V only — use a level shifter |
| ADC2 + WiFi | ADC2 reads fail silently when WiFi radio is on |
| GPIO6–11 | Reserved for SPI flash — using them causes crashes |
| GPIO12 HIGH at boot | Selects 1.8 V flash voltage, prevents booting |
| Input-only pins (34–39) | No output, no internal pull-ups — cannot drive or pull |
| UART1 default pins | Overlap flash (GPIO9/10) — always remap before use |
| PWM + analogWrite | Both use LEDC; do not mix `ledcAttach()` and `analogWrite()` on the same pin |
| Interrupt on 36/39 | Unreliable when ADC or WiFi sleep is active |
| Boot pin contention | GPIO0, 2, 15 state at reset determines boot mode |
