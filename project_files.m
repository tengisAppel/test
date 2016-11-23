%% Up-sampling
% z is the original baseband signal
N = length(z);
NN = 2^14; % Number of frequency grid points
f = (0:NN-1)/NN;
semilogy(f,abs(fft(z,NN))) % Check transform
xlabel('relative frequency f/fs');

%% Now upsample
R = 8; %
zu = zeros(N*R,1);
zu(1:R:end) = z;
semilogy(f,abs(fft(zu,NN))) % Check transform
xlabel('normalized frequency f/fs');

%% Design a LP interpolation filter
B = firpm(63,2*[0 0.5/R 0.5/R*1.6 1/2],[1 1 0 0]);
zi = conv(zu,B);
% zi is now the interpolated signal
figure(1)
plot([real(zi) imag(zi)])
figure(2)
semilogy(f,abs([fft(zu,NN) fft(zi,NN) fft(B.',NN)]) ) % Check transforms
legend('Up-sampled z_u','Interpolated after LP filtering','LP-filter')
xlabel('relative frequency f/fs');

%% Modulation
fs = 16000;
fcm = 4000;
F = (0:NN-1)/NN*fs;
n = (0:length(zi)-1)';
zm = zi.*exp(i*2*pi*fcm/fs*n);
semilogy(F,abs([fft(zi,NN) fft(zm,NN) ]) ) % Check transforms
legend('Interpolated','Modulated')
xlabel('Frequency (Hz)');

%% Make signal real
zmr = real(zm);
semilogy(F,abs([fft(zi,NN) fft(zm,NN) fft(zmr,NN) ]) ) % Check transforms
legend('Interpolated','Modulated','Real and modulated')
xlabel('Frequency (Hz)');



