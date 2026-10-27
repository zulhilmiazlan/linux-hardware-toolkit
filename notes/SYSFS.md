# sysfs reference: what each file is and its units

| What | Path | Units | Notes |
|---|---|---|---|
| CPU package energy | `/sys/class/powercap/intel-rapl:*/energy_uj` (zone named `package-0`) | µJ | Root-only on the laptop due to power side-channel protections associated with the *Platypus* attack. Read twice and calculate the delta. The laptop's range is 262,143 J, wrapping after approximately 18 hours at 4 W or 48 minutes at 90 W. Desktop range still needs checking. |
| Energy counter range | `/sys/class/powercap/intel-rapl:*/max_energy_range_uj` | µJ | The counter's wrap point. Use this value when calculating energy deltas across a wrap. |
| CPU temperature | `/sys/class/hwmon/hwmon*/temp*_input` (device named `coretemp` / `k10temp`) | millidegrees Celsius (°C × 1000) | Divide by 1000 to get °C. Find the sensor by its `name` file, not its `hwmon` number. |
| Core clock | `/sys/devices/system/cpu/cpu*/cpufreq/scaling_cur_freq` | kHz | Snapshots of the same core read 1108 MHz and 3602 MHz moments apart because frequency changes rapidly. `perf stat -e cycles -a` averaged only about 0.19 GHz per CPU because idle cores sleep in C-states and count no cycles. Neither value alone tells the whole performance story. |
| CPU frequency driver | `/sys/devices/system/cpu/cpufreq/policy*/scaling_driver` | text | On the laptop, the driver is `intel_pstate`. |
| Energy-performance preference (EPP) | `/sys/devices/system/cpu/cpufreq/policy*/energy_performance_preference` | text | On the laptop, the value is `balance_performance`. With `intel_pstate`, the `powersave` governor can still boost frequency; it does not mean the CPU is locked to a low clock. |
| GPU power | `/sys/class/hwmon/hwmon*/power1_average` (device named `amdgpu`) | µW | On the RX 9070 XT, this is the available power-reading file; there is no `power1_input`. Sensor reads returned `Device or resource busy` (`EBUSY`) while the GPU was runtime-suspended and succeeded when forced active. |
| GPU power cap | `/sys/class/hwmon/hwmon*/power1_cap` (device named `amdgpu`) | µW | The RX 9070 XT reported a cap of 304 W. This is the configured limit, not a measurement of current power draw. |
| GPU temperature | `/sys/class/hwmon/hwmon*/temp*_input` (device named `amdgpu`) | millidegrees Celsius (°C × 1000) | Divide by 1000 to get °C. Check available `temp*_label` files where present, and identify the device by its `name`. |
| GPU runtime power state | `/sys/bus/pci/devices/<PCI-address>/power/runtime_status` | text | Map the PCI address to the GPU before reading it. Unlabelled `card*` output was ambiguous because it included other devices and connectors. |
| Runtime power policy | `/sys/bus/pci/devices/<PCI-address>/power/control` | text | `auto` allows runtime power management; `on` prevents runtime suspension. Setting it to `on` confirmed the GPU sensor failure was related to suspension, but the trigger for suspension remains unconfirmed. |
| CPU frequency policy | `/sys/devices/system/cpu/cpu*/cpufreq/scaling_governor` | text | Shows the selected governor, not the instantaneous clock. Read alongside `scaling_cur_freq` when investigating frequency behaviour. |

## Measurement reminders

- To estimate average power from an energy counter, calculate the energy delta in joules and divide by elapsed seconds: watts = joules / seconds.
- Account for counter wrap using `max_energy_range_uj`; a simple subtraction can produce the wrong result if the counter wraps between readings.
- Discover sensor paths at runtime and identify devices by their `name` files rather than hardcoding `hwmon` indices.
- The laptop's RAPL wrap estimates assume constant power consumption at the stated rates. Actual wrap time depends on the measured power draw.
