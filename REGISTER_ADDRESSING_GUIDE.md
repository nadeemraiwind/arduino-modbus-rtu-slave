# Register Addressing Guide {#register_addressing_guide}

@page register_addressing_guide Register Addressing Guide

This guide maps user-facing Modicon register numbers to zero-based addresses used by most Modbus master tools.

## Why This Page Exists

Most support issues come from off-by-one addressing assumptions.

- Modicon notation is 1-based (for example 40001).
- Protocol payload addresses are zero-based (for example start address 0).

## Register Map (Modicon vs Zero-Based)

| Domain | Modicon Range | Zero-Based Start..End | Typical Function Code |
| ------ | ------------- | --------------------- | --------------------- |
| Coils (0x) | 1..2000 | 0..1999 | FC01/FC05/FC15 |
| Discrete Inputs (1x) | 10001..12000 | 0..1999 | FC02 |
| Input Registers (3x) | 30001..30125 | 0..124 | FC04 |
| Holding Registers (4x) | 40001..40125 | 0..124 | FC03/FC06/FC16 |

## Common Conversions

- 40001 -> address 0
- 40010 -> address 9
- 40050 -> address 49
- 30001 -> address 0
- Coil 1 -> address 0

## Practical Rule

When your tool asks for Start Address, subtract the Modicon base:

- 4x: start = modicon - 40001
- 3x: start = modicon - 30001
- 1x: start = modicon - 10001
- 0x: start = modicon - 1
