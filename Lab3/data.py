import os
import pickle
import numpy as np
import pandas as pd
from scipy import stats
from sklearn.preprocessing import MinMaxScaler


BASE_PATH = "../WESAD/"

all_processed_windows = np.empty((0, 7))
#all_processed_windows = np.array(all_processed_windows)

for i in range(2, 18):
    if i == 12: 
        continue

    subject_path = os.path.join(BASE_PATH, f'S{i}')
    pickle_file = os.path.join(subject_path, f'S{i}.pkl')

    with open(pickle_file, 'rb') as f:
        data = pickle.load(f, encoding='bytes')

        labels = data[b'label']
        signals = data[b'signal'][b'chest']

        # 只選擇 label == 1 或 label == 2 的數據
        mask = (labels == 1) | (labels == 2)
        labels = labels[mask]
        signals = {k: v[mask] for k, v in signals.items()}

        ecg_signal = signals[b'ECG'].flatten()
        emg_signal = signals[b'EMG'].flatten()
        eda_signal = signals[b'EDA'].flatten()
        resp_signal = signals[b'Resp'].flatten()
        temp_signal = signals[b'Temp'].flatten()
        
        # 確保所有信號長度一致
        min_length = min( len(ecg_signal), len(emg_signal), len(eda_signal), len(resp_signal), len(temp_signal), len(labels))
        ecg_signal = ecg_signal[:min_length]

        emg_signal = emg_signal[:min_length]
        eda_signal = eda_signal[:min_length]
        resp_signal = resp_signal[:min_length]
        temp_signal = temp_signal[:min_length]
        labels = labels[:min_length]
       

        subject_id = f'S{i}'
        #window_features = np.concatenate([ecg_signal,emg_signal,eda_signal,resp_signal,temp_signal,labels],axis=1)
        window_features = np.column_stack([ecg_signal,emg_signal,eda_signal,resp_signal,temp_signal,labels])
        #print(window_features.shape) #(1231300, 6)

       

        subject_column = np.full((window_features.shape[0], 1), subject_id)

        # 將 subject_column 加入到 window_features 中
        window_features_with_subject = np.hstack([window_features, subject_column])
        all_processed_windows = np.concatenate((window_features_with_subject,all_processed_windows), axis=0)



# Columns including subject
columns = ['ECG_singal','EMG_signal','EDA_signal','Resp_signal','Temp_signal',
           'Label', 'Subject']
#print(all_processed_windows.shape)
processed_data_df = pd.DataFrame(all_processed_windows, columns=columns)
#print(processed_data_df)

# 正規化所有特徵
scaler = MinMaxScaler()

# 提取特徵列
features = processed_data_df.iloc[:, :-2].values  # 所有特徵列，排除標籤和受試者ID
features_normalized = scaler.fit_transform(features)

# 創建一個新的DataFrame來存儲正規化後的數據
normalized_df = pd.DataFrame(features_normalized, columns=processed_data_df.columns[:-2])

# 將正規化後的數據與標籤、受試者ID合併
normalized_df['Label'] = processed_data_df['Label']
normalized_df['Subject'] = processed_data_df['Subject']

output_file = "../output/lab3_data.csv"
normalized_df.to_csv(output_file, index=False)

print(f"Processed data saved to {output_file}")
     