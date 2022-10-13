function [data, space] = read_8565e (GPIB_IDN, CMD)
% If you want to get data from the visa device
% use CMD = "--RDATA" for read_data
% although the command asks for type, the TYPE COMMAND is not used

fa = ask_visa(GPIB_IDN, 'FA?');
fb = ask_visa(GPIB_IDN, 'FB?');



data = rvisa_implt(GPIB_IDN, CMD, 'NOT_USED');

len   = length(data);
space = linspace(fa, fb, len);
end