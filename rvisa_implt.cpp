/**
 * @file rvisa_implt.cpp
 * @author Peter Tso (tsopeter@ku.edu)
 * @brief Read visa code for use in MATLAB 2022A
 *        Adapted from: https://github.com/Terrabits/c-visa-example
 *        User:  https://github.com/Terrabits
 * 
 * @version 0.1
 * @date 2022-10-06
 *
 * 
 */

#include "mex.hpp"
#include "mexAdapter.hpp"

#include <cstdio>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <array>
#include <stdexcept>

/**
 *  Modify the path to the visa.h library.
 *  I know there probably is an easier or better way of doing this but,
 *  right now it is a stop-gap measure.
 *
 *  Run compile.m to compile the function before running. Read compile.m comments for more instructions
 *  read_visa.m
 */
#include "C:\Program Files\IVI Foundation\VISA\Win64\Include\visa.h"
#include "C:\Program Files\IVI Foundation\VISA\Win64\Include\visatype.h"
#include "C:\Program Files\IVI Foundation\VISA\Win64\Include\vpptype.h"

// This defines the base type, how the data is stored internally
#define BASE_TYPE double

// This prints out an error if an invalid argument is passed, or other errors has happened
#define PRINT_ERR(str){\
    matlabPtr->feval(u"error", 0, std::vector<matlab::data::Array>({factory.createScalar(str)}));\
}


// Internal implementation of PRINT_BUFFER, it prints with speicifed length, casting, amount, and seperators
#define PRINT_BUFFER_UNTIL(str, len, u, cast, amt, sep){\
    size_t pbu___counter = 0;\
    for (char *it = str; it != str + len && *it != u; ++it) {\
        if (cast) {\
            std::cout<<int_to_hex(*it);\
        }\
        else {\
            std::cout<<*it;\
        }\
        if (sep) std::cout<<'/';\
        pbu___counter++;\
        if (pbu___counter % 5 == 0 && amt != -1){\
            std::cout<<'\n';\
        }\
        if (pbu___counter >= amt && amt != -1) break;\
    }\
    std::cout<<'\n';\
}

// Simpler definition of PRINT_BUFFER_UNTIL with printing characters in mind
#define PRINT_BUFFER(str, len, cast){\
    PRINT_BUFFER_UNTIL(str, len, '\0', cast, -1, false);\
}

#define COMP(a0, a1, len){\
    for (size_t i = 0; i < len; ++i) {\
        if (a0[i] != a1[i]) {\
            std::cout<<"("<<i<<"): data = "<<a0[i]<<"\n";\
        }\
    }\
}

#define CUSTOM_READ_DATA "--RDATA"

#define BUFFER_CAP 1 << 20  // Allocate 1 MB on stack memory

class MexFunction : public matlab::mex::Function  {
    public:
        void operator()(matlab::mex::ArgumentList output, matlab::mex::ArgumentList input) {
            data.clear();

            // check to see if arguments are valid
            check_arguments(output, input);

            // define our custom command
            std::string custom_cmd_r = CUSTOM_READ_DATA;

            // convert data to std::string (easier to manipulate)
            matlab::data::CharArray ca_gpib_addr = std::move(input[0]);
            matlab::data::CharArray ca_cmd       = std::move(input[1]);
            matlab::data::CharArray ca_type      = std::move(input[2]);

            std::string gpib_addr_str = CharArray_To_String(ca_gpib_addr);
            std::string cmd_str       = CharArray_To_String(ca_cmd);
            std::string type_str      = CharArray_To_String(ca_type);

            // define the access_mode
            ViAccessMode access_mode = VI_NULL;

            // Define timeout length
            ViUInt32 timeout_ms      = 5000;

            // define the communication buffer with set buffer size (this is different than main buffer)
            const ViUInt32 BUFSIZE = BUFFER_CAP;
            char buffer[BUFSIZE] = {0};
            const char comp[BUFSIZE] = {0};
            ViInt32 io_bytes;

            // Create a resource manager
            ViSession resource_manager;
            ViStatus  status;
            status    = viOpenDefaultRM(&resource_manager);
            if (status < VI_SUCCESS) {
                PRINT_ERR("Could not open VISA resource manager.");
                return;
            }

            // Connect to the GPIB instrument
            ViSession instrument;
            status = viOpen(resource_manager, gpib_addr_str.c_str(), access_mode, timeout_ms, &instrument);
            if (status < VI_SUCCESS) {
                viStatusDesc(resource_manager, status, buffer);
                PRINT_BUFFER(buffer, BUFSIZE, false);
                PRINT_ERR("Could not connect to instrument.");
                return;
            }
            else {
                std::cout<<"Instrument: "<<gpib_addr_str<<" has been connected.\n";
            }


            // setup timeout
            viSetAttribute(instrument, VI_ATTR_TMO_VALUE, timeout_ms);

            // setup EOL character
            viSetAttribute(instrument, VI_ATTR_TERMCHAR, '\n');

            // if it is the custom read command, utilize a special read function
            if (cmd_str == custom_cmd_r) {
                status = read_from (resource_manager, instrument, buffer, BUFSIZE, io_bytes);
            }
            else {
                // Write command to instrument
                status = viWrite(instrument, (ViBuf)cmd_str.c_str(), (ViInt32)cmd_str.length(), (ViPUInt32)&io_bytes);
                if (status < VI_SUCCESS) {
                    viStatusDesc(resource_manager, status, buffer);
                    PRINT_BUFFER(buffer, BUFSIZE, false);
                    PRINT_ERR("Error writing to instrument");
                    viClose (instrument);   // bug fix: RELEASE CONTROL
                    return;
                }
                else {
                    std::cout<<"Instruction: "<<cmd_str<<" has been sent to instrument.\n";
                }
    
                // read from instrument
                status = viRead(instrument, (ViPBuf)buffer, BUFSIZE, (ViPUInt32)&io_bytes);
                if (status < VI_SUCCESS) {
                    std::cout<<"Before status, # bytes in buffer: "<<io_bytes<<'\n';;
                    PRINT_BUFFER_UNTIL(buffer, BUFSIZE, -1, true, io_bytes, true);
                    viStatusDesc(resource_manager, status, buffer);
                    PRINT_BUFFER(buffer, BUFSIZE, false);
                    PRINT_ERR("Error reading from instrument");
                    viClose (instrument);   // bug fix: RELEASE CONTROL
                    return;
                }
    
                if (io_bytes < BUFSIZE) {
                    buffer[io_bytes] = '\0';
                }
                else {
                    buffer[BUFSIZE] = '\0';
                }
            }

            // print out buffer
            if (status < VI_SUCCESS) {
                std::cout<<"Data acquisition length (in bytes): "<<io_bytes<<'\n';
                std::cout<<"Status byte = "<<status<<'\n';
                COMP(buffer, comp, BUFSIZE);
                std::cout<<gpib_addr_str<<": ";
                PRINT_BUFFER_UNTIL(buffer, BUFSIZE, -1, true, 10, true);
            }

            // prepare data into typed array and return to user in 'matlab' space
            matlab::data::TypedArray<BASE_TYPE> v = factory.createArray({1, data.size()}, data.begin(), data.end());

            // Give the output to MATLAB space
            output[0] = std::move(v);

            // No need to clean-up heap data due to RAII, but clean-up should be done here
            // if necessary
            viClose (instrument);
        }
    private:

        /**
         *  @brief This checks to see if the arguments passed into operator() are valid.
         *  @param
         *   [&output] This is the output argument, a array[] type
         *   [&input]  These are the input arguments, specifically three strings
         *  @throw
         *   feval->error return matlab error when wrong arguments are passed in
         */
        void check_arguments(matlab::mex::ArgumentList &output, matlab::mex::ArgumentList &input) {
            // Check if argument size is correct
            if (input.size() != 3) {
                PRINT_ERR("Three inputs req'd");
            }
        }

        /**
         *  @brief Useful little helper function for printing out matlab's char array
         *
         */
        void array_printer(matlab::data::CharArray &arr) {
            for (auto c : arr)
                std::cout<<(char)c;
            std::cout<<'\n';
        }

        /**
         *  @brief Converts matlab's char array to a string in C++
         *
         */
        std::string CharArray_To_String(matlab::data::CharArray &arr) {
            std::string buffer;
            for (auto c : arr) buffer += (char)c;
            return buffer;
        }

        /**
         * @brief Casts integers into hexadecimals
         *        Referrenced from: https://stackoverflow.com/questions/5100718/integer-to-hex-string-in-c
         *        User: https://stackoverflow.com/users/233522/kornel-kisielewicz
         * 
         * 
         * @param i Integer
         * @return std::string 
         */
        std::string int_to_hex (int i) {
            std::stringstream stream;
            stream << "0x" << std::setfill ('0') << std::setw(sizeof(int)*2) << std::hex << i;
            return stream.str();
        }

        /**
         *  @brief Define a custom read data option
         *
         */
        ViStatus read_from(ViSession &rm, ViSession &instrument, char *buffer, const ViUInt32 &BUFSIZE, ViInt32 &io_bytes) {
            ViStatus status;
            char one_swp_cmd[]     = "SNGLS;TS;";   // When we sweep the trace is saved into trace_a
            char tdf_p[]           = "TDF P;";      // Utilize the parameter unit

            status = viWrite(instrument, (ViBuf)one_swp_cmd, (ViInt32)sizeof(one_swp_cmd), (ViPUInt32)&io_bytes);

            // We now need to get the data from the OSA (stored in trace_a)
            // The predefined trace ranges from 1 to any length between 3 and 2048 (typically 800)
            status = viWrite(instrument, (ViBuf)tdf_p, (ViInt32)sizeof(tdf_p), (ViPUInt32)&io_bytes);
      
            // We define our payload
            std::string q_cmd = "SPAN, TRA?;";
            status = viWrite (instrument, (ViBuf)q_cmd.c_str(), (ViInt32)q_cmd.length(), (ViPUInt32)&io_bytes);
            if (status < VI_SUCCESS) {
                viStatusDesc (rm, status, buffer);
                PRINT_BUFFER(buffer, BUFSIZE, false);
                return status;
            }

            status = viRead(instrument, (ViPBuf)buffer, BUFSIZE, (ViPUInt32)&io_bytes);
            if (status >= VI_SUCCESS) {
                parser(buffer, io_bytes);
            }
            else {
                std::cout<<"Before status, # bytes in buffer: "<<io_bytes<<'\n';;
                PRINT_BUFFER_UNTIL(buffer, BUFSIZE, -1, true, io_bytes, true);
                viStatusDesc(rm, status, buffer);
                PRINT_BUFFER(buffer, BUFSIZE, false);
                PRINT_ERR("Error reading from instrument with read_from()");
            }
            return status;
        }

        // Data is returned as comma-seperated list
        void parser(const char *buffer, const ViInt32 count) {
            std::string sbuffer;
            size_t i = 0, last_addr;
            BASE_TYPE value;
            while (i < count) {
                // When the data encountered is a comma, read from sbuffer
                if (buffer[i] == ',') {
                    value = std::stod(sbuffer.c_str(), &last_addr);
                    data.push_back(value);
                    sbuffer.clear();    // clear the sbuffer
                }
                else {
                    sbuffer += buffer[i];   // Else append to sbuffer
                }
                ++i;    // iterate through data
            }
            // edge case when the sbuffer is not empty
            if (sbuffer.length() > 0) {
                value = std::stod(sbuffer.c_str(), &last_addr);
                data.push_back(value);
            }
        }

        std::shared_ptr<matlab::engine::MATLABEngine> matlabPtr = getEngine();
        matlab::data::ArrayFactory factory;

        std::vector<BASE_TYPE> data;    // Intermediate data where the output is stored.
};
