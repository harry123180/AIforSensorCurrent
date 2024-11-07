import os

# 設定資料夾路徑
folder_path = 'buf'

# 獲取資料夾內所有檔案名稱
for filename in os.listdir(folder_path):
    # 檢查是否為CSV檔案並且檔名包含"Class"
    if filename.endswith('.csv') and 'Class' in filename:
        # 新檔名將"Class"替換為"case"
        new_filename = filename.replace('Class', 'case')
        # 取得完整的檔案路徑
        old_file = os.path.join(folder_path, filename)
        new_file = os.path.join(folder_path, new_filename)
        # 重新命名檔案
        os.rename(old_file, new_file)
        print(f'Renamed: {filename} -> {new_filename}')
