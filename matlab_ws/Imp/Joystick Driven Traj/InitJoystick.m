clc
clear 
close all

%% Sampling Time

Ts = 1e-3; % fundamental sample time [sec]

%% NN Dimension

% system
n = 6; % number of system states
p = 3; % number of system inputs/outputs

% observer NN
inputLayerSizeObsv  = 2 * p + n;
hiddenLayerSizeObsv = 5;
outputLayerSizeObsv = p;

% Controller NN
inputLayerSizeCtrl  = n + p;
hiddenLayerSizeCtrl = 20;
outputLayerSizeCtrl = p;

%% Initial Conditions

% observer network weights init conds
Vobsv0 = 0.1 * randn(hiddenLayerSizeObsv, inputLayerSizeObsv);
Wobsv0 = 0.1 * randn(outputLayerSizeObsv, hiddenLayerSizeObsv);

% controller network weights init conds
Vctrl0 = 0.1 * randn(hiddenLayerSizeCtrl, inputLayerSizeCtrl);
Wctrl0 = 0.1 * randn(outputLayerSizeCtrl, hiddenLayerSizeCtrl);

% observer init cond
x_hat0 = zeros(n, 1);

%% Control Saturation

motor_max_torque = 2.2;
gear_ratio       = [1; 40/16; 70/16];

u_max =   motor_max_torque * gear_ratio;
u_min = - motor_max_torque * gear_ratio;

voltage_max    = 15;
torque2voltage = voltage_max ./ u_max;