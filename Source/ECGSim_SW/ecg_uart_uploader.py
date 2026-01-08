import serial
import time
import sys
import numpy as np
import matplotlib.pyplot as plt

DAC_VREF = 3.3
DAC_BITS = 12
DAC_MAX = (1 << DAC_BITS) - 1        # 4095
DAC_MID = DAC_MAX // 2               # 2047

DIVIDER_RATIO = 10.0 / (10000.0 + 10.0)   # ≈ 0.000999

ECG_PP_MV = 3.0   # desired ECG amplitude at RA–LA

# Required DAC peak-to-peak voltage
DAC_PP_V = (ECG_PP_MV * 1e-3) / DIVIDER_RATIO   # ≈ 3.0 V

# Corresponding DAC peak-to-peak codes
DAC_PP_CODES = (DAC_PP_V / DAC_VREF) * DAC_MAX

DAC_HALF_SWING = DAC_PP_CODES / 2.0

def ecg_to_dac_3mVpp(ecg):
    """
    Convert ecgsyn ECG waveform to 12-bit DAC codes that produce
    ~3 mV p-p ECG after 10k–10Ω–10k resistor network.

    Parameters
    ----------
    ecg : array-like
        ECG waveform from nk.ecg_simulate()

    Returns
    -------
    dac : np.ndarray (uint16)
        DAC codes ready for MCP4725
    """

    ecg = np.asarray(ecg, dtype=np.float64)

    # 1️⃣ Find min and max
    vmin = ecg.min()
    vmax = ecg.max()
    vrange = vmax - vmin

    if vrange < 1e-12:
        return np.full_like(ecg, DAC_MID, dtype=np.uint16)

    # 2️⃣ Normalize to [-1, +1]
    ecg_norm = 2.0 * (ecg - vmin) / vrange - 1.0

    # 3️⃣ Scale to DAC
    dac = DAC_MID + ecg_norm * DAC_HALF_SWING

    # 4️⃣ Clamp and convert
    dac = np.clip(dac, 0, DAC_MAX)

    return dac.astype(np.uint16)

def normalize_ecg_endpoints(ecg):
    """
    Removes linear baseline drift so first and last samples match.
    Safe for tiling ECG beats.
    """
    ecg = np.asarray(ecg, dtype=np.float64)

    N = len(ecg)
    if N < 2:
        return ecg.copy()

    start = ecg[0]
    end = ecg[-1]

    # Linear trend from start to end
    trend = np.linspace(start, end, N)

    # Remove trend, re-anchor to start value
    ecg_corrected = ecg - trend + start

    return ecg_corrected

class ECGUARTUploader:
    """Handle UART communication with firmware device for ECG data upload."""
    
    def __init__(self, port='COM3', baudrate=115200, timeout=2.0):
        """
        Initialize UART connection.
        
        Args:
            port: Serial port (e.g., 'COM3' on Windows, '/dev/ttyUSB0' on Linux)
            baudrate: Baud rate for communication
            timeout: Read timeout in seconds
        """
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout
        self.ser = None
        
    def connect(self):
        """Establish UART connection."""
        try:
            self.ser = serial.Serial(
                port=self.port,
                baudrate=self.baudrate,
                timeout=self.timeout,
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE
            )
            print(f"Connected to {self.port} at {self.baudrate} baud")
            return True
        except serial.SerialException as e:
            print(f"Failed to connect: {e}")
            return False
    
    def disconnect(self):
        """Close UART connection."""
        if self.ser and self.ser.is_open:
            self.ser.close()
            print("Disconnected")
    
    def send_command(self, command):
        """Send command to device."""
        if not self.ser or not self.ser.is_open:
            raise RuntimeError("Serial connection not open")
        self.ser.write(command.encode())
        print(f">>> Sent: {repr(command)}")
    
    def read_response(self, wait_for=None, max_wait=None):
        """
        Read response from device.
        
        Args:
            wait_for: Optional string to wait for (blocking)
            max_wait: Optional maximum wait time in seconds
            
        Returns:
            Response string or None if timeout
        """
        if not self.ser or not self.ser.is_open:
            raise RuntimeError("Serial connection not open")
        
        response = ""
        start_time = time.time()
        max_timeout = max_wait if max_wait else self.timeout
        
        while True:
            if self.ser.in_waiting > 0:
                byte = self.ser.read().decode('utf-8', errors='ignore')
                response += byte

                # If caller expects a specific token, wait until it's present.
                if wait_for:
                    if wait_for in response:
                        print(f"<<< Received: {repr(response)}")
                        return response
                else:
                    # No specific token requested: treat any received data as acknowledgement
                    print(f"<<< Received: {repr(response)}")
                    return response

                # Check for timeout (only reached when waiting for a token)
                if time.time() - start_time > max_timeout:
                    if response:
                        print(f"<<< Received: {repr(response)}")
                    else:
                        print("<<< Timeout (no response)")
                    return response if response else None
            else:
                # Small delay to prevent CPU spinning
                time.sleep(0.01)
                if time.time() - start_time > max_timeout:
                    if response:
                        print(f"<<< Received: {repr(response)}")
                    else:
                        print("<<< Timeout (no response)")
                    return response if response else None
    
    def get_firmware_info(self):
        """Send GetFirmwareInfo command and confirm response."""
        print("\n[Step 1] Getting Firmware Info...")
        self.send_command("GetFirmwareInfo\r")
        response = self.read_response(wait_for="ok")

        if response is None :
            print("ERROR: No 'ok' acknowledgement from device")
            return False
        
        print("SUCCESS: Firmware info received")
        return True
    
    def initiate_ecg_download(self, data_size):
        """Send InitiateEcgDownload command with data size."""
        print(f"\n[Step 2] Initiating ECG Download (size={data_size})...")
        self.send_command(f"InitiateEcgDownload {data_size}\r")
        response = self.read_response(wait_for="ok")

        if response is None or "ok" not in response.lower():
            print(f"ERROR: Expected 'ok' response, got: {repr(response)}")
            return False
        
        print("SUCCESS: Download initiated")
        return True
    
    def send_ecg_data(self, ecg_data):
        """
        Send ECG data one value at a time.
        
        Args:
            ecg_data: List or array of float values
            
        Returns:
            True if all data sent successfully, False otherwise
        """
        print(f"\n[Step 3] Sending {len(ecg_data)} ECG samples...")
        
        for idx, value in enumerate(ecg_data):
            # Format value to 5 decimal places
            formatted_value = f"{value:.5f}"
            command = f"DownloadEcgData {idx} {formatted_value}\r"
            
            self.send_command(command)
            response = self.read_response(wait_for="ok", max_wait=1.0)
            
            # Print progress every 100 samples
            if (idx + 1) % 100 == 0:
                print(f"  Sent {idx + 1}/{len(ecg_data)} samples")
            
            if response is None or "ok" not in response.lower():
                print(f"ERROR: No 'ok' acknowledgement for sample {idx}, got: {repr(response)}")
                return False
        
        print(f"SUCCESS: All {len(ecg_data)} samples sent")
        return True
    
    def upload_ecg(self, ecg_data):
        """
        Complete ECG upload sequence.
        
        Args:
            ecg_data: List or array of float values
            
        Returns:
            True if upload successful, False otherwise
        """
        try:
            # Step 1: Get firmware info
            if not self.get_firmware_info():
                return False
            
            time.sleep(0.5)
            
            # Step 2: Initiate download
            if not self.initiate_ecg_download(len(ecg_data)):
                return False
            
            time.sleep(0.5)
            
            # Step 3: Send data
            if not self.send_ecg_data(ecg_data):
                return False
            
            print("\n✓ ECG upload completed successfully!")
            return True
            
        except Exception as e:
            print(f"\nERROR: {e}")
            return False


if __name__ == "__main__":
    # Example usage with simulated ECG data
    import neurokit2 as nk
    
    # Generate sample ECG data
    print("Generating sample ECG data...")
    required_hr = 30
    required_len = int(2 * 1000 * (60 / required_hr))  # 2 beats
    ecg = nk.ecg_simulate(sampling_rate=1000, heart_rate=required_hr, method="ecgsyn", length=required_len)
    
    minimumLength = int(required_len / 4)
    isolated_ecg = ecg[minimumLength:-minimumLength]
    normalized_ecg = normalize_ecg_endpoints(isolated_ecg)
    dac_ecg = ecg_to_dac_3mVpp(normalized_ecg)
    print(f"Generated {len(dac_ecg)} ECG samples\n")
    
    # Configure your COM port here
    COM_PORT = "COM5"  # Change to your device's COM port
    BAUD_RATE = 115200  # Change if needed
    

    # Create uploader and connect
    uploader = ECGUARTUploader(port=COM_PORT, baudrate=BAUD_RATE, timeout=2.0)
    
    if uploader.connect():
        # Upload ECG data
        uploader.upload_ecg(dac_ecg)
        uploader.disconnect()
    else:
        print("Failed to connect to device")
        sys.exit(1)
