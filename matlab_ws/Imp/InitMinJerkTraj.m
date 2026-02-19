clc
clear 
close all

%% NN Dimension

n = 6; % number of system states
p = 3; % number of system inputs/outputs

inputLayerSizeObsv  = 2 * p + n;
hiddenLayerSizeObsv = 5;
outputLayerSizeObsv = p;

inputLayerSizeCtrl  = n + p;
hiddenLayerSizeCtrl = 20;
outputLayerSizeCtrl = p;

%% Path Generation (Min Jerk)

q1WayPoints = [0, 0.75,  0.25,  -0.7, 0];
q2WayPoints = [0,  0.5,  -0.3,  0.45, 0];
q3WayPoints = [0, -0.3,   0.1,  0.25, 0];

wayPoints  = [q1WayPoints; q2WayPoints; q3WayPoints];
timePoints = [0, 15, 35, 50, 70];
% timePoints = [0, 12, 27, 44, 60];

%% Initial Conditions

% observer network weights init conds
Vobsv0 = 0.1 * randn(hiddenLayerSizeObsv, inputLayerSizeObsv);
Wobsv0 = 0.1 * randn(outputLayerSizeObsv, hiddenLayerSizeObsv);

% controller network weights init conds
Vctrl0 = 0.1 * randn(hiddenLayerSizeCtrl, inputLayerSizeCtrl);
Wctrl0 = 0.1 * randn(outputLayerSizeCtrl, hiddenLayerSizeCtrl);

% system init cond
x0 = zeros(n, 1);

% observer init cond
x_hat0 = x0;

%% Control Saturation

motor_max_torque = 2.2;
gear_ratio = [1; 40/16; 70/16];

u_max =   motor_max_torque * gear_ratio;
u_min = - motor_max_torque * gear_ratio;

voltage_max    = 15;
torque2voltage = voltage_max ./ u_max;

Ts = 1e-3;
pp = 14;



% Note: Changing the PD gains
% old vals --> 0.1, 15
% new vals --> 0.01, 20

