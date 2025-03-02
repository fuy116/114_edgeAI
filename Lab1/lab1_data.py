import os
import pickle
import pandas as pd

#資料路徑處理
BASE_PATH = os.path.join(os.path.dirname(__file__), 'WESAD')


all_data = []

# 讀取受試者S2~S17的資料
for i in range(2, 18):
    if i == 12: 
        continue

    subject_path = os.path.join(BASE_PATH, f'S{i}')
    pickle_file = os.path.join(subject_path, f'S{i}.pkl')

    with open(pickle_file, 'rb') as f:
        data = pickle.load(f, encoding='bytes')

        labels = data[b'label']
        signals = data[b'signal'][b'chest']

        #flatten 將多維陣列轉換為一維陣列
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

# 將所有受試者的資料合併成一個 DataFrame
full_data = pd.concat(all_data, ignore_index=True)

# 定義一個函數，從每個受試者的每個標籤中隨機抽取 40 筆資料
def sample_data(group):
    # 初始化一個空的列表來存儲抽樣結果
    samples = []
    for label in [1, 2]:
        label_data = group[group['Label'] == label]
        if len(label_data) >= 40:
            samples.append(label_data.sample(n=40, random_state=42))
        else:
            raise ValueError(f"受試者 {group['Subject'].iloc[0]} 的 Label {label} 資料不足 40 筆，僅有 {len(label_data)} 筆。")
    return pd.concat(samples)

# 按照受試者分組，並對每個組別應用抽樣函數  
sampled_data = full_data.groupby('Subject').apply(sample_data).reset_index(drop=True)

# 輸出data
#print(sampled_data)

output_path = os.path.join(BASE_PATH, "data.csv")  # 存在 WESAD 資料夾內
sampled_data.to_csv(output_path, index=False)
