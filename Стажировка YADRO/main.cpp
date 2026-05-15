#include <iostream>
#include <vector>
#include <cmath>
#include <numbers>
#include <complex>

class SinusGenerator {
private:
    double _freq;
    double _amp;
    double _samp;
    double _diff_phase;
    double _sin;
    double _cos;
    double _diff_sin;
    double _diff_cos;

public:
    SinusGenerator(double freq, double amp) {
        if ((freq < 0.0) || (freq > 50.0)) {
            throw std::invalid_argument("Frequency range: [0, 50]");
        }
        if (amp < 0.0) {
            throw std::invalid_argument("Amplitude > 0");
        }
        _freq = freq;
        _amp = amp;
        _samp = 100.0;
        _diff_phase = 2.0 * std::numbers::pi * _freq/ _samp;
        _sin = 0.0;
        _cos = 1.0;
        _diff_sin = std::sin(_diff_phase);
        _diff_cos = std::cos(_diff_phase);
    }
    
    double generate() {
        double value = _amp * _sin;
        double next_sin = _sin * _diff_cos + _cos * _diff_sin;
        double next_cos = _cos * _diff_cos - _sin * _diff_sin;
        _sin = next_sin;
        _cos = next_cos;

        return value;
    }
};


class Quantization {
private:
    double _int_max;

public:
    Quantization(double amp) {
        if (amp < 0.0) {
            throw std::invalid_argument("Amplitude > 0");
        }
        _int_max = INT16_MAX / amp;
    }

    int16_t quantization(double sin_value) {
        double value = sin_value * _int_max;
        value = std::round(value);
        if (value > INT16_MAX) {
            value = INT16_MAX;
        }
        if (value < INT16_MIN) {
            value = INT16_MIN;
        }

        return static_cast<int16_t>(value);
    }

    double recon(int16_t quantized_sin_value) {
        return static_cast<double>(quantized_sin_value) / _int_max;
    }
};



std::vector<double> interpolation(std::vector<double> vec) {
    std::vector<double> interpolated_vec;
    for (int i = 0; i < vec.size() - 1; i++) {
        interpolated_vec.push_back(vec[i]);
        interpolated_vec.push_back((vec[i]+vec[i+1])/2);
    }
    interpolated_vec.push_back(vec.back());
    
    return interpolated_vec;
}


std::vector<int16_t> interpolation(std::vector<int16_t> vec) {
    std::vector<int16_t> interpolated_vec;
    for (int i = 0; i < vec.size() - 1; i++) {
        interpolated_vec.push_back(vec[i]);
        interpolated_vec.push_back(static_cast<int16_t>(std::round((vec[i] + vec[i + 1]) / 2)));
    }
    interpolated_vec.push_back(vec.back());

    return interpolated_vec;
}


void spectrum(std::vector<double> interpolated_sin_values, double samp, std::vector<double>& freqs, std::vector<double>& amplitudes) {
    int n = interpolated_sin_values.size();
    for (int i = 0; i < n / 2; i++) {
        std::complex<double> sum(0.0, 0.0);
        double samp_i = static_cast<double>(i) * samp / static_cast<double>(n);
        for (int j = 0; j < n; j++) {
            double angle = -2.0 * std::numbers::pi * static_cast<double>(i) * static_cast<double>(j) / static_cast<double>(n);
            std::complex<double> exp(std::cos(angle), -std::sin(angle));
            sum += static_cast<std::complex<double>>(interpolated_sin_values[j] * exp);
        } 
        double amp = std::abs(sum) * 2.0 / static_cast<double>(n);
        if (i == 0) {
            amp /= 2.0;
        }
        freqs.push_back(samp_i);
        amplitudes.push_back(amp);
    }
}


void interpolation_quality(std::vector<double> interpolated_sin_values, double samp, double freq) {
    std::vector<double> freqs;
    std::vector<double> amplitudes;
    spectrum(interpolated_sin_values, samp, freqs, amplitudes);

    double main_amp = 0.0;
    int main_amp_ind = 0;
    for (int i = 0; i < amplitudes.size(); i++) {
        if (amplitudes[i] > main_amp) {
            main_amp = amplitudes[i];
            main_amp_ind = i;
        }
    }
    double noise = 0.0;
    for (int i = 0; i < amplitudes.size(); i++) {
        if (i != main_amp_ind) {
            noise += amplitudes[i] * amplitudes[i];
        }
    }
    double noise_amp = std::sqrt(noise/ amplitudes.size());
    double quality = 0.0;
    quality = 20.0 * std::log10(main_amp / noise_amp);
    std::cout << "Interpolation Quality: " << quality << " dB\n";
}


void spectrum_int(std::vector<int16_t> interpolated_int_sin_values, double samp, std::vector<double>& freqs, std::vector<double>& amplitudes) {
    int n = interpolated_int_sin_values.size();
    for (int i = 0; i < n / 2; i++) {
        std::complex<double> sum(0.0, 0.0);
        double samp_i = static_cast<double>(i) * samp / static_cast<double>(n);
        for (int j = 0; j < n; j++) {
            double angle = -2.0 * std::numbers::pi * static_cast<double>(i) * static_cast<double>(j) / static_cast<double>(n);
            std::complex<double> exp(std::cos(angle), -std::sin(angle));
            sum += static_cast<std::complex<double>>(static_cast<double>(interpolated_int_sin_values[j]) * exp);
        }
        double amp = std::abs(sum) * 2.0 / static_cast<double>(n);
        if (i == 0) {
            amp /= 2.0;
        }
        freqs.push_back(samp_i);
        amplitudes.push_back(amp);
    }
}


void interpolation_quality_int(std::vector<int16_t> interpolated_int_sin_values, double samp, double freq) {
    std::vector<double> freqs;
    std::vector<double> amplitudes;
    spectrum_int(interpolated_int_sin_values, samp, freqs, amplitudes);

    double main_amp = 0.0;
    int main_amp_ind = 0;
    for (int i = 0; i < amplitudes.size(); i++) {
        if (amplitudes[i] > main_amp) {
            main_amp = amplitudes[i];
            main_amp_ind = i;
        }
    }
    double noise = 0.0;
    for (int i = 0; i < amplitudes.size(); i++) {
        if (i != main_amp_ind) {
            noise += amplitudes[i] * amplitudes[i];
        }
    }
    double noise_amp = std::sqrt(noise / amplitudes.size());
    double quality = 0.0;
    quality = 20.0 * std::log10(main_amp / noise_amp);
    std::cout << "Quant Interpolation Quality: " << quality << " dB\n";
}


int main(){
    double freq = 4.0;
    double amp = 1.0;
    SinusGenerator sinGen(freq, amp);
    Quantization quant(amp);

    int n = 100;
    std::vector<double> sin_values;
    std::vector<int16_t> quantized_sin_values;
    std::vector<double> recon_sin_values;

    double max_abs_err = 0.0;
    double rms_err = 0.0;
    double sum_sin = 0.0;

    for (int i = 0; i < n; i++) {
        double val = sinGen.generate();
        int16_t quant_val = quant.quantization(val);
        double recon_val = quant.recon(quant_val);

        double abs_err = std::abs(val - recon_val);
        if (abs_err > max_abs_err) {
            max_abs_err = abs_err;
        }
        rms_err += abs_err * abs_err;
        sum_sin += val * val;

        sin_values.push_back(val);
        quantized_sin_values.push_back(quant_val);
        recon_sin_values.push_back(recon_val);
    }
    rms_err = std::sqrt(rms_err / n);
    double rms_sin = std::sqrt(sum_sin / n);
    double SNR = 20 * std::log10(rms_sin / rms_err);

    std::cout << "RMS Error: " << rms_err << "\n";
    std::cout << "Max ABS Error: " << max_abs_err << "\n";
    std::cout << "SNR: " << SNR << " dB\n";

    std::vector<double> interpolated_sin_values = interpolation(sin_values);
    std::vector<int16_t> interpolated_int_sin_values = interpolation(quantized_sin_values);


    interpolation_quality(interpolated_sin_values, 200.0, freq);
    interpolation_quality_int(interpolated_int_sin_values, 200.0, freq);
}