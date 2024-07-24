[TOC]

This documentation aims to provide a tutorial for Xilinx FPGA PCIe-XDMA integration into OAI and its usage. It can offload LDPC decoding to FPGA.

# Requirements

- XDMA driver

# XDMA Driver Build & Install

The *xdma_driver* directory contains the following:

```bash
xdma_driver
├── cmake
├── FPGA_TEST_datasheet.txt
├── include
├── libfpga_0720_vt.so
├── libfpga_8038_vt.so
├── libfpga_ldpc.a
├── libTHIRD_PARTY.a
├── nr_ldpc_decoding_pym.h
├── README.md
├── tests
├── xdma
└── xdma_diag.h
```

Before building the driver, ensure that your system recognizes the Xilinx device. You can check this using the `lspci` command:

```bash
$ lspci | grep Xilinx
01:00.0 Serial controller: Xilinx Corporation Device 8038
```

Building and Installing the Driver

```
cd xdma_driver/xdma
sudo make clean
sudo make install
cd xdma_driver/tests
sudo ./load_driver.sh
```

# OAI Build

```bash
# Get openairinterface5g source code
git clone https://gitlab.eurecom.fr/oai/openairinterface5g.git ~/openairinterface5g
cd ~/openairinterface5g

# Install OAI dependencies
cd ~/openairinterface5g/cmake_targets
./build_oai -I

# Build OAI gNB & UE
cd ~/openairinterface5g
source oaienv
cd cmake_targets
./build_oai --ninja -w SIMU --gNB --nrUE -P --build-lib "ldpc_xdma" -C -c
```

Shared object file *libldpc_xdma.so* is created during the compilation. This object is conditionally compiled. Selection of the library to compile is done using `--build-lib ldpc_xdma`. 

# 5G PHY simulators

The simulated test uses the option `--loader.ldpc.shlibversion _xdma` to select the XDMA version for loading into the LDPC interface. Additionally, the option `--nrLDPC_coding_xdma.num_threads_prepare` is used to specify the number of threads for preparing data before the LDPC processing, specifically for the deinterleaving and rate matching parts.

Another way to activate the feature is to add the `slot_xdma.conf` file with the following content:

```
nrLDPC_coding_xdma : {
  num_threads_prepare : 2;
};

loader : {
  ldpc : {
    shlibversion : "_xdma";
  };
};

```

and use option `-O slot_xdma.conf`. 

## nr_ulsim test

Example command for running nr_ulsim with LDPC decoding offload to the FPGA:

```bash
cd ~/openairinterface5g/cmake_targets/ran_build/build
sudo ./nr_ulsim -n100 -m28 -r273 -R273 -s22 -I10 -C8 -P --loader.ldpc.shlibversion _xdma --nrLDPC_coding_xdma.num_threads_prepare 2
```

or

```
sudo ./nr_ulsim -n100 -m28 -r273 -R273 -s22 -I10 -C8 -P -O slot_xdma.conf
```

# Run

Both gNB and nrUE use the option `--loader.ldpc.shlibversion _xdma` to select the XDMA version for loading into the LDPC interface and `--nrLDPC_coding_xdma.num_threads_prepare` to specify the number of threads for preparing data before the LDPC processing, specifically for the deinterleaving and rate matching parts.

Another way to activate the feature is to add the following content to the `.conf` file you want to use:

```
nrLDPC_coding_xdma : {
  num_threads_prepare : 2;
};

loader : {
  ldpc : {
    shlibversion : "_xdma";
  };
};

```

and use option `-O *.conf`. 

## gNB

Example command using rfsim:

```bash
cd ~/openairinterface5g/cmake_targets/ran_build/build
sudo ./nr-softmodem --sa --rfsim --log_config.global_log_options level,nocolor,time -O ../../../ci-scripts/conf_files/gnb.sa.band78.106prb.rfsim.conf --loader.ldpc.shlibversion _xdma --nrLDPC_coding_xdma.num_threads_prepare 2
```

or 

```bash
sudo ./nr-softmodem --sa --rfsim --log_config.global_log_options level,nocolor,time -O ../../../ci-scripts/conf_files/gnb.sa.band78.106prb.rfsim.conf
```

if you have added the configuration to the `.conf` file.

## UE

Example command using rfsim:

```bash
cd ~/openairinterface5g/cmake_targets/ran_build/build
sudo ./nr-uesoftmodem --sa --rfsim -r 106 --numerology 1 --band 78 -C 3319680000 --ue-nb-ant-tx 1 --ue-nb-ant-rx 1 -O ../../../ci-scripts/conf_files/nrue1.uicc.cluCN.conf --rfsimulator.serveraddr 10.201.1.100 --loader.ldpc.shlibversion _xdma --nrLDPC_coding_xdma.num_threads_prepare 2
```

or 

```bash
sudo ./nr-uesoftmodem --sa --rfsim -r 106 --numerology 1 --band 78 -C 3319680000 --ue-nb-ant-tx 1 --ue-nb-ant-rx 1 -O ../../../ci-scripts/conf_files/nrue1.uicc.cluCN.conf --rfsimulator.serveraddr 10.201.1.100
```

if you have added the configuration to the `.conf` file.
