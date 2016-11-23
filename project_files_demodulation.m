%% Demodulation
fs = 16000;
fcm = 4000;
F = (0:NN-1)/NN*fs;
n = (0:length(zi)-1)';
yrec = zmr; % The received real valued signal
yib = yrec.*exp(-i*2*pi*fcm/fs*n);
semilogy(F,abs([fft(y,NN) fft(yib,NN) ]) ) % Check transforms
legend('Modulated','Demodulated')
xlabel('Frequency (Hz)')

yi = conv(yib,B);
% yi is now a lowpass signal
figure(2)
semilogy(f,abs([fft(yib,NN) fft(yi,NN) fft(B.',NN)]) ) % Check transforms
legend('Demodulated','after LP filtering','LP-filter')
xlabel('relative frequency f/fs');


%% Down-sampling
D = 8; % Same as upsampling rate!!
y = yi(1:D:end);
semilogy(f,abs([fft(y,NN)]) ) % Check transforms
%% Check in time domain
plot(real(y))
