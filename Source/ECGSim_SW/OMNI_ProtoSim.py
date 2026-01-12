"""
ECG Simulator Frontend

Simple Tkinter GUI wrapper around `ecg_uart_uploader.ECGUARTUploader`.

Features:
- HR input box
- `Configure` button to choose COM port
- `Download` button to generate ECG and send to device
- Progress bar showing real-time send progress

This file does NOT modify `ecg_uart_uploader.py`; it imports and uses its classes
and helper functions.
"""
import threading
import time
import sys
import io
import contextlib
import re
import tkinter as tk
from tkinter import ttk, messagebox
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure

try:
    import neurokit2 as nk
    HAS_NK = True
except Exception:
    HAS_NK = False

import serial.tools.list_ports

from ecg_uart_uploader import ECGUARTUploader, normalize_ecg_endpoints, ecg_to_dac_3mVpp, set_dac_pp_voltage


class ConfigWindow(tk.Toplevel):
    def __init__(self, master, current_port, on_select, benchmark_freq=1.0):
        super().__init__(master)
        self.title("Configuration")
        self.resizable(False, False)
        self.on_select = on_select

        # COM Port
        ttk.Label(self, text="Available COM ports:").grid(row=0, column=0, sticky="w", padx=8, pady=8)
        ports = self._list_ports()
        self.port_var = tk.StringVar(value=current_port or (ports[0] if ports else ""))
        self.combo = ttk.Combobox(self, values=ports, textvariable=self.port_var, state="readonly")
        self.combo.grid(row=1, column=0, padx=8, pady=(0, 8))

        # Benchmark frequency
        ttk.Label(self, text="Benchmark poll freq (s):").grid(row=2, column=0, sticky="w", padx=8, pady=(8, 0))
        self.freq_var = tk.StringVar(value=str(benchmark_freq))
        freq_entry = ttk.Entry(self, textvariable=self.freq_var, width=10)
        freq_entry.grid(row=3, column=0, sticky="w", padx=8, pady=(0, 8))

        btn_frame = ttk.Frame(self)
        btn_frame.grid(row=4, column=0, sticky="e", padx=8, pady=8)
        ttk.Button(btn_frame, text="OK", command=self._ok).grid(row=0, column=0, padx=4)
        ttk.Button(btn_frame, text="Refresh", command=self._refresh).grid(row=0, column=1, padx=4)
        ttk.Button(btn_frame, text="Cancel", command=self.destroy).grid(row=0, column=2, padx=4)

    def _list_ports(self):
        return [p.device for p in serial.tools.list_ports.comports()]

    def _refresh(self):
        ports = self._list_ports()
        self.combo['values'] = ports
        if ports:
            self.port_var.set(ports[0])

    def _ok(self):
        port = self.port_var.get()
        if not port:
            messagebox.showwarning("No port", "Please select a COM port")
            return
        try:
            freq = float(self.freq_var.get())
            if freq <= 0:
                raise ValueError()
        except Exception:
            messagebox.showerror("Invalid frequency", "Benchmark frequency must be > 0")
            return
        self.on_select(port, freq)
        self.destroy()


class App:
    def __init__(self, root):
        self.root = root
        root.title("OMNI ProtoSim V0.1")

        # State
        self.selected_port = None
        self.baudrate = 115200
        self.uploader = None
        self.sending = False
        self.benchmark_active = False
        self.benchmark_freq = 1.0
        self.benchmark_thread = None
        self.benchmark_data = []  # list of (timestamp, value) tuples
        self.benchmark_stop = threading.Event()
        self.benchmark_lock = threading.Lock()

        # Layout compartments
        main = ttk.Frame(root, padding=10)
        main.pack(fill="both", expand=True)

        # Top section for benchmark plot (initially hidden)
        self.plot_frame = ttk.LabelFrame(main, text="Benchmark", padding=8)
        self.plot_frame.pack(fill="both", expand=True, pady=(0, 8))
        self.plot_canvas = None  # Will be created if benchmark starts
        self.fig = None
        self.ax = None
        self._init_plot()

        # Bottom section: left controls, right status
        bottom = ttk.Frame(main)
        bottom.pack(fill="both", expand=True)

        left = ttk.LabelFrame(bottom, text="Controls", padding=8)
        left.pack(side="left", fill="y", padx=(0, 8))

        right = ttk.LabelFrame(bottom, text="Status", padding=8)
        right.pack(side="right", fill="both", expand=True)

        # Controls
        ttk.Label(left, text="Heart Rate (bpm):").grid(row=0, column=0, sticky="w")
        self.hr_var = tk.StringVar(value="60")
        self.hr_entry = ttk.Entry(left, textvariable=self.hr_var, width=10)
        self.hr_entry.grid(row=1, column=0, pady=(0, 6))

        self.port_label = ttk.Label(left, text="COM: (not set)")
        self.port_label.grid(row=2, column=0, pady=(6, 6))

        btn_cfg = ttk.Button(left, text="Configure", command=self.open_config)
        btn_cfg.grid(row=3, column=0, sticky="ew", pady=(0, 6))

        btn_download = ttk.Button(left, text="Download", command=self.start_download)
        btn_download.grid(row=4, column=0, sticky="ew", pady=(0, 6))

        self.btn_benchmark = ttk.Button(left, text="Benchmark", command=self.toggle_benchmark)
        self.btn_benchmark.grid(row=5, column=0, sticky="ew")

        # Status area + progress
        self.log = tk.Text(right, height=12, wrap="word", state="disabled")
        self.log.pack(fill="both", expand=True)

        pb_frame = ttk.Frame(right)
        pb_frame.pack(fill="x", pady=(8, 0))
        self.progress = ttk.Progressbar(pb_frame, mode="determinate", style="Green.Horizontal.TProgressbar")
        self.progress.pack(fill="x", side="left", expand=True)
        self.progress_label = ttk.Label(pb_frame, text="Idle", width=14)
        self.progress_label.pack(side="right", padx=(8, 0))

    def log_msg(self, s):
        def _append():
            self.log.configure(state="normal")
            self.log.insert("end", s + "\n")
            self.log.see("end")
            self.log.configure(state="disabled")
        self.root.after(0, _append)

    @contextlib.contextmanager
    def _suppress_uploader_prints(self):
        """Temporarily suppress stdout/stderr from the uploader module prints."""
        old_stdout = sys.stdout
        old_stderr = sys.stderr
        sys.stdout = io.StringIO()
        sys.stderr = io.StringIO()
        try:
            yield
        finally:
            sys.stdout = old_stdout
            sys.stderr = old_stderr

    def _silent_call(self, func, *args, **kwargs):
        with self._suppress_uploader_prints():
            return func(*args, **kwargs)

    def _init_plot(self):
        """Initialize matplotlib figure for benchmark plotting."""
        self.fig = Figure(figsize=(8, 3), dpi=100)
        self.ax = self.fig.add_subplot(111)
        self.ax.set_xlabel("Time (s)")
        self.ax.set_ylabel("Avg Trigger Time (ms)")
        self.ax.set_title("Benchmark Monitor")
        self.ax.grid(True)

        self.plot_canvas = FigureCanvasTkAgg(self.fig, master=self.plot_frame)
        self.plot_canvas.get_tk_widget().pack(fill="both", expand=True)

    def _update_plot(self):
        """Redraw the plot with current benchmark data (last 5 seconds only)."""
        if self.plot_canvas is None or not self.benchmark_data:
            return
        
        with self.benchmark_lock:
            data_copy = self.benchmark_data.copy()
        
        if not data_copy:
            return
        
        # Keep only data from the last 5 seconds
        current_time = time.time()
        cutoff_time = current_time - 5.0
        filtered_data = [d for d in data_copy if d[0] >= cutoff_time]
        
        if not filtered_data:
            return
        
        t0 = filtered_data[0][0]
        times = [(d[0] - t0) for d in filtered_data]
        values = [d[1] for d in filtered_data]
        
        self.ax.clear()
        self.ax.plot(times, values, 'b-o', markersize=4)
        self.ax.set_xlabel("Time (s)")
        self.ax.set_ylabel("Avg Trigger Time (ms)")
        self.ax.set_title("Benchmark Monitor (Last 5s)")
        self.ax.grid(True)
        self.fig.tight_layout()
        self.plot_canvas.draw()

    def toggle_benchmark(self):
        """Start or stop benchmark monitoring."""
        if not self.selected_port:
            messagebox.showerror("No COM port", "Please configure a COM port first")
            return
        
        if self.benchmark_active:
            self.stop_benchmark()
        else:
            self.start_benchmark()

    def start_benchmark(self):
        """Start the benchmark monitoring thread."""
        self.benchmark_active = True
        self.benchmark_data = []
        self.benchmark_stop.clear()
        self.btn_benchmark.config(text="Stop Benchmark")
        self.log_msg("Benchmark started")
        
        self.benchmark_thread = threading.Thread(target=self._benchmark_thread, daemon=True)
        self.benchmark_thread.start()

    def stop_benchmark(self):
        """Stop the benchmark monitoring thread."""
        self.benchmark_active = False
        self.benchmark_stop.set()
        self.btn_benchmark.config(text="Benchmark")
        self.log_msg("Benchmark stopped")
        if self.benchmark_thread:
            self.benchmark_thread.join(timeout=2.0)

    def _benchmark_thread(self):
        """Background thread that periodically sends GetAvgTriggerTime and updates plot."""
        # Create a separate connection for benchmark (don't use uploader)
        try:
            bench_uploader = ECGUARTUploader(port=self.selected_port, baudrate=self.baudrate, timeout=1.0)
            if not self._silent_call(bench_uploader.connect):
                self.log_msg("Failed to connect for benchmark")
                self.benchmark_active = False
                return
            
            while not self.benchmark_stop.is_set():
                try:
                    # Send command
                    cmd = "GetAvgTriggerTime\r"
                    self._silent_call(bench_uploader.send_command, cmd)
                    
                    # Read response: "Avg: %lu ms\n"
                    resp = self._silent_call(bench_uploader.read_response, wait_for="ms", max_wait=1.5)
                    
                    if resp:
                        # Parse "Avg: <value> ms"
                        match = re.search(r'Avg:\s*(\d+)\s*ms', resp)
                        if match:
                            value = int(match.group(1))
                            t = time.time()
                            
                            with self.benchmark_lock:
                                self.benchmark_data.append((t, value))
                            
                            # Update plot in main thread
                            self.root.after(0, self._update_plot)
                    
                    # Wait for the configured frequency
                    time.sleep(self.benchmark_freq)
                
                except Exception as e:
                    self.log_msg(f"Benchmark error: {e}")
                    time.sleep(0.5)
            
            self._silent_call(bench_uploader.disconnect)
        
        except Exception as e:
            self.log_msg(f"Benchmark thread error: {e}")
        finally:
            self.benchmark_active = False

    def open_config(self):
        def on_select(port, freq):
            self.selected_port = port
            self.benchmark_freq = freq
            self.port_label.config(text=f"COM: {port}")
            self.log_msg(f"Selected port: {port}, benchmark freq: {freq}s")

        ConfigWindow(self.root, self.selected_port, on_select, self.benchmark_freq)

    def start_download(self):
        if self.sending:
            messagebox.showinfo("Busy", "A download is already in progress")
            return

        try:
            hr = float(self.hr_var.get())
            if hr <= 30:
                raise ValueError()
        except Exception:
            messagebox.showerror("Invalid HR", "Please enter a valid heart rate > 0")
            return

        if not self.selected_port:
            messagebox.showerror("No COM port", "Please configure a COM port first")
            return

        self.sending = True
        self.progress['value'] = 0
        self.progress_label.config(text="Preparing")
        thread = threading.Thread(target=self._download_thread, args=(hr,), daemon=True)
        thread.start()

    def _generate_ecg(self, hr):
        sampling_rate = 1000
        required_len = int(2 * sampling_rate * (60.0 / hr))  # produce ~2 beats

        if HAS_NK:
            ecg = nk.ecg_simulate(sampling_rate=sampling_rate, heart_rate=hr, method="ecgsyn", length=required_len)
        else:
            # Fallback simple synthetic waveform (sine-like with a spike)
            t = [i / sampling_rate for i in range(required_len)]
            import math
            ecg = [0.2 * math.sin(2 * math.pi * hr / 60.0 * tt) for tt in t]

        minimumLength = int(required_len / 4)
        isolated_ecg = ecg[minimumLength:-minimumLength]
        normalized_ecg = normalize_ecg_endpoints(isolated_ecg)
        set_dac_pp_voltage(2.0)
        dac_ecg = ecg_to_dac_3mVpp(normalized_ecg)
        return dac_ecg

    def _download_thread(self, hr):
        # Stop benchmark if it's running
        was_benchmark_active = self.benchmark_active
        if was_benchmark_active:
            self.stop_benchmark()
        
        try:
            self.log_msg("Generating ECG data...")
            dac_ecg = self._generate_ecg(hr)
            total = len(dac_ecg)
            self.root.after(0, lambda: self.progress.configure(maximum=total))

            # Create uploader and connect (suppress noisy prints coming from uploader)
            self.log_msg(f"Connecting to {self.selected_port} @ {self.baudrate}...")
            uploader = ECGUARTUploader(port=self.selected_port, baudrate=self.baudrate, timeout=2.0)
            if not self._silent_call(uploader.connect):
                self.log_msg("Failed to connect to device")
                self.sending = False
                self.root.after(0, lambda: self.progress_label.config(text="Disconnected"))
                return

            # Step 1: firmware info
            self.log_msg("Requesting firmware info...")
            if not self._silent_call(uploader.get_firmware_info):
                self.log_msg("Firmware info failed")
                self._silent_call(uploader.disconnect)
                self.sending = False
                return

            time.sleep(0.3)

            # Step 2: initiate download
            self.log_msg(f"Initiating download (size={total})...")
            if not self._silent_call(uploader.initiate_ecg_download, total):
                self.log_msg("Initiate download failed")
                self._silent_call(uploader.disconnect)
                self.sending = False
                return

            time.sleep(0.2)

            # Step 3: send samples one by one and update progress
            self.log_msg(f"Sending {total} samples...")
            for idx, value in enumerate(dac_ecg):
                if not uploader.ser or not uploader.ser.is_open:
                    self.log_msg("Serial connection closed unexpectedly")
                    break

                formatted_value = f"{int(value)}"
                cmd = f"DownloadEcgData {idx} {formatted_value}\r"
                # suppress the uploader's prints for send/read
                self._silent_call(uploader.send_command, cmd)
                resp = self._silent_call(uploader.read_response, wait_for="ok", max_wait=1.0)
                if resp is None or "ok" not in resp.lower():
                    self.log_msg(f"No ok for sample {idx}: {repr(resp)}")
                    break

                # update progress in main thread
                self.root.after(0, lambda v=idx+1: self.progress.step(1))
                percent = int(((idx+1) / total) * 100)
                self.root.after(0, lambda p=percent: self.progress_label.config(text=f"{p}%"))

            self.log_msg("Download finished (check device response)")
            self._silent_call(uploader.disconnect)

        except Exception as e:
            self.log_msg(f"ERROR: {e}")
        finally:
            self.sending = False
            self.root.after(0, lambda: self.progress_label.config(text="Idle"))
            # Restart benchmark if it was active before download
            if was_benchmark_active:
                self.root.after(0, self.start_benchmark)


def main():
    root = tk.Tk()
    style = ttk.Style()
    try:
        style.theme_use('clam')
    except Exception:
        pass
    app = App(root)
    root.mainloop()


if __name__ == '__main__':
    main()
