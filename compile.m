function compile()
% -L<lib-PATH>
mex -L"C:\Program Files\IVI Foundation\VISA\Win64\Lib_x64\msc" -lvisa32.lib rvisa_implt.cpp;
mex -L"C:\Program Files\IVI Foundation\VISA\Win64\Lib_x64\msc" -lvisa32.lib qvisa_implt.cpp;
end

