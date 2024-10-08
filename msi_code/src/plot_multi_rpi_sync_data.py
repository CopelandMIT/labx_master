import matplotlib.pyplot as plt
import pandas as pd

# Load your data
data_file = '/home/daniel/labx_time_sync/time_experiment/synchronization_metrics_2_rpi_1_radar_1727290489.1156256.csv'
appendix = "_radar_3600"

# Assuming you have a CSV file with columns 'timestamp', 'max_offset_ms', 'mean_offset_ms', 'jitter_ms', 'mean_root_dispersion_ms'
data = pd.read_csv(data_file)
start_time = data['timestamp'][0]
time = data['timestamp'] - start_time

# Plot the max offset
plt.figure()
plt.plot(time, data['max_offset_ms'], label='Max Offset (ms)')
plt.xlabel('Time (seconds)')
plt.ylabel('Max Offset (ms)')
plt.title('Maximum Offset Between SBCs Over Time')
plt.legend()

# Save the max offset plot
plt.savefig(f'max_offset_plot{appendix}.png')

# Plot the mean offset
plt.figure()
plt.plot(time, data['mean_offset_ms'], label='Mean Offset (ms)')
plt.xlabel('Time (seconds)')
plt.ylabel('Mean Offset (ms)')
plt.title('Mean Offset Between SBCs Over Time')
plt.legend()

# Save the mean offset plot
plt.savefig(f'mean_offset_plot{appendix}.png')

# Plot the jitter (standard deviation of offsets)
plt.figure()
plt.plot(time, data['jitter_ms'], label='Jitter (ms)')
plt.xlabel('Time (seconds)')
plt.ylabel('Jitter (ms)')
plt.title('Jitter (Std Dev of Offsets) Over Time')
plt.legend()

# Save the jitter plot
plt.savefig(f'jitter_plot{appendix}.png')

# Plot the mean root dispersion
plt.figure()
plt.plot(time, data['mean_root_dispersion_ms'], label='Mean Root Dispersion (ms)')
plt.xlabel('Time (seconds)')
plt.ylabel('Mean Root Dispersion (ms)')
plt.title('Mean Root Dispersion Over Time')
plt.legend()

# Save the mean root dispersion plot
plt.savefig(f'mean_root_dispersion_plot{appendix}.png')
