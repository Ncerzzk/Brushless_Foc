rps=1; %1r/s
PAIRS=7;
PART_NUM=24;

interupt_time=rps*PAIRS*PART_NUM;
PSC=1281/interupt_time;

ARR=84000000/(PSC+1)/interupt_time;

Thetas=0:15:345;

for i=1:6
    U.theta=(i-1)*pi/3;
    U.x=cos(U.theta);
    U.y=sin(U.theta);
    Vects(i)=U;
end


for i=1:24
    theta=Thetas(i);
    theta_rad=theta/360*2*pi;
    index1=fix(theta/60)+1;
    index2=fix(theta/60)+2;
    if index2>6
        index2=1;
    end
    u1=Vects(index1);
    u2=Vects(index2);
    target=[2/3*cos(theta_rad);2/3*sin(theta_rad)];
    k=[u1.x u2.x;u1.y u2.y];
    t=inv(k)*target
end