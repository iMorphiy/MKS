clear all;
tab=csvread("ntc.csv");
temp=tab(:,1);
R_NTC=tab(:,2);

ADC_bit_word_size=10;
ADC_range=2^ADC_bit_word_size;

ADC_Val=ADC_range*(R_NTC./(R_NTC+10));

plot(ADC_Val, temp);
hold on
ADC_polynom=polyfit(ADC_Val, temp, 10);

ADC_Val_2=0:1023;
temp_2 = round(polyval(ADC_polynom, ADC_Val_2), 1);
hold on
plot(ADC_Val_2, temp_2, 'r');

dlmwrite('data.dlm', temp_2*10, ',');