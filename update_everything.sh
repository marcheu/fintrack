#/bin/sh

python3 ./download_data.py

mkdir -p tmp
echo "VCR 1.0" > tmp/portfolio_VCR.txt
./fintrack -r tmp/portfolio_VCR.txt -m financial_data/etf/WANT.csv

