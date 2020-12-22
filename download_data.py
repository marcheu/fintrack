#!/usr/bin/python3

from pandas_datareader import data
import os

etf_list = [
'AGQ',
'ARKK',
'ARKW',
'BIB',
'BIV',
'BND',
'CHAU',
'DDM',
'DIG',
'EDV',
'EET',
'EFO',
'ERX',
'EZJ',
'FAS',
'LABU',
'LTL',
'MGK',
'MVV',
'NUGT',
'QLD',
'QQQ',
'RETL',
'ROM',
'RXL',
'SAA',
'SOXL',
'SSO',
'TECL',
'TMF',
'TNA',
'TQQQ',
'UBR',
'UBT',
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
'XPP'
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

for stock_name in etf_list:
	series = data.DataReader(stock_name, 'yahoo', '1980-01-01')
	series.to_csv(stock_name + '.csv')
	os.rename(stock_name + '.csv', 'financial_data/etf/' + stock_name + '.csv')

for stock_name in stock_list:
	series = data.DataReader(stock_name, 'yahoo', '1980-01-01')
	series.to_csv(stock_name + '.csv')
	os.rename(stock_name + '.csv', 'financial_data/stock/' + stock_name + '.csv')

