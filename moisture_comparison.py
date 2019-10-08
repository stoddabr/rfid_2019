import pandas as pd 
from os import listdir
import numpy as np 
import matplotlib.pyplot as plt
import scipy.stats as st

pd.options.mode.chained_assignment = None  # default='warn'


list_of_file_obj = [
  {
    'file': 'a_b.txt',
    'lid': True,
    'base': 'bin',
    'distance': 'close'
  },
  {
    'file': 'a_f.txt',
    'lid': True,
    'base': 'foam',
    'distance':'close'
  },
  {
    'file': 'a_b2.txt',
    'lid': True,
    'base': 'bin',
    'distance': 'close'
  },
  {
    'file': 'a_b_far.txt',
    'lid': True,
    'base': 'bin',
    'distance': 'far'
  },
  {
    'file': 'a_b_lid.txt',
    'lid': False,
    'base': 'bin',
    'distance': 'far'
  },
  {
    'file': 'a_f_lid.txt',
    'lid': False,
    'base': 'foam',
    'distance': 'far'
  }
]

# from https://stackoverflow.com/questions/15033511/compute-a-confidence-interval-from-sample-data
def c_i_95(series):
  a = series.values
  return st.norm.interval(0.8, loc=np.mean(a), scale=np.std(a)/np.sqrt(series.size) ) #

df = []
df_filtered = []

lid_ser = pd.Series([])
lid_total = 0
lid_count = 0
nolid_ser = pd.Series([])
nolid_total = 0 
nolid_count = 0

base_ser = pd.Series([])
base_total = 0
base_count = 0
nobase_ser = pd.Series([])
nobase_total = 0 
nobase_count = 0

close_ser = pd.Series([])
close_total = 0
close_count = 0
far_ser = pd.Series([])
far_total = 0 
far_count = 0

# graph all on a single graph
color_int = 0
colors = ['b','g','r','c','m','y','k','0.5','orchid','darkseagreen','teal','orange']

fig = plt.figure()
ax1 = fig.add_subplot(111)
df = pd.read_csv(
  'data/'+'a_b_far.txt', 
  error_bad_lines=False, 
  header=None,
  names=['val','rssi','freq','id']
).dropna()
for f in list_of_file_obj:
  df = pd.read_csv(
    'data/'+f['file'], 
    error_bad_lines=False, 
    header=None,
    names=['val','rssi','freq','id']
  ).dropna()

  # frequency limits taken from datasheet, supported US region frequency 
  df_filtered = df[
    ( 917000 < df.freq) & (df.freq < 927200 ) 
  ]

  df_filtered['val'] = pd.to_numeric(df_filtered.val)
  df_filtered['rssi'] = pd.to_numeric(df_filtered.rssi)
  df_filtered['id'] = pd.to_numeric(df_filtered.id)

  try:
    ax1.scatter(
      df_filtered.val, df_filtered.rssi,
      c=colors[color_int], label=f, alpha=0.1
    )
    color_int += 1
  except:
    print('scatter plot faled: ',f)

  if f['lid']:
    lid_count = lid_count + df_filtered['val'].size
    lid_total = lid_total + df_filtered['val'].sum()
    lid_ser = lid_ser.append(df_filtered['val'])
  else:
    nolid_count = lid_count + df_filtered['val'].size
    nolid_total = lid_total + df_filtered['val'].sum()  
    nolid_ser = nolid_ser.append(df_filtered['val'])

  if f['base'] == 'bin':
    base_count = base_count + df_filtered['val'].size
    base_total = base_total + df_filtered['val'].sum()
    base_ser = base_ser.append(df_filtered['val'])
  else:
    nobase_count = base_count + df_filtered['val'].size
    nobase_total = base_total + df_filtered['val'].sum()  
    nobase_ser = nobase_ser.append(df_filtered['val'])

  if f['distance'] == 'far':
    far_count = far_count + df_filtered['val'].size
    far_total = far_total + df_filtered['val'].sum()
    far_ser = far_ser.append(df_filtered['val'])
  else:
    close_count = base_count + df_filtered['val'].size
    close_total = base_total + df_filtered['val'].sum()  
    close_ser = close_ser.append(df_filtered['val'])

print('ave lid', lid_total/lid_count, c_i_95(lid_ser) )
print('ave no lid', nolid_total/nolid_count, c_i_95(nolid_ser) )

print('ave bin', base_total/base_count, c_i_95(base_ser) )
print('ave foam', nobase_total/nobase_count, c_i_95(nobase_ser) )

print('ave far', far_total/far_count, c_i_95(far_ser) )
print('ave close', close_total/close_count, c_i_95(close_ser) )

plt.ylabel('RSSI')
plt.xlabel('RFID Moisture Value')
plt.legend(bbox_to_anchor=(1.04,1), loc="upper left")
plt.show()
