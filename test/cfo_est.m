function cfo_estimation()
    % Time-domain CP based method and Frequency-domain (Moose/Classen) method
    clear, clf
    CFO = 0.15;
    Nfft = 128; % FFT size
    Nbps = 2; M = 2^Nbps; % Number of bits per (modulated) symbol
    Es = 1; A = sqrt(3/2/(M-1)*Es); % Signal energy and QAM normalization factor
    N = Nfft; Ng = Nfft/4; Nofdm = Nfft + Ng; Nsym = 3;
    x = []; % Transmit signal

    for m = 1:Nsym
        msgint = randi([0, M-1], 1, N); % Генерация случайных целых чисел
        if m <= 2
            Xp = add_pilot(zeros(1, Nfft), Nfft, 4); 
            Xf = Xp; % add_pilot
        else 
            Xf = A * qammod(msgint, M, 'gray'); % Используем qammod
        end
        xt = ifft(Xf, Nfft); % IFFT
        x_sym = add_CP(xt, Ng); % add CP
        x = [x x_sym];
    end

    y = x; % No channel effect
    sig_pow = y * y' / length(y); % Signal power calculation
    SNRdBs = 0:3:30; MaxIter = 100;

    for i = 1:length(SNRdBs)
        SNRdB = SNRdBs(i);
        MSE_CFO_CP = 0; 
        MSE_CFO_Moose = 0; 
        MSE_CFO_Classen = 0;

        rng(1); % Initialize seed for random number
        y_CFO = add_CFO(y, CFO, Nfft); % Add CFO

        for iter = 1:MaxIter
            y_aw = awgn(y_CFO, SNRdB, 'measured'); % AWGN added
            CFO_est_CP = CFO_CP(y_aw, Nfft, Ng); % CP-based
            MSE_CFO_CP = MSE_CFO_CP + (CFO_est_CP - CFO)^2;

            CFO_est_Moose = CFO_Moose(y_aw, Nfft); % Moose
            MSE_CFO_Moose = MSE_CFO_Moose + (CFO_est_Moose - CFO)^2;

            CFO_est_Classen = CFO_Classen(y_aw, Nfft, Ng, Xp); % Classen
            MSE_CFO_Classen = MSE_CFO_Classen + (CFO_est_Classen - CFO)^2;
        end % End of (iter) loop

        MSE_CP(i) = MSE_CFO_CP / MaxIter; 
        MSE_Moose(i) = MSE_CFO_Moose / MaxIter;
        MSE_Classen(i) = MSE_CFO_Classen / MaxIter;
    end % End of SNR loop

    semilogy(SNRdBs, MSE_CP, '-+'), grid on, hold on
    semilogy(SNRdBs, MSE_Moose, '-x'), semilogy(SNRdBs, MSE_Classen, '-*')
    xlabel('SNR[dB]'), ylabel('MSE'); title('CFO Estimation');
    legend('CP-based technique', 'Moose', 'Classen')
end

function CFO_est = CFO_CP(y, Nfft, Ng)
    % Time-domain CFO estimation based on CP (Cyclic Prefix)
    nn = 1:Ng; 
    CFO_est = angle(y(nn + Nfft) * y(nn)') / (2 * pi); % Eq.(5.27)
end

function CFO_est = CFO_Moose(y, Nfft)
    % Frequency-domain CFO estimation using Moose method
    % based on two consecutive identical preambles (OFDM symbols)
    for i = 0:1
        Y(i + 1, :) = fft(y(Nfft * i + 1:Nfft * (i + 1)), Nfft); 
    end
    CFO_est = angle(Y(2, :) * Y(1, :)') / (2 * pi); % Eq.(5.30)
end

function CFO_est = CFO_Classen(yp, Nfft, Ng, Nps)
    % Frequency-domain CFO estimation using Classen method
    % based on pilot tones in two consecutive OFDM symbols
    if length(Nps) == 1
        Xp = add_pilot(zeros(1, Nfft), Nfft, Nps); % Pilot signal
    else
        Xp = Nps; % If Nps is an array, it must be a pilot sequence Xp
    end
    Nofdm = Nfft + Ng; 
    kk = find(Xp ~= 0); % Extract pilot tones
    Xp = Xp(kk); 
    
    for i = 1:2
        yp_without_CP = remove_CP(yp(1 + Nofdm * (i - 1):Nofdm * i), Ng);
        Yp(i, :) = fft(yp_without_CP, Nfft);
    end
    
    CFO_est = angle(Yp(2, kk) .* Xp * (Yp(1, kk) .* Xp)') / (2 * pi); % Eq.(5.31)
    CFO_est = CFO_est * Nfft / Nofdm; % Eq.(5.31)
end

function xp = add_pilot(x, Nfft, Nps)
    % CAZAC (Constant Amplitude Zero AutoCorrelation) sequence –> pilot
    % Nps : Pilot spacing
    if nargin < 3
        Nps = 4; % Установите значение по умолчанию
    end
    Np = Nfft / Nps; % Number of pilots
    xp = x; % Prepare an OFDM signal including pilot signal for initialization
    for k = 1:Np
        % Добавьте пилотные сигналы в xp
        xp((k-1)*Nps + 1) = 1; % Пример добавления пилота
    end
end

function x_cp = add_CP(x, Ng)
    % Добавление циклического префикса
    x_cp = [x(end-Ng+1:end), x]; % Добавляем CP
end

function x_no_cp = remove_CP(x_cp, Ng)
    % Удаление циклического префикса
    x_no_cp = x_cp(Ng+1:end); % Удаляем CP
end

function y_cfo = add_CFO(y, CFO, Nfft)
    % Добавление частотной несогласованности (CFO)
    t = (0:length(y)-1)';
    y_cfo = y .* exp(1j * 2 * pi * CFO * t / Nfft); % CFO добавление
end
