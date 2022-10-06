function [data, space] = read_visa(GPIB_IDN, CMD)%, TYPE)
% If you want to get data from the visa device
% use CMD = "--RDATA" for read_data
% although the command asks for type, the TYPE COMMAND is not used

span = ask_visa(GPIB_IDN, 'SPANWL?;');
center = ask_visa(GPIB_IDN, 'CENTERWL?;');

% the center wavelength is half of the span
% lower bound is center - span/2
% upper bound is center + span/2
lb = center - (span/2);
ub = center + (span/2);

data = rvisa_implt(GPIB_IDN, CMD, 'NOT_USED');

len   = length(data);
space = linspace(lb, ub, len);

end

