% 计算各个扇区的T1 T2公式
% 用以加快每一步的SVPWM计算

U1.c=0;U1.b=0;U1.a=1;
U2.c=0;U2.b=1;U2.a=0;
U3.c=0;U3.b=1;U3.a=1;
U4.c=1;U4.b=0;U4.a=0;
U5.c=1;U5.b=0;U5.a=1;
U6.c=1;U6.b=1;U6.a=0;
Vects=[U4;U6;U2;U3;U1;U5];

digits(6);
for i=1:6
    Vects(i).theta=(i-1)*pi/3;
end

for i=1:6
    utemp1=Vects(i);
    if i+1==7
        utemp2=Vects(1);
    else
        utemp2=Vects(i+1);
    end
    syms alpha beta;
    syms T1 T2;
    equ1=alpha-T1*cos(utemp1.theta)-T2*cos(utemp2.theta);
    equ2=beta-T1*sin(utemp1.theta)-T2*sin(utemp2.theta);
    i
    [T1 T2]=solve(equ1,equ2,T1,T2);
    vpa([T1 T2])
end

