%%
clear
clc

c = [-1-1i -1+1i 1-1i 1+1i]/sqrt(2); 

N = 128;
pack = randsrc(1,2*N, [0 1]);

m = buffer(pack,2);
m = bi2de(m','left-msb')+1;

s = c(m);

z = ifft(s,1024);

%%

channel = 1;
switch channel
    case 1 
        Nh = 60;
        n = 0:1:59;
        h = 0.8.^n;
        h = [h];
    case 2 % ej klar
        Nh = 100;
        h = zeros(1,Nh);
        h(1) = 0.5;
        h(9) = 0.5;
    case 3
        Nh = 60;
        n = 0:1:59;
        h = 0.8.^n;
        h = [h];
end
 
Ncp  = Nh+10;
cp = z(end-(Ncp-1):end);
zz = [cp z];

y_len = length(zz)+length(h)-1;

sigma = 0.01;
w = sigma/sqrt(2)*(randn(y_len,1) + 1i*randn(y_len,1))';
y = conv(h,zz) + w;

delay = 0;
r = fft(y(Ncp+1+delay:end-(Nh-1)+delay),N);
H = fft(h,length(r));
s_hat = conj(H).*r;

i = 1;
for k = 1:length(s_hat)
    b_hat(i) = sign(real(s_hat(k)));
    i = i + 1;
    b_hat(i) = sign(imag(s_hat(k)));
    i = i + 1;
end
b_hat(b_hat==-1) = 0;

isequal(pack,b_hat)
% pack == b_hat


%% crap below
figure();
plot(s,'or')
hold on
plot(s_hat,'xb')
plot(r,'+g')
legend('s(k)','s_{hat}(k)','r(k)')

bit_string = '';
for i = 1:length(pack)
    bit_string = [bit_string num2str(b_hat(i))];
end
disp(bit_string)
bit_string = '';
for i = 1:length(pack)
    bit_string = [bit_string num2str(pack(i))];
end
disp(bit_string)

fprintf('Lost bits: %u\n', (2*N-sum(pack == b_hat))./(2*N))

