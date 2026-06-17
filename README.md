# WUP-1028

Firmware para Raspberry Pi Pico que overclockea por USB-A el adaptador fisico de Controles de Nintendo GameCube, pensado específicamente para los adaptadores que no se overclockean por Software.
## Cableado

- USB-A `D+` a `GP2`
- USB-A `D-` a `GP3`
- USB-A `V+` a `VBUS`
- USB-A `GND` a `GND`

- El lado PC anuncia endpoints interrupt IN/OUT de 37 bytes con intervalo de 1 ms.
- Los paquetes de entrada del adaptador se reenvían a la PC sin remapear botones.
