import os
import glob

sources = []

for n in glob.glob("src/**/*.cpp", recursive=True):
	sources.append(n)

print("\n".join(sources))
