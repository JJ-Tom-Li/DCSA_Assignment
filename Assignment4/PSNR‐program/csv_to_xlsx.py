import pandas as pd
from os import listdir
from os.path import join
from os import remove
def csv_to_xlsx(filename):
     csv = pd.read_csv(filename, encoding='utf-8')
     filename=filename[:-4]+".xlsx"
     #print(filename)
     csv.to_excel(filename, sheet_name='data')

if __name__ == '__main__':
    path = '../PSNR_result'
    files = listdir(path)
    for f in files:
        fullpath = join(path, f)
        org_path=fullpath
        csv_to_xlsx(fullpath)
        remove(org_path)