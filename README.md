# For querying and communication with HP 70951A OSA

Utilizng Matlab C/C++ MEX API, it allows querying of data from HP 70951A or deriviatives.

## Requirements
1. Installation of visa.h and like libraries. Can be sourced from National Instruments or Keysight (code uses NI-VISA)
2. Change PATHS of libraries containing the libraries in the compile.m function
3. Change PATHS of the source code of visa.h and like .h headers. See rvisa_implt.cpp and qvisa_implt.cpp for more details

## Steps
1. After satisfying requirements. For first-time runs, run the compile.m function. This will compile both rvisa_implt.cpp and qvisa_implt.cpp functions.
2. Use the read_visa (GPIB, CMD) and ask_visa (GPIB, CMD) to communicate with HP 70951A OSA or derivatives.

That's it!
