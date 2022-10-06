function [o_arg1] = read_visa(GPIB_IDN, CMD, TYPE)
% If you want to get data from the visa device
% use CMD = "--RDATA" for read_data

lb = ask_visa(GPIB_IDN, '')

o_arg1 = rvisa_implt(GPIB_IDN, CMD, TYPE);
end

