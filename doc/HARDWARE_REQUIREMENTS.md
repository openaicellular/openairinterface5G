<table style="border-collapse: collapse; border: none;">
  <tr style="border-collapse: collapse; border: none;">
    <td style="border-collapse: collapse; border: none;">
      <a href="http://www.openairinterface.org/">
         <img src="./images/oai_final_logo.png" alt="" border=3 height=50 width=150>
         </img>
      </a>
    </td>
    <td style="border-collapse: collapse; border: none; vertical-align: center;">
      <b><font size = "5">Hardware Requirement for Using OAI Stack</font></b>
    </td>
  </tr>
</table>

This document explains the minimal and performance hardware requirement for OpenAirInterface (OAI) software stack (UE and RAN stack). The information provided in this document is based on experimentation, if you have a feedback then open an issue or send an email on the mailing list. 

**Table of Contents**

[[_TOC_]]

# Supported CPU Architecture

|Architecture                               |
|------------------------------------------ |
|x86_64 (Intel, AMD)                        |
|aarch64 (Neoverse-N1, N2 and Grace hopper) |

**Note**: 

- On `x86_64` platform the CPU should support `avx2` instruction set (Minimum requirement).

# Minimum Hardware Requirement for x86_64 Platforms

The minimum hardware requirements depends on the radio unit you would like to use or the test case that you would like to execute.  

## Simulated Radio 

OAI offers an inbuilt simulated radio, [RFSimulator](../radio/rfsimulator/README.md). It can be used to understand how different channel model behaves. It is not designed to do high performance testing. The above requirements are valid for both 4G and 5G RAN and UE Stack. 

### Minimum requirements for RAN Stack

- CPU: 4
- Memory: 4Gi
- `avx2` instruction set
- `sctp` protocol should be enabled

### Minimum requirements for UE Stack

- CPU: 3
- Memory: 3Gi
- `avx2` instruction set


## USRP B2XX or Blade RF

USRP B2XX or Blade RF are USB based radios recommended to use with USB 3.0. You can choose a minimum hardware to do functional testing and performance hardware for performance testing. This hardware you can find in Mini-PCs or laptops.

### Minimum requirements for RAN Stack

- CPU: 4
- Memory: 4Gi
- Instruction set: `avx2` 
- Protocol should be enabled: `sctp` 
- USB 3.0

### Minimum requirements for UE Stack only with USRP B2XX

- CPU: X
- Memory: X
- Instruction set: `avx2` 
- USB 3.0

### Recommended for performance for RAN Stack

- Boost Frequency > 4GHz 
- DDR4 or DDR5 RAM
- Use only performance cores
- Disable CPU sleep states
- CPU: X
- Memory: 4Gi
- Instruction set: `avx2` 
- Protocol should be enabled: `sctp` 
- USB 3.0

### Recommended for performance for UE Stack only with USRP B2XX

- Boost Frequency > 4GHz 
- DDR4 or DDR5 RAM 
- Use only performance cores
- Disable CPU sleep states
- CPU: X
- Memory: 4Gi
- Instruction set: `avx2` 
- USB 3.0

Apart from this you should follow [this document](./tuning_and_security.md) to 
tune your system to get high performance. 

## USRP N3XX/X3XX/X4XX

USRP N3XX/X3XX/X4XX requires two dedicated 10G SFP+ connections. For these radios we only recommend having performance hardware. This hardware you can find in Desktop servers or rack/blade servers. 

### Recommended for performance for RAN Stack

- Boost Frequency > 4GHz 
- DDR4 or DDR5 RAM 
- Use only performance cores
- Disable CPU sleep states
- CPU: X
- Memory: X
- Instruction set: `avx2` 
- Protocol should be enabled: `sctp` 
- (Optional) Realtime kernel to have better Jitter statistics
- Intel x710/xx710/E-810 or Mellanox connect 5x or 6x

### Recommended for performance for UE Stack

- Boost Frequency > 4GHz 
- DDR4 or DDR5 RAM
- Use only performance cores
- Disable CPU sleep states
- CPU: X
- Memory: X
- Instruction set: `avx2` 
- (Optional) Realtime kernel to have better Jitter statistics
- Intel x710/xx710/E-810 or Mellanox connect 5x or 6x

In case you are using Mellanox NIC cards then you have to download `mlnx-ofed` and configure your NIC for performance.

Apart from this you should follow [this document](./tuning_and_security.md) to 
tune your system to get high performance. 

## O-RAN Radio Units

We have dedicated documentation for O-RAN Radio Units. [Refer to that documentation](./ORAN_FHI7.2_Tutorial.md) before purchasing a Desktop server or rack/blade server. 

