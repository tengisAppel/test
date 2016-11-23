clear variables
close all
clc

c = [-1-1i -1+1i 1-1i 1+1i]/sqrt(2); 

% Signal to be transmitter
N = 128;
pack = randsrc(1,2*N, [0 1]);

m = buffer(pack,2);
m = bi2de(m','left-msb')+1;

s = c(m);
z = ifft(s);

% Traning signal
Ntrain = N; % Should be larger then Nh
pack_train = randsrc(1,2*Ntrain, [0 1]);

m_train = buffer(pack_train,2);
m_train = bi2de(m_train','left-msb')+1;

s_train = c(m_train);
z_train = ifft(s_train);

% Choose the which channel the signal is trasmitted over
channel = 1;
switch channel
    case 1 
        Nh = 60; 
        n = 0:1:59;
        h = 0.8.^n;
    case 2 
        Nh = 100;
        h = zeros(1,Nh);
        h(1) = 0.5;
        h(9) = 0.5;
end
 
% Choose the length of the cyclic prefix, should be longer then Nh
Ncp  = 32;

% Magnitude
sigma = 0;

% Assemble the correct z_train
Ntraincp = Ncp;
z  = [z_train(end-(Ntraincp-1):end) z_train z(end-(Ncp-1):end) z];

y_len = length(z)+length(h)-1;


%% Up-sampling
% z_frame is the original baseband signal
N = length(z);
NN = 2^14;
f = (0:NN-1)/NN;

% Visualisera
figure()
plot(f,abs(fft(z,NN)))
figure()
semilogy(f,abs(fft(z,NN)));
xlabel('relative frequency f/f_s')

%% Now up-sample
R = 8;
zu = zeros(N*R,1);
zu(1:R:end) = z;
figure()
semilogy(f,abs(fft(zu,NN)));
xlabel('relative frequency f/fs')

%% Design a LP interpolation filer
B = firpm(63,2*[0 0.5/R 0.5/R*1.6 0.5],[1 1 0 0])
zi = conv(zu,B);
% zi is noe the interpolated signal
figure()
plot([real(zi) imag(zi)])

figure()
semilogy(f,abs([fft(zu,NN) fft(zi,NN) fft(B.',NN)]))
legend('Up-sampeled z_u','Interpolated after LP filtering','LP-filter')
xlabel('realtive frequency f/f_s')

%% Modulation
fs = 16000;
fcm = 4000;
F = (0:NN-1)/NN*fs;
n = (0:length(zi)-1)';

zm = zi.*exp(i*2*pi*fcm/fs*n);
semilogy(F,abs([fft(zi,NN) fft(zm,NN)]))
legend('Interpolated','Modulated')
xlabel('Frequency (Hz)')

% Make signal real
zmr = real(zm);
semilogy(F,abs([fft(zi,NN) fft(zm,NN) fft(zmr,NN)]))
legend('Interpolated','Modulated','Real and modulated')
xlabel('Frequency (Hz)')


%%
% % Calculate the effect of the channel and the noise of the signal
% w = sigma/sqrt(2)*(randn(y_len,1) + 1i*randn(y_len,1))';
% y = conv(h,z_frame) + w;
% 
% % Extract the correct signals in the train
% y_train = y(Ntraincp+1:Ntraincp+Ntrain)
% y_signal = y(Ntraincp+Ntrain+Ncp+1:end-(Nh-1))
% 
% % OFDM^-1 of the train signal
% r_train = fft(y_train);
% 
% % OFDM^-1 of the signal
% r = fft(y_signal);
% 
% % Approximate channel
% H_train = r_train./s_train
% H = H_train;
% 
% % Equalization
% s_hat = conj(H).*r;
% 
% % Decode the bits
% i = 1;
% for k = 1:length(s_hat)
%     b_hat(i) = sign(real(s_hat(k)));
%     i = i + 1;
%     b_hat(i) = sign(imag(s_hat(k)));
%     i = i + 1;
% end
% b_hat(b_hat==-1) = 0;
% 
% isequal(pack,b_hat)
% % pack == b_hat
% 
% 
% %% crap below
% figure();
% plot(s,'or')
% hold on
% plot(s_hat,'xb')
% plot(r,'+g')
% legend('s(k)','s_{hat}(k)','r(k)')
% 
% bit_string = '';
% for i = 1:length(pack)
%     bit_string = [bit_string num2str(b_hat(i))];
% end
% disp(bit_string)
% bit_string = '';
% for i = 1:length(pack)
%     bit_string = [bit_string num2str(pack(i))];
% end
% disp(bit_string)
% 
% fprintf('Lost bits: %u\n', 2*N-sum(pack == b_hat))