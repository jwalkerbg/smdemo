| Supported Targets | ESP32-S3 |
| ----------------- | -------- |

# Finite State Machine - Component & Example

Finite State Machine component offers FSM driver which can run state machines organized as linked tables. No `switch` operators are used. This approach is good and disciplined implementation with no way to escape from the theoretical definition.

Theoretically FSM is an automat of Mealy: FSM is in one of finite number states, reacts on incoming event by executing a transition and issuing output (transition action). Transitions can have `guards` and states can have `exit` and `entry` actions. This approach offers well organized tracing. The standard `esp_log` module is used for logging. The work of FSM can be seen in real time.

# How to use this example.

## Hardware Required

* A development module with any supported Espressif SOC chip.

The example uses two GPIO's, one for button input - `BUTTON_GPIO` and one for LED output - `LED_GPIO`. They are defined in `proc.c` and can be changed accordingly to the board used or if combined with other examples.

## Software Dependencies.

The example depends on `iot_button` component. It is downloaded into the project automatically, because it is mentioned in `main/idf_component.yml`.

## Build and flash

Enable `CONFIG_SM_EVENT_TYPE_DEFINED_IN_APPLICATION`, `CONFIG_SM_TRACER`, `CONFIG_SM_TRACER_LOSTEVENT` and `CONFIG_SM_TRACER_VERBOSE` for successful build.

Run `idf.py -p PORT flash monitor` to build, flash and monitor the project.

(To exit the serial monitor, type ``Ctrl-]``.)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.

## Console Output

```plain
I (26) boot: ESP-IDF v5.4.1-dirty 2nd stage bootloader
I (27) boot: compile time Jun 20 2025 20:30:41
I (27) boot: Multicore bootloader
I (27) boot: chip revision: v0.2
I (30) boot: efuse block revision: v1.3
I (34) boot.esp32s3: Boot SPI Speed : 80MHz
I (38) boot.esp32s3: SPI Mode       : DIO
I (41) boot.esp32s3: SPI Flash Size : 2MB
I (45) boot: Enabling RNG early entropy source...
I (50) boot: Partition Table:
I (52) boot: ## Label            Usage          Type ST Offset   Length
I (59) boot:  0 nvs              WiFi data        01 02 00009000 00006000
I (65) boot:  1 phy_init         RF data          01 01 0000f000 00001000
I (72) boot:  2 factory          factory app      00 00 00010000 00100000
I (78) boot: End of partition table
I (81) esp_image: segment 0: paddr=00010020 vaddr=3c030020 size=1044ch ( 66636) map
I (101) esp_image: segment 1: paddr=00020474 vaddr=3fc93800 size=02b50h ( 11088) load
I (103) esp_image: segment 2: paddr=00022fcc vaddr=40374000 size=0d04ch ( 53324) load
I (116) esp_image: segment 3: paddr=00030020 vaddr=42000020 size=2212ch (139564) map
I (141) esp_image: segment 4: paddr=00052154 vaddr=4038104c size=02770h ( 10096) load
I (144) esp_image: segment 5: paddr=000548cc vaddr=600fe100 size=0001ch (    28) load
I (151) boot: Loaded app from partition at offset 0x10000
I (151) boot: Disabling RNG early entropy source...
I (166) cpu_start: Multicore app
I (175) cpu_start: Pro cpu start user code
I (175) cpu_start: cpu freq: 160000000 Hz
I (176) app_init: Application information:
I (176) app_init: Project name:     smdemo_espidf
I (180) app_init: App version:      1.0.0
I (184) app_init: Compile time:     Jun 20 2025 20:26:41
I (189) app_init: ELF file SHA256:  f40ccf770...
I (193) app_init: ESP-IDF:          v5.4.1-dirty
I (197) efuse_init: Min chip rev:     v0.0
I (201) efuse_init: Max chip rev:     v0.99
I (205) efuse_init: Chip rev:         v0.2
I (209) heap_init: Initializing. RAM available for dynamic allocation:
I (215) heap_init: At 3FC96D30 len 000529E0 (330 KiB): RAM
I (221) heap_init: At 3FCE9710 len 00005724 (21 KiB): RAM
I (226) heap_init: At 3FCF0000 len 00008000 (32 KiB): DRAM
I (231) heap_init: At 600FE11C len 00001ECC (7 KiB): RTCRAM
I (237) spi_flash: detected chip: generic
I (240) spi_flash: flash io: dio
W (243) spi_flash: Detected size(16384k) larger than the size in the binary image header(2048k). Using the size in the binary image header.
I (255) sleep_gpio: Configure to isolate all GPIO pins in sleep state
I (262) sleep_gpio: Enable automatic switching of GPIO sleep configuration
I (268) main_task: Started on CPU0
I (278) main_task: Calling app_main()
I (278) APP: Application version: 1.0.0
I (288) ANVS: nvs_commit_task entered
I (288) ANVS: appstore exists
I (288) ANVS: key 'appmark', type '2', value '1'
I (288) ANVS: key 'opmode', type '2', value '3'
I (288) gpio: GPIO[12]| InputEn: 1| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:0
I (298) button: IoT Button Version: 4.1.3
I (308) gpio: GPIO[13]| InputEn: 0| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:0
I (318) SM: Created SM event loop
I (318) SM: SM event loop registered
I (318) PROCESS: Starting P1
I (318) main_task: Returned from app_main()
I (328) PROCESS: Trace context 0
I (328) PROCESS: P1a0 executed
I (328) PROCESS: ID=0001, S1=sP1_START, S2=sP1_RESOLVE, Event=evP1Start, Action=0 permitted
I (338) PROCESS: Trace context 1
I (338) PROCESS: Trace context 0
I (348) PROCESS: P1a4 executed
I (348) PROCESS: ID=0001, S1=sP1_RESOLVE, S2=sP1_MANUAL, Event=evP1Trigger4, Action=4 permitted
I (358) PROCESS: Trace context 1
 ... later ...
I (1738808) proc: BUTTON_SINGLE_CLICK
I (1738808) PROCESS: Trace context 0
I (1738808) PROCESS: P1a10 executed
I (1738818) ANVS: NVS data committed successfully.
I (1738818) PROCESS: ID=0001, S1=sP1_MANUAL, S2=sP1_TEST, Event=evButtonSingleClick, Action=10 permitted
I (1738818) PROCESS: Trace context 1
I (1739168) proc: BUTTON_SINGLE_CLICK
I (1739168) PROCESS: Trace context 0
I (1739168) PROCESS: P1a6 executed
I (1739178) ANVS: NVS data committed successfully.
I (1739178) PROCESS: ID=0001, S1=sP1_TEST, S2=sP1_STANDBY, Event=evButtonSingleClick, Action=6 permitted
I (1739178) PROCESS: Trace context 1
```

## Notes

The example uses `nvs` to safe current state in nvs so as after restart it to be restored. This happens by storing the value of operative mode variable. See `anvs.h` and `anvs.c`. This module uses a thread executed by CPU1 for storing data in nvs. This way the  main program is run without interruption on CPU0.

Operative mode is one of these defined in `device_modes_t` in `commondefs.h`. `OP_MODE_STANDBY` is chosen initially. Then operative modes are changed in round ring by pressing the button. The new state is immediately stored in nvs.

The example uses one LED which blinks with different period in the different states. This is enough to see that pressing a button leads to a change in the application and this change is controlled exclusively by the FSM.

So we have:

* **Input device**: a button, that delivers user interaction. The input event is `evButtonSingleClick`. It is generated in the registered callback function `button_event_cb` in `proc.c`. It is called by the component `iot_button`. See the code in `proc.c` about how to create a button object and how to register a callback function for given button event.
* **FSM**. The FSM driver is implemented in the component `state_machine`. The FSM data is in `process.h` and `process.c`. The graphical diagram of the FSM is in `diagrams.drawio`. It is interesting to see how FSM handles the initial initialization in `P0a0` and how determines which state to go to. Then the loop between the states is executed by pressing the button and generating `evButtonSingleClick`
* **Output device**: a LED, which blinks. The blink function is implemented using `esp_timer`. The action `P1a6`, `P1a7`, `P1a8`, `P1a9`, `P1a10` are transition actions. They are used to change blinking period.

There is no even single `if` operator in `process.c`. All the logic is in the FSM data tables. Actions are just actions and nothing else. They work assuming that are called in the right moment and context. The input device does need to know that a LED is driven after button events. It just informs the system (the FSM) that a button event has happened. The FSM decides what will happen next. And the actions (doers) do it.

Please see the graphic diagram to take shape of the logic.

## Future exercises

Add second button, par example on GPIO14. Add a callback function that reacts to its Single clock event. Add new FSM event to the `EVENT_LIST` for that button event. Then add transitions in the FSM data to rotate the operative states in opposite direction.
