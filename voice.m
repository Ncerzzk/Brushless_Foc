clear;
[data,fs]=audioread('2.mp3');
data=data(:,1);
num=size(data)
num=num(1)

freq=14400;  %²ÉÑùÂÊ =115200/8

for i = 1:floor(num/(fs/freq))
    result(i)=data((i-1)*(floor(fs/freq))+1);
end
K=1/max(result);
result=result*K;

%sound(result,freq);

mcu_result=result*128+127;  
% fid=fopen('voice.c','w');
% fprintf(fid,'#include "stdio.h"\r\n');
% num=size(mcu_result');
% num=num(1);
% fprintf(fid,'uint8_t voice[%d]={\r\n',num);
% 
% for i=1:size(mcu_result')
%     fprintf(fid,'%d,',round(mcu_result(i)));
%     if(mod(i,50)==0)
%         fprintf(fid,'\r\n');
%     end
% end
% fprintf(fid,'};');
% fclose(fid);

fid=fopen('voice.txt','w');
for i=1:size(mcu_result')
    temp=round(mcu_result(i));
    if(temp<0)
        temp=0;
    end
    fprintf(fid,'%d,',temp);
    if(mod(i,50)==0)
        fprintf(fid,'\r\n');
    end
end
fclose(fid);



