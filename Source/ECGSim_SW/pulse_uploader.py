"""
Upload a 1Hz pulse with 3mV amplitude and 10% duty cycle to the device via UART.
"""

import sys
import numpy as np
from ecg_uart_uploader import ECGUARTUploader

# Constants
VCC = 3.3  # DAC reference voltage in V
DAC_BITS = 12
DAC_MAX = (1 << DAC_BITS) - 1  # 4095

def generate_pulse_dac_codes(sampling_rate=1000, duration_seconds=1, frequency_hz=1, duty_cycle=0.1, amplitude_mv=3.0):
    """
    Generate a square pulse and convert directly to 12-bit DAC codes.
    
    Args:
        sampling_rate: Sampling frequency in Hz
        duration_seconds: Duration of waveform in seconds (default 1 second)
        frequency_hz: Pulse frequency in Hz (1Hz means period = 1 second)
        duty_cycle: Duty cycle as fraction (0.1 = 10%)
        amplitude_mv: Peak amplitude in mV
        
    Returns:
        dac_codes: numpy array of 12-bit DAC codes (uint16)
    """
    num_samples = int(sampling_rate * duration_seconds)
    period_samples = int(sampling_rate / frequency_hz)
    high_samples = int(period_samples * duty_cycle)
    
    # Create one period
    one_period = np.concatenate([
        np.ones(high_samples),  # 1 = high state
        np.zeros(period_samples - high_samples)  # 0 = low state
    ])
    
    # Repeat to fill duration
    num_periods = int(num_samples / period_samples) + 1
    pulse = np.tile(one_period, num_periods)[:num_samples]
    
    # Convert to DAC codes directly
    # amplitude_mv is the desired DAC output voltage in mV
    dac_voltage_v = amplitude_mv * 1e-3  # Convert mV to V
    dac_code_high = int((dac_voltage_v / VCC) * DAC_MAX)
    dac_code_low = 0
    
    # Generate DAC codes
    dac_codes = np.where(pulse > 0.5, dac_code_high, dac_code_low).astype(np.uint16)
    
    return dac_codes, dac_code_high


if __name__ == "__main__":
    # Generate 1 second of 1Hz pulse with 3mV amplitude and 10% duty cycle
    print("Generating 1Hz pulse (3mV amplitude, 10% duty cycle)...")
    dac_pulse, dac_high = generate_pulse_dac_codes(
        sampling_rate=1000,
        duration_seconds=1,
        frequency_hz=1,
        duty_cycle=0.1,
        amplitude_mv=6.0
    )
    
    print(f"Generated {len(dac_pulse)} samples")
    print(f"DAC code range: {dac_pulse.min()} to {dac_pulse.max()}")
    print(f"High state DAC code: {dac_high} (corresponds to {(dac_high / DAC_MAX * VCC * 1000):.3f} mV output)")
    
    # Configure your COM port here
    COM_PORT = "COM3"  # Change to your device's COM port
    BAUD_RATE = 115200
    
    # Create uploader and connect
    print(f"\nConnecting to {COM_PORT}...")
    uploader = ECGUARTUploader(port=COM_PORT, baudrate=BAUD_RATE, timeout=2.0)
    
    if uploader.connect():
        # Upload pulse data
        uploader.upload_ecg(dac_pulse)
        uploader.disconnect()
    else:
        print("Failed to connect to device")
        sys.exit(1)

