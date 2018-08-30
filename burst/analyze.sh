#!/bin/sh
for f in *txt; do echo -ne "$f: "; grep -P 'sat_py_beacon_bp_[0-9]+.[0-9]+-[0-9]+_bytes=[0-9]+:(a{27,28}|a{57,60}|a{88,92}|a{120,124}|a{150,158}|a{210,225})END.' $f | wc -l; done
