syms ia ib;
ic=-ia-ib;

alpha=ia+ib*cos(2*pi/3)+ic*cos(4*pi/3);
beta=ib*sin(2*pi/3)+ic*sin(4*pi/3);

pretty(alpha*2/3)
pretty(beta*2/3)