# N-Body Solar System Simulator

> A pure C++17 project implementing a numerically invariant integrator ("kick-drift-kick" leapfrog) for an N-body model of the Solar System.

## 1. Quick Start

```bash
# Clone or unpack the project and enter its root directory
make               # Build the main binary bin/solver
make test          # Build and run the test binary bin/test

# Example of a typical run
make run ARGS="data/ 15.08.2025 1e-9 86400"
#              └───  ──────────  ──── ─────
#              │       │         │     │
#              │       │         └──────── initial time step (dt_init) in seconds
#              │       └────────────────── relative error tolerance ε_rel
#              └────────────────────────── data directory for bodies
```

### Debug Mode

```bash
make debugrun ARGS="data/ 14.02.2026 1e-9 43200"
# Adds -g and -DDEBUG flags, prints detailed diagnostics every 100 steps
```

## 2. Dependencies

* **g++ 9+** with C++17 support
* **make**
* On Windows: build in MSYS2/MinGW or create a project in Visual Studio and include sources/headers.

## 3. Makefile Targets

| Target           | Description                                                                 |
| ---------------- | --------------------------------------------------------------------------- |
| `make`           | Build the main binary `bin/solver`.                                         |
| `make test`      | Build and run the test binary `bin/test` with parameters from `ARGS`.       |
| `make run`       | Build (if needed) and run `bin/solver` with parameters from `ARGS`.         |
| `make debugrun`  | Build with debugging flags (`-g -DDEBUG`) and run `bin/solver` with `ARGS`. |
| `make debugtest` | Build test with debugging flags and run `bin/test` with `ARGS`.             |
| `make clean`     | Remove all object files and binaries in `build/` and `bin/`.                |

`ARGS` format:

```
ARGS="<data_dir> <DD.MM.YYYY> <epsrel> <dt_init> [PlanetName]"
```

* `<data_dir>`: directory with JPL HORIZONS data files
* `<DD.MM.YYYY>`: target integration date (≥ epoch)
* `<epsrel>`: relative tolerance for invariants (energy, momentum)
* `<dt_init>`: initial time step in seconds
* `[PlanetName]`: (test only) the planet name for bin/test (default: Earth)

## 4. Directory Structure

```
project/
├── Makefile          # build and test rules
├── include/          # header files (*.h, *.hpp)
├── src/              # source files (*.cpp)
│   └── test.cpp      # minimal functional test (1 year orbit)
├── bin/              # built executables
├── build/            # intermediate object files and dependencies
└── data/             # example input data directory
```

## 5. Input File Format

* Each file is an export from **JPL HORIZONS**.
* Must include lines:

  * `A.D. YYYY-Mon-DD ...` — epoch date (time zero)
  * `Mass (kg) = ...` — body mass
  * `X = ... Y = ... Z = ...` — position in km
  * `VX= ... VY= ... VZ= ...` — velocity in km/s
* Body name is taken from `Revised ... <Name>` line.

## 6. Running the Solver

```
./bin/solver <data_dir> <DD.MM.YYYY> <epsrel> <dt_init>
```

**Workflow:**

1. Load bodies from `<data_dir>`.
2. Parse epoch date (`t=0`).
3. Convert target date to seconds since epoch.
4. Build arrays of masses, positions, velocities.
5. Integrate using leapfrog (kick-drift-kick) with adaptive `dt`:

   * If invariants (energy, momentum, angular momentum) exceed `epsrel`, halve `dt`.
   * Otherwise accept step and (optionally) double `dt`, limited by remaining time.
6. Output final positions and velocities.

With `DEBUG` defined, prints step-by-step invariant diagnostics every 100 steps and at the end.

## 7. Planet Offset Test

`make test` or `make debugtest` builds and runs **bin/test**:

* Integrates exactly one year for a chosen planet (`PlanetName`, default Earth).
* Prints linear displacement and relative energy change.

## 8. Units

* Distance: **km**
* Mass: **kg**
* Time: **s**
* Gravitational constant: `G = 6.67430e-20 km³·kg⁻¹·s⁻²`

## 9. Code Architecture

| File/Class               | Role                                                   |
| ------------------------ | ------------------------------------------------------ |
| `Vec3`                   | 3D vector with arithmetic, dot/cross products          |
| `SolarBody`              | Mass, position, velocity, and name of one body         |
| `SolarSystem`            | Container of bodies; parsing, computing energy/momenta |
| `Integrator`             | Leapfrog (kick-drift-kick) integrator with adaptive dt |
| `Utils::date_to_seconds` | Convert `DD.MM.YYYY` to seconds since epoch            |

## 10. Program Logic (No Code)

Described as functional blocks:

1. **Data Parsing**

   * Read input files. Extract masses, positions, velocities, names, epoch.
   * Populate arrays: `m[i]`, `q[i]`, `v[i]`.

2. **Integrator Initialization**

   * Read target date, `epsrel`, `dt_init`.
   * Convert date to `t_end` seconds.
   * Set `t = 0`.

3. **Compute Accelerations**

   * For each body `i`:
     `a[i] = -G * Σ_{j != i} m[j] * (q[i] - q[j]) / |q[i] - q[j]|^3`.

4. **Integration Loop (kick-drift-kick)**

   * While `t < t_end`:

     1. **Kick:** `v_half[i] = v[i] + a[i] * (dt/2)`
     2. **Drift:** `q_new[i] = q[i] + v_half[i] * dt`
     3. **Recompute a:** `a_new = compute_accelerations(q_new)`
     4. **Kick:** `v_new[i] = v_half[i] + a_new[i] * (dt/2)`

5. **Adaptive Step & Invariants**

   * Compute new invariants `E_new, P_new, L_new`.
   * If any relative change > `epsrel`, `dt /= 2` and repeat.
   * Else accept: `q = q_new`, `v = v_new`, `t += dt`, optionally `dt *= 2`.

6. **Completion**

   * When `t >= t_end`, output final `q[i]`, `v[i]`.
   * Optionally log invariants history.
