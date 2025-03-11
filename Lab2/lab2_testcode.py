import os
import pickle
import pandas as pd
import numpy as np
from scipy.signal import butter, lfilter

def butter_bandpass(lowcut, highcut, fs, order=5):
    nyq = 0.5 * fs
    low = lowcut / nyq
    high = highcut / nyq
    b, a = butter(order, [low, high], btype='band')
    return b, a

def butter_bandpass_filter(data, lowcut, highcut, fs, order=5):
    b, a = butter_bandpass(lowcut, highcut, fs, order=order)
    y = lfilter(b, a, data)
    return y

def signal_frequency_band_energies(sampled_signal, frequency_bands, sampling_frequency, order=5, fill_value=0):
    energies = []
    for bands in frequency_bands:
        filtered_signal = butter_bandpass_filter(sampled_signal, bands[0], bands[1], sampling_frequency, order)
        if np.any(np.isnan(filtered_signal)):   
            energies.append(fill_value)   
        else:
            energy = np.sum(filtered_signal**2)
            energies.append(energy)
    return energies

def sliding_window(data, window_size, step_size):
    windows = []
    for start in range(0, len(data) - window_size + 1, step_size):
        window = data[start:(start + window_size)]
        windows.append(window)
    return np.array(windows)

 
BASE_PATH = os.path.join(os.path.dirname(__file__), '..', 'Lab1', 'WESAD')

all_data = []

 
for i in range(2, 17):   
    if i == 12:  
        continue

    subject_path = os.path.join(BASE_PATH, f'S{i}')
    pickle_file = os.path.join(subject_path, f'S{i}.pkl')

    with open(pickle_file, 'rb') as f:
        data = pickle.load(f, encoding='bytes')

        labels = data[b'label']
        signals = data[b'signal'][b'chest']

       
        df = pd.DataFrame({
            'ECG': signals[b'ECG'].flatten(),
            'EMG': signals[b'EMG'].flatten(),
            'EDA': signals[b'EDA'].flatten(),
            'Resp': signals[b'Resp'].flatten(),
            'Temp': signals[b'Temp'].flatten(),
            'Label': labels.flatten()
        })

        df['Subject'] = f'S{i}'
        all_data.append(df)

        window_size = 128   
        step_size = 64  
 
        energy_values = {col: [] for col in ['ECG', 'EMG', 'EDA', 'Resp', 'Temp']}

        for column in ['ECG', 'EMG', 'EDA', 'Resp', 'Temp']:

            signal_windows = sliding_window(df[column].values, window_size, step_size)

      
            if column == 'ECG':
                for window in signal_windows:
                    energies = signal_frequency_band_energies(window, 
                                                              [[0.01, 0.04], [0.04, 0.15], [0.15, 0.4], [0.4, 1.0]], 32)
                    energy_values[column].append(energies)
            else:
                
                energy_values[column].extend([np.nan] * len(signal_windows))

   
        for column in energy_values:
            for i, energy in enumerate(energy_values[column]):
                for j, value in enumerate(energy):
                    df.loc[i, f'{column}_Energy_Band_{j}_Window_{i}'] = value


full_data = pd.concat(all_data, ignore_index=True)

output_path = os.path.join(BASE_PATH, "lab2_data_with_window_features.csv")  
full_data.to_csv(output_path, index=False)
