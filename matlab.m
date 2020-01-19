rps=1; %1r/s
PAIRS=7;
PART_NUM=3600;

interupt_time=rps*PAIRS*PART_NUM;
PSC=1281/interupt_time;

ARR=84000000/(PSC+1)/interupt_time;

Thetas=0:360/PART_NUM:360;
Target=ones(1,PART_NUM+1);

U1.c=0;U1.b=0;U1.a=1;
U2.c=0;U2.b=1;U2.a=0;
U3.c=0;U3.b=1;U3.a=1;
U4.c=1;U4.b=0;U4.a=0;
U5.c=1;U5.b=0;U5.a=1;
U6.c=1;U6.b=1;U6.a=0;
Vects=[U4;U6;U2;U3;U1;U5];

for i=1:6
    theta=(i-1)*pi/3;
    Vects(i).theta=theta;
    Vects(i).x=cos(theta);
    Vects(i).y=sin(theta);
end


for i=1:PART_NUM
    theta=Thetas(i);
    theta_rad=theta/360*2*pi;
    area=floor(theta/60)+1; %theta ²»»á³¬¹ı360¡ã
    index1=area;
    index2=area+1;
    if(index2>6)
        index2=1;
    end
    u1=Vects(index1);
    u2=Vects(index2);
    target=[sqrt(3)/2*cos(theta_rad);sqrt(3)/2*sin(theta_rad)];
    k=[u1.x u2.x;u1.y u2.y];
    t=inv(k)*target;
    t0=(1-t(1)-t(2))/2;
    
    T(i)=t(1);
    T2(i)=t(2);
end

plot(T+T2);
