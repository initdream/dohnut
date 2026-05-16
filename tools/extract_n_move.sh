#!/bin/bash

g++ -O2 -std=c++17 rcf.cpp -o rcf_extractor

for f in music*.rcf; do
    ./rcf_extractor "$f"
done

./rcf_extractor ambience.rcf

cd extracted_rcf/sound || exit

python3 renamesound.py

cd ../..

cp -a extracted_rcf/sound/*/ sound/
rm -rf extracted_rcf/sound/*/