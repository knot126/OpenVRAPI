#!/usr/bin/env python3
from subprocess import run
from os import listdir
from shutil import copy

try:
	run(['ndk-build'], check=True)
	apkid = listdir('/tmp/apk-editor-studio/apk')[0]
	apkpath = f"/tmp/apk-editor-studio/apk/{apkid}"
	copy('./libs/armeabi-v7a/libvrapi.so', f'{apkpath}/lib/armeabi-v7a/libvrapi.so')
except Exception as e:
	print("Error:", e)
