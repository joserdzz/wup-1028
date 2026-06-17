# WUP-028 Pico 1000 Hz Bridge

Firmware para Raspberry Pi Pico RP2040 que expone a la PC un dispositivo USB `057E:0337`
`WUP-028` y usa un segundo puerto USB host por PIO en `GP2/GP3` para hablar con el
adaptador físico de controles GameCube.

## Cableado

- USB-A `D+` a `GP2`
- USB-A `D-` a `GP3`
- USB-A `V+` a `VBUS`
- USB-A `GND` a `GND`
- Pico MicroUSB a la PC

## Compilar

```powershell
cmake -S . -B build-fw -G Ninja
cmake --build build-fw
```

El UF2 queda en:

```text
build-fw/wup_1028_oc.uf2
```

## Notas

- El lado PC anuncia endpoints interrupt IN/OUT de 37 bytes con intervalo de 1 ms.
- El comando `0x13` inicializa el adaptador físico.
- Los paquetes de entrada del adaptador se reenvían a la PC sin remapear botones.
- Los paquetes OUT de la PC, incluyendo rumble `0x11`, se reenvían al adaptador físico.
