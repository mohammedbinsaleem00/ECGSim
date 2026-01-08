ECG Simulator Frontend
======================

This is a Tkinter-based frontend that wraps the existing `ecg_uart_uploader.py` uploader.

Features
--------
- **HR Input**: Enter a heart-rate (bpm) and generate ECG data
- **Configure Port**: Choose COM port and set benchmark polling frequency
- **Download**: Transfer ECG data to device with real-time green progress bar
- **Benchmark Monitor**: Live plot of average trigger times from the device
  - Periodically sends `GetAvgTriggerTime\r` command
  - Parses response `"Avg: %lu ms\n"` and plots in real-time
  - Configurable polling frequency from configuration dialog

Usage
-----

Run the GUI:

```bash
python ecg_sim_frontend.py
```

Configuration
-------------
- Click **Configure** to open settings
  - Select COM port (auto-detected)
  - Set benchmark polling frequency in seconds (default 1.0s)
- Click **Download** to send ECG data (requires COM port configured)
- Click **Benchmark** to start live monitoring
  - Plot appears in the expanded area at the top
  - Click again to stop monitoring

Notes
-----
- This module imports and uses `ECGUARTUploader` and helper functions from `ecg_uart_uploader.py`. That file is not modified.
- `neurokit2` is optional — if not installed a simple synthetic waveform is used.
- Benchmark runs in a separate thread with no impact on download operation.
