# Copilot Instructions for Arduino Monorepo (Arduino IDE 2)

## Scope and Project Boundaries
- Treat this repository as a monorepo where each sub-folder is a self-contained Arduino project.
- Do not suggest importing, sharing, or cross-referencing files between different project/model folders unless explicitly asked.
- Assume all advice must be local to the active project folder.

## Context Awareness (Required)
- Determine the active project from the immediate parent folder of the `.ino` file being edited.
- Use that parent folder name as the primary project context for all code suggestions, wiring guidance, and dependency recommendations.
- If context is ambiguous, ask for the target `.ino` file path before proposing architecture-specific changes.

## Hardware-Specific Behavior (Required)
- If the active folder name indicates a board family (for example: `Uno`, `ESP32`, `Nano_BLE`), tailor all guidance to that board architecture.
- Include board-specific considerations in suggestions:
  - Logic voltage expectations (`5V` vs `3.3V`) and safe peripheral interfacing.
  - Available PWM-capable pins and any board-specific pin mapping caveats.
  - Core/library compatibility for that architecture (AVR, ESP32, mbed, etc.).
  - Resource constraints and strengths (RAM/flash, Wi-Fi/BLE availability, ADC/DAC differences).
- Prefer board-correct APIs and libraries; do not assume APIs are portable across all Arduino cores.

## Coding Standards
- Use modern C++ that is compatible with Arduino IDE 2 toolchains.
- Prefer non-blocking designs:
  - Avoid `delay()` in normal control flow.
  - Recommend `millis()`/state-machine/timer-based scheduling patterns.
- Keep examples production-lean for microcontrollers:
  - Minimize dynamic allocation when avoidable.
  - Use `const`/`constexpr` where appropriate.
  - Keep ISR usage minimal and safe.
- For dependencies, recommend installation via Arduino IDE 2 Library Manager when possible, including exact library names when known.

## Output Expectations
- When giving code, include concise setup notes relevant to the active board (selected board package, required libraries, voltage notes).
- When giving wiring help, include pin labels and highlight any pins that are input-only, boot-strapping, or otherwise constrained on that board.
- If a recommendation depends on uncertain board details, state the assumption explicitly and request confirmation.
