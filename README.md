# Feature Overview
* Accurate inductance, capacitance, and resistance measurements @ from 100Hz to 75kHz
* Automatic display of relevant component parameters, or ability to manually select 2 displayed parameters
* Support for frequency sweep plots
* Touchscreen and physical button interface
* Open and short probe calibration

# Specifications

## Frequency Range
* 100Hz - 75kHz in increments of 100Hz

## Autoranging
* TIA Gains: 100Ω, 1kΩ, 10kΩ, and 100kΩ
* ADC Input Amps: 1x, 5x, 25x, 100x


## Component Ranges


## Display Parameters
Parameter | Description
--- | ---
Rs  | Series resistance (ESR/ESL)
Rp  | Parallel resistance
Cs  | Series capacitance
Cp  | Parallel capacitance
Ls  | Series inductance
Lp  | Parallel inductance
Xs  | Series reactance
Q   | Quality factor
D   | Dissipation factor
\|Z\| | Impedance magnitude
∠Z | Impedance angle



## Automatic Parameter Selection

1pF to 10mF
1uH to 100H
1mΩ to 1MΩ

(actual ranges to be confirmed)


# Measurement Principle
https://www.desmos.com/calculator/hkwapyh1lp



To be placed later:

* Many thanks to the work of WeTec/wschuma. Much of the audio codec design and firmware was based on their work on EEVblog and Github
https://www.eevblog.com/forum/projects/teensylcr-a-teensy-based-diy-lcr-meter/
https://github.com/wschuma/TeensyLCR


* In order to maximize SNR, the system will automatically switch between 4 transimpedance amplifier gains. These gains are 100Ω, 1kΩ, 10kΩ, and 100kΩ. For derivative projects, I would recommend switching to reed relays instead of analog muxes. At high frequenies on the 100kΩ range, the input signal is heavily attenuated