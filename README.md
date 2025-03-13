# CETI Whale Tag - V3

# Building
## Docker
To run the build process in docker run:
```sh
make build
```
to manually run docker 
```sh
docker build -t ceti-img . # create image
docker run --rm --volume $(pwd):/ceti-firmware ceti-img make #run make inside image
```


## Local(Linux)

Works with Ubuntu 22.04 or higher

### 1. Initialize subrepositories:
This repo uses submodule. Clone it with the `--recursive` flag:
```
git clone --recursive https://github.com/Project-CETI/ceti-whale-tag-v3-fw.git
```
or
```
git submodule update --init --recursive
```

### 2. Install dependencies
Required package are listed in [`packages.txt`](packages.txt) and can be install 
```sh
sudo apt-get update && sudo apt-get install --no-install-recommends -y $(cat ./packages.txt)
```

### Building
The .elf file can be installed with the `make` command:
```sh
make # Build with DEBUG configuration
make DEBUG=0 # Build release version of .elf file
```

# Flashing
### Prerequirements:

### Flashing
```sh
make flash
```

# Dev Environment