import os
import pickle
import pandas as pd
import numpy as np
from scipy.signal import butter, lfilter
from scipy import stats

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
    # 先標準化信號
    mean_val = np.mean(sampled_signal)
    std_val = np.std(sampled_signal)
    if std_val < 1e-10:  # 避免除以接近零的值
        return [fill_value] * len(frequency_bands)
    
    normalized_signal = (sampled_signal - mean_val) / std_val
    
    energies = []
    for bands in frequency_bands:
        filtered_signal = butter_bandpass_filter(normalized_signal, bands[0], bands[1], sampling_frequency, order)
        
        # 處理異常值
        if np.any(np.isnan(filtered_signal)) or np.any(np.isinf(filtered_signal)):
            energies.append(fill_value)
        else:
            # 使用平均能量而不是總能量
            energy = np.mean(filtered_signal**2)  # 使用平均值而不是總和
            energies.append(energy)
    return energies
def extract_windows(data, window_size):
    """將信號分割成不重疊的窗口，每個窗口大小為window_size"""
    windows = []
    for i in range(0, len(data), window_size):  # 每次跳過window_size的長度
        window = data[i:i + window_size]
        if len(window) == window_size:  # 確保每個窗口的大小是固定的
            windows.append(window)
    return np.array(windows)

# 資料路徑處理
BASE_PATH = os.path.join(os.path.dirname(__file__), '..', 'WESAD')

# 用於存儲所有壓縮後的數據
all_compressed_data = []

# 讀取受試者S2~S17的資料
for i in range(2, 17):  # 設定範圍
    if i == 12:  # 跳過受試者12
        continue
        
    print(f"處理受試者 S{i}...")
    
    subject_path = os.path.join(BASE_PATH, f'S{i}')
    pickle_file = os.path.join(subject_path, f'S{i}.pkl')

    try:
        with open(pickle_file, 'rb') as f:
            data = pickle.load(f, encoding='bytes')

            labels = data[b'label']
            signals = data[b'signal'][b'chest']

            # flatten 將多維陣列轉換為一維陣列
            original_df = pd.DataFrame({
                'ECG': signals[b'ECG'].flatten(),
                'EMG': signals[b'EMG'].flatten(),
                'EDA': signals[b'EDA'].flatten(),
                'Resp': signals[b'Resp'].flatten(),
                'Temp': signals[b'Temp'].flatten(),
                'Label': labels.flatten()
            })
            
            # 只選擇 Label == 1 或 Label == 2 的資料
            filtered_df = original_df[original_df['Label'].isin([1, 2])]
            
            if filtered_df.empty:
                print(f"受試者 S{i} 沒有 Label 為 1 或 2 的資料，跳過")
                continue
                
            # 設定窗口大小
            window_size = 5000
            
            # 為每個信號創建窗口
            signal_windows = {}
            for column in ['ECG', 'EMG', 'EDA', 'Resp', 'Temp']:
                signal_windows[column] = extract_windows(filtered_df[column].values, window_size)
            
            # 為標籤創建窗口
            label_windows = extract_windows(filtered_df['Label'].values, window_size)
            
            # 檢查是否有窗口被創建
            if len(signal_windows['ECG']) == 0:
                print(f"受試者 S{i} 沒有足夠的資料來創建 {window_size} 大小的窗口，跳過")
                continue
                
            # 創建一個新的壓縮數據列表
            compressed_data = []
            
            # 處理每個窗口
            for window_idx in range(len(signal_windows['ECG'])):
                # 獲取這個窗口的最常見標籤作為窗口標籤
                window_label = stats.mode(label_windows[window_idx], keepdims=True)[0][0]
                
                # 創建一個新的特徵字典
                window_features = {
                    'Subject': f'S{i}',
                    'Label': window_label
                }
                
                # 計算每個信號的統計特徵
                for column in ['ECG', 'EMG', 'EDA', 'Resp', 'Temp']:
                    window_data = signal_windows[column][window_idx]
                    window_features[f'{column}_Mean'] = np.mean(window_data)
                    # window_features[f'{column}_Std'] = np.std(window_data)
                    # window_features[f'{column}_Min'] = np.min(window_data)
                    # window_features[f'{column}_Max'] = np.max(window_data)
                    # window_features[f'{column}_Median'] = np.median(window_data)
                    # window_features[f'{column}_RMS'] = np.sqrt(np.mean(window_data**2))  # 均方根
                
                # 只為ECG信號計算頻域特徵
                ecg_energies = signal_frequency_band_energies(
                    signal_windows['ECG'][window_idx], 
                    [[0.01, 0.04], [0.04, 0.15], [0.15, 0.4], [0.4, 1.0]], 
                    32
                )
                
                # 將頻域特徵添加到窗口特徵中
                for band_idx, band_name in enumerate(['VLF', 'LF', 'HF', 'VHF']):
                    window_features[f'ECG_{band_name}_Energy'] = ecg_energies[band_idx]
                
                # 將這個窗口的特徵添加到壓縮數據列表
                compressed_data.append(window_features)
            
            # 將壓縮後的數據轉換為DataFrame
            subject_compressed_df = pd.DataFrame(compressed_data)
            all_compressed_data.append(subject_compressed_df)
            
            print(f"受試者 S{i} 已處理完成。原始行數: {len(filtered_df)}, 壓縮後行數: {len(subject_compressed_df)}")
            
    except Exception as e:
        print(f"處理受試者 S{i} 時發生錯誤: {e}")
        continue

# 合併所有壓縮後的數據
if all_compressed_data:
    full_compressed_data = pd.concat(all_compressed_data, ignore_index=True)
    
    # 打印資料筆數
    print(f"原始數據總筆數: (無法計算，因為直接使用了窗口處理)")
    print(f"壓縮後數據總筆數: {full_compressed_data.shape[0]}")
    
    # 輸出資料至CSV檔案
    output_path = os.path.join(BASE_PATH, "lab2_compressed_data_with_features.csv")  # 儲存到 WESAD 資料夾內
    full_compressed_data.to_csv(output_path, index=False)
    
    print(f"壓縮數據已保存到: {output_path}")
else:
    print("沒有產生任何壓縮數據，請檢查數據源或參數設置。")