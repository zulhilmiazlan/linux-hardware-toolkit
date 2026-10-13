# 001: GPU sensors return "Device or resource busy"

**Date:** 2026-10-13  
**System:** Ryzen 7 7800X3D, Radeon RX 9070 XT, MSI B650M GAMING PLUS WIFI, CachyOS, kernel 7.2.9

**Summary**

GPU power-cap sensor reads failed with `Device or resource busy` when the RX 9070 XT was runtime-suspended. A controlled A/B/A test confirmed that the reads succeeded when runtime power management was forced to keep the GPU active and failed again after restoring automatic power management and allowing the GPU to suspend. The cause of the suspension itself, suspected to be related to the monitor being off, remains untested.

## Symptom

During the initial hardware telemetry capture, reading the GPU power-cap files failed.

**Command, run in fish:**

```fish
grep . /sys/class/drm/card*/device/hwmon/hwmon*/power1_cap* >> $out
```

**Output:**

```text
grep: /sys/class/drm/card1/device/hwmon/hwmon2/power1_cap: Device or resource busy
grep: /sys/class/drm/card1/device/hwmon/hwmon2/power1_cap_default: Device or resource busy
grep: /sys/class/drm/card1/device/hwmon/hwmon2/power1_cap_max: Device or resource busy
grep: /sys/class/drm/card1/device/hwmon/hwmon2/power1_cap_min: Device or resource busy
```

The command redirected standard output to `$out`, but not standard error. Consequently, the error messages appeared in the terminal rather than in the output file.

## Hypotheses

1. **The sensor files or GPU driver were unavailable or malfunctioning.** The failed reads were consistent with this possibility, but did not establish why access failed.
2. **`card1` was the integrated GPU and had entered a low-power state.** This was an inference made after seeing ambiguous runtime-status output, not an established fact.
3. **GPU runtime power management was affecting sensor availability.** If the discrete GPU was suspended, its telemetry interface might not be readable until the device became active.

## Investigation

### 1. Check runtime status, initially

I first ran:

```fish
cat /sys/class/drm/card*/device/power/runtime_status
```

The output contained an `active` line, a `suspended` line, and `unsupported` entries for connectors. Because the output did not identify the source path beside each value, I could not determine which card was suspended.

At this point, I inferred that `card1` might be the integrated GPU in a low-power state. This inference still needed verification.

### 2. Identify the GPU mapping

I checked PCI device identities and the DRM card mapping:

```fish
lspci -nn | grep -iE 'vga|display'
command ls -l /sys/class/drm/ | grep -E 'card[0-9]+ '
```

The mapping showed that `card0` was the Radeon 610M integrated GPU at PCI address `16:00.0`, while `card1` was the RX 9070 XT at `03:00.0`.

This overturned my initial inference: **`card1` was the discrete GPU, not the integrated GPU.** The monitor was also connected to the discrete GPU, but that was a separate observation rather than the evidence used to establish the card mapping.

### 3. Confirm runtime status with labelled paths

I queried the cards individually, preserving their paths in the output:

```fish
grep . /sys/class/drm/card{0,1}/device/power/runtime_status
```

This showed that the RX 9070 XT was runtime-suspended. The labelled output resolved the ambiguity from step 1.

During the investigation, sensor reads for power, temperatures, frequency and power cap failed while the GPU was suspended. The `power1_input` file was absent, while `power1_average` existed. Repeated `power1_cap` reads failed consistently.

### 4. Force the GPU active and test sensor reads

I changed the runtime power-control setting to `on`, waited briefly, and read the runtime status and available sensors:

```fish
cat /sys/class/drm/card1/device/power/control
echo on | sudo tee /sys/class/drm/card1/device/power/control
sleep 2
grep . /sys/class/drm/card1/device/power/runtime_status /sys/class/drm/card1/device/hwmon/hwmon2/{power1_average,power1_cap,temp1_input,temp2_input,freq1_input}
echo auto | sudo tee /sys/class/drm/card1/device/power/control
```

**Observed output:**

```text
auto
on
/sys/class/drm/card1/device/power/runtime_status:active
/sys/class/drm/card1/device/hwmon/hwmon2/power1_average:22000000
/sys/class/drm/card1/device/hwmon/hwmon2/power1_cap:304000000
/sys/class/drm/card1/device/hwmon/hwmon2/temp1_input:35000
/sys/class/drm/card1/device/hwmon/hwmon2/temp2_input:37000
/sys/class/drm/card1/device/hwmon/hwmon2/freq1_input:4000000
auto
```

With `power/control` set to `on`, the GPU became active and the sensor reads succeeded. I then restored `auto`.

The raw readings correspond to approximately 22 W average power, a 304 W power cap, and temperatures of 35°C and 37°C. The temperature labels have not yet been verified. The reported frequency of `4000000` also needs further investigation under load.

### 5. Restore automatic power management and retest

To test whether the failure returned after the GPU suspended, I restored automatic power management, waited 30 seconds, and read the runtime status and power cap:

```fish
echo auto | sudo tee /sys/class/drm/card1/device/power/control
sleep 30
grep . /sys/class/drm/card1/device/power/runtime_status /sys/class/drm/card1/device/hwmon/hwmon2/power1_cap
```

**Observed output:**

```text
[sudo] password for hilmi:
auto
/sys/class/drm/card1/device/power/runtime_status:suspended
grep: /sys/class/drm/card1/device/hwmon/hwmon2/power1_cap: Device or resource busy
```

The power-cap read failed again after the GPU returned to the suspended state.

Together, the tests form an **A/B/A sequence**:

- **A:** Automatic power management; GPU suspended; power-cap read fails.
- **B:** Runtime power control forced to `on`; GPU active; power-cap read succeeds.
- **A again:** Automatic power management restored; GPU suspended after 30 seconds; power-cap read fails again.

This makes coincidence substantially less likely and provides strong evidence linking the sensor-read failure to the GPU's runtime power state.

## Root cause

**Confirmed:** Sensor reads for the RX 9070 XT fail with `Device or resource busy` while the GPU is runtime-suspended and succeed when it is forced active. The A/B/A test demonstrated that the failure returned after automatic power management was restored and the GPU suspended again.

**Not yet confirmed:** What triggers the GPU to suspend. The monitor being off is suspected, but this has not been tested independently.

## Fix / design consequence

For the telemetry collector:

- Record the GPU's `runtime_status` alongside sensor readings.
- Handle `Device or resource busy` as an unavailable reading, log the error, and continue collecting other metrics.
- Never substitute zero for a failed read. Zero is a measurement; an unavailable value is a collection failure.
- Distinguish missing sensor files from files that exist but cannot currently be read.
- Discover available sensor files and inspect their labels rather than assuming every GPU exposes the same set.
- Decide whether measurement runs should force `power/control=on` for consistency, and record the setting either way. Keeping the GPU active may improve repeatability, but it also changes the power state and potentially the baseline being measured.

## Open items

- Test whether turning the monitor on and off reliably changes the RX 9070 XT's runtime status and sensor availability.
- Inspect `temp*_label` to identify what the two temperature readings represent.
- Check the reported frequency under GPU load and verify its units and interpretation.
- Investigate the boot message `Unsupported screen format RA24`. Its relationship to this issue is not established.

## What I learned

- Unlabelled command output can hide which device a result belongs to. Preserve paths when comparing multiple devices.
- An initial inference should remain a hypothesis until device identity is verified.
- An A/B/A test is stronger than a one-way test because it checks whether the original behaviour returns when the original condition is restored.
- Telemetry software must distinguish a real zero from an unavailable or failed reading.
- A workaround can change the system being measured. Whether to force a device active is a measurement-design decision, not automatically the correct permanent fix.