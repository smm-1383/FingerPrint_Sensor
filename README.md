# Fingerprint Sensor -- ATmega32 (Proteus Mock)

This project implements a **fake fingerprint sensor** on an **ATmega32**
microcontroller for use in **Proteus simulations**.\
It does *not* perform real biometric processing --- it only simulates a
minimal UART‑based fingerprint module for testing embedded systems.

------------------------------------------------------------------------

## 📌 Features

-   MCU: **ATmega32 @ 8 MHz**
-   UART: **9600 baud, 8N1**
-   Fake fingerprint interface using:
    -   7 pattern bits on **PA0..PA6** (0--127 pattern value)
    -   1 finger‑detect input on **PA7** (1 = finger present)
-   EEPROM‑based mapping:\
    **`ID → Pattern`**\
    (one pattern can be linked to multiple IDs)
-   Supports:
    -   **Enroll** (assign pattern to an ID)
    -   **Scan** (identify the ID)
    -   **Delete** (remove a stored ID)
-   Clean UART command parser + interrupt‑based RX

------------------------------------------------------------------------

## 📌 Pinout

### Sensor simulation pins

  -----------------------------------------------------------------------
  Pin                     Function
  ----------------------- -----------------------------------------------
  **PA0..PA6**            7‑bit pattern input (Logic States / Toggles in
                          Proteus)

  **PA7**                 Finger detect (1 = finger present, 0 = no
                          finger)
  -----------------------------------------------------------------------

### UART pins

  Pin             Function
  --------------- ------------------------------------------------------
  **PD0 / RXD**   Receive data from master (connect to TX of terminal)
  **PD1 / TXD**   Send data to master (connect to RX of terminal)

------------------------------------------------------------------------

## 📌 UART Protocol

### Boot message

Sent once after startup:

    READY\r\n

------------------------------------------------------------------------

### 1. Scan (`S`)

**Command:**

    S\n

**Responses:** - No finger detected: `NOF\r\n` - Pattern not registered:
`FAIL\r\n` - Pattern matched: `OK:<id>\r\n`

------------------------------------------------------------------------

### 2. Enroll (`E<num>`)

Assign current pattern to an ID.

**Command examples:**

    E7
    E25

**Responses:** - No finger: `NOF\r\n` - Same pattern already stored:
`ENEX\r\n` - Successfully enrolled: `ENOK\r\n`

**Notes:** - Mapping is stored as: **id_pattern\[id\] = pattern** - One
pattern may be used for multiple IDs (no collision check)

------------------------------------------------------------------------

### 3. Delete (`D<num>`)

Deletes the stored entry for the given ID.

**Command:**

    D7

**Responses:** - ID not found: `DNEX\r\n` - Deleted successfully:
`DELOK\r\n`

------------------------------------------------------------------------

## 📌 EEPROM Layout

Stored as an array:

``` c
eeprom unsigned char id_pattern[128];
```

-   Index = **ID (0--127)**
-   Value = **Pattern (0--127)** or **0xFF** if empty
-   On first run, EEPROM is initialized with `0xFF`

------------------------------------------------------------------------

## 📌 Building (CodeVisionAVR)

This project must be built using **CodeVisionAVR (CVAVR)**.

Steps: 1. Create a new CVAVR project for **ATmega32 @ 8MHz** 2. Add the
provided `.c` file 3. Build project → generate `.hex` 4. Load `.hex`
into ATmega32 inside Proteus

> ⚠️ If you change the clock frequency, you must update `UBRRL` in
> `uart_init()` accordingly.

------------------------------------------------------------------------

## 📌 Testing in Proteus with Virtual Terminal

To test the fake sensor without a master MCU:

1.  Place a **Virtual Terminal** on the schematic
2.  Connect:
    -   ATmega32 **TX (PD1)** → Terminal **RX**
    -   ATmega32 **RX (PD0)** ← Terminal **TX**
    -   Common **GND**
3.  Run simulation
4.  Send commands (`S`, `E7`, `D7`, etc.)

### If the Virtual Terminal window doesn't appear:

In Proteus: - Go to: **Debug → Virtual Terminal** -
OR use shortcut: **Alt + D, then 4** (probably 4)
- OR right‑click the terminal → *Show Instrument*

This will reopen the terminal panel even if it was closed or hidden
behind other windows.

------------------------------------------------------------------------

## 📌 Notes

-   Designed strictly for **simulation/testing** --- not for real
    fingerprint hardware
-   Perfect for:
    -   Mocking fingerprint modules in student projects
    -   Developing main‑MCU code before real hardware is available
    -   UART protocol testing


------------------------------------------------------------------------

Enjoy simulating! 🚀
