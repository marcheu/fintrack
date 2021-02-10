#!/usr/bin/python3

from pandas_datareader import data
import os

etf_list = [
'AGQ',
'BIB',
'BIV',
'BND',
'CHAU',
'CWEB',
'CURE',
'DDM',
'DIG',
'EDC',
'EDV',
'EET',
'EFO',
'ERX',
'EZJ',
'FAS',
'GUSH',
'IMTM',
'INDL',
'JNUG',
'LABU',
'LTL',
'MGK',
'MIDU',
'MSEGX',
'MVV',
'NUGT',
'PPA',
'QLD',
'QQQ',
'RETL',
'ROM',
'RXL',
'SAA',
'SDG',
'SOXL',
'SSO',
'TECL',
'TMF',
'TNA',
'TQQQ',
'TTT',
'TYD',
'UBR',
'UBT',
'UST',
'UCC',
'UCO',
'UDOW',
'UGE',
'UGL',
'UJB',
'ULE',
'UMDD',
'UPRO',
'UPV',
'UPW',
'URE',
'URTY',
'USD',
'UWM',
'UXI',
'UYG',
'UYM',
'VAW',
'VBK',
'VBR',
'VCR',
'VDC',
'VDE',
'VEMPX',
'VEXAX',
'VFH',
'VGLT',
'VGPMX',
'VGT',
'VHT',
'VIGAX',
'VIIIX',
'VIS',
'^VIX',
'VNQ',
'VO',
'VONG',
'VOO',
'VOOG',
'VOT',
'VOX',
'VPU',
'VT',
'VTI',
'VTSAX',
'VTWG',
'VUG',
'VWO',
'VXF',
'XPP',
'YINN'
]


stock_list = [
'AAPL',
'ADBE',
'AMD',
'AMZN',
'GOOG',
'GOOGL',
'INTC',
'MSFT',
'NFLX'
]

# Verify that destination directory exists
for path in ['financial_data/etf', 'financial_data/stock']:
    if not os.path.exists(path):
        print(path, "does not exist, creating it.")
        os.makedirs(path)

for stock_name in etf_list:
    print("Downloading ETF", stock_name)
    series = data.DataReader(stock_name, 'yahoo', '1980-01-01')
    series.to_csv(stock_name + '.csv')
    os.rename(stock_name + '.csv', 'financial_data/etf/' + stock_name + '.csv')

for stock_name in stock_list:
    print("Downloading stock", stock_name)
    series = data.DataReader(stock_name, 'yahoo', '1980-01-01')
    series.to_csv(stock_name + '.csv')
    os.rename(stock_name + '.csv', 'financial_data/stock/' + stock_name + '.csv')

