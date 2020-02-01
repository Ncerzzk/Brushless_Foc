clear;
[data,fs]=audioread('1.m4a');
data=data(:,1);
num=size(data)
num=num(1)

freq=9000;  %²ÉÑùÂÊ

for i = 1:num/(fs/freq)
    result(i)=data((i-1)*(round(fs/freq))+1);
end
K=1/max(result);
result=result*K;

sound(result,freq);

mcu_result=result*255+128;  
fid=fopen('voice.c','w');
fprintf(fid,'#include "stdio.h"\r\n');
num=size(mcu_result');
num=num(1);
fprintf(fid,'uint8_t voice[%d]={\r\n',num);

for i=1:size(mcu_result')
    fprintf(fid,'%d,',round(mcu_result(i)));
    if(mod(i,50)==0)
        fprintf(fid,'\r\n');
    end
end
fprintf(fid,'};');
fclose(fid);



