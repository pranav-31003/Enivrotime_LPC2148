# EnviroTime — Digital Clock + Temperature Monitor

EnviroTime is a bare-metal embedded C project built for the **LPC2148 (ARM7TDMI-S)** microcontroller. It continuously displays the current time, date, day of week, and ambient temperature on a 16x2 LCD, with an alarm feature and a password-protected settings menu.

Developed in **Keil µVision** as a mini/academic project.

## Features

- **Live clock + calendar** — HH:MM:SS and DD/MM/YYYY with day of week, driven by the LPC2148's on-chip RTC
- **Temperature monitoring** — reads an LM35 sensor via the on-chip ADC and displays °C in real time
- **Alarm system** — buzzer sounds when the RTC time matches the set alarm time; a dedicated switch silences it
- **Password-protected edit menu** — 4x4 matrix keypad entry, 4-digit password, locks out for 30 seconds after 3 failed attempts
- **Editable settings** — time, date, alarm, and password can all be changed from the on-device menu

## Display layout

```
┌────────────────┐
│12:45:22 T:28°C │   Line 1: Time + Temperature
│02/05/2026 FRI  │   Line 2: Date + Day
└────────────────┘
```

## Hardware

| Component        | Notes                                  |
|-------------------|-----------------------------------------|
| LPC2148           | ARM7TDMI-S microcontroller @ 60 MHz     |
| 16x2 LCD          | HD44780-compatible, 8-bit mode          |
| LM35              | Analog temperature sensor               |
| 4x4 matrix keypad | Used for password + menu entry          |
| Buzzer            | Alarm indicator (via NPN transistor)    |
| 2x push switches  | Edit mode entry / Alarm stop            |

### Pin connections

| Peripheral        | LPC2148 Pin(s)      | Notes                          |
|--------------------|----------------------|----------------------------------|
| LCD Data D0–D7     | P0.0 – P0.7          | 8-bit data bus                  |
| LCD RS             | P0.8                 | Register Select                 |
| LCD RW             | Tied to GND           | Write-only, no MCU pin needed   |
| LCD EN             | P0.9                 | Enable strobe                   |
| Edit switch (SW1)  | P0.10                | Input, active LOW               |
| Alarm switch (SW2) | P0.11                | Input, active LOW               |
| Buzzer output      | P0.12                | Output, active HIGH             |
| LM35 sensor        | P0.28 (AIN1 / AD0.1) | Analog input                    |
| Keypad ROW0–ROW3   | P1.20 – P1.23         | Output                          |
| Keypad COL0–COL3   | P1.16 – P1.19         | Input                           |

## Project structure

```
EnviroTime/
├── main/          # Application entry point and main loop
├── lcd/           # 16x2 LCD driver
├── rtc/           # Real-time clock driver
├── adc/           # ADC driver (LM35 temperature reading)
├── keypad/        # 4x4 matrix keypad driver
├── buzzer/        # Buzzer (alarm) driver
├── switch_drv/    # Push-button switch driver
├── security/      # Password authentication module
├── menu/          # Edit menu (time/date/alarm/password)
├── delay/         # Software delay utilities
├── types/         # Common type definitions
├── defines/       # Bit-manipulation macros
├── Startup.s      # Startup assembly file
└── MINI_PROJECT.uvproj   # Keil µVision project file
```

## Software / Tools required

- **Keil µVision (MDK-ARM)** — IDE and compiler used for this project
- **Flash Magic** or a similar LPC2148 flashing tool, to program the `.hex` file over UART/ISP

## How to build

1. Open `MINI_PROJECT.uvproj` in Keil µVision.
2. Build the project (`F7`) — this generates `MINI_PROJECT.hex`.
3. Connect the LPC2148 board in ISP mode and flash `MINI_PROJECT.hex` using Flash Magic (or your preferred flashing tool).
4. Power on the board — the LCD will show a 2-second splash screen, then start monitoring.

## How it works (main loop)

1. `SystemInit_EnviroTime()` initializes the LCD, RTC, ADC, keypad, buzzer, switches, security, and menu modules, then shows a splash screen.
2. The main loop repeatedly:
   - Reads RTC time/date and LM35 temperature, and updates the LCD
   - Checks whether the alarm time has been reached, and sounds the buzzer if so
   - Checks whether the alarm-stop switch was pressed, to silence the buzzer
   - Checks whether the edit switch was pressed — if so, prompts for a password and, on success, opens the edit menu
   - Delays 200ms to keep the display refresh smooth

## Default credentials

- Default password: `1234` (changeable from the edit menu)
- Default startup time/date is set in `main.c` inside `SystemInit_EnviroTime()` — update this to the real current time before flashing.

## Author

Embedded Systems Team — Mini Project, 2026

## License

Add a license of your choice (e.g. MIT) if you'd like others to freely reuse this code.
