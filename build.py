#!/usr/bin/env python3
from subprocess import run
from os import listdir
from shutil import copy
from pathlib import Path

try:
	Path("jni/basic_glsl.h").write_text('const char * const shaderSource = "' + Path("jni/basic.glsl").read_text().replace("\\", "\\\\").replace("\t", "\\t").replace("\n", "\\n") + '";')
	run(['ndk-build'], check=True)
	apkid = listdir('/tmp/apk-editor-studio/apk')[0]
	apkpath = f"/tmp/apk-editor-studio/apk/{apkid}"
	print(f"Install to {apkpath}/lib/armeabi-v7a/libvrapi.so")
	copy('./libs/armeabi-v7a/libvrapi.so', f'{apkpath}/lib/armeabi-v7a/libvrapi.so')
except Exception as e:
	print("Error:", e)
