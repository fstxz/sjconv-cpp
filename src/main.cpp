#include <iostream>
#include <jack/jack.h>
#include <ostream>
#include <sndfile.h>
#include <string>
#include <thread>
#include <vector>

#include "FFTConvolver/FFTConvolver.h"
#include "argparse.hpp"

using fftconvolver::FFTConvolver;

std::vector<jack_port_t *> input_ports;
std::vector<jack_port_t *> output_ports;
std::vector<float> ir;
std::vector<FFTConvolver *> convolvers;

int process(jack_nframes_t nframes, void *arg) {
    for (int i = 0; i < input_ports.size(); i++) {
        auto in = (jack_default_audio_sample_t *)jack_port_get_buffer(
            input_ports[i], nframes);
        auto out = (jack_default_audio_sample_t *)jack_port_get_buffer(
            output_ports[i], nframes);
        convolvers[i]->process(in, out, nframes);
    }

    return 0;
}

void jack_shutdown(void *arg) { exit(1); }

int main(int argc, char *argv[]) {
    argparse::ArgumentParser program("sjconv-cpp", "0.1.0");

    program.add_argument("-f", "--file")
        .required()
        .help("path to the impulse response");

    program.add_argument("-p", "--ports")
        .help("number of input/output channels")
        .default_value(2)
        .scan<'i', int>();

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error &err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    std::string filepath = program.get<std::string>("--file");
    int port_num = program.get<int>("--ports");

    if (port_num == 0) {
        std::cerr << "Number of ports must be more than 0" << std::endl;
        return 1;
    }

    SF_INFO sfinfo;

    SNDFILE *sndfile = sf_open(filepath.data(), SFM_READ, &sfinfo);

    if (!sndfile) {
        std::cerr << "Error opening \"" << filepath
                  << "\": " << sf_strerror(sndfile) << std::endl;
        return 1;
    }

    long num_samples = sfinfo.frames * sfinfo.channels;

    std::vector<float> audio_data(num_samples);

    long frames_read = sf_read_float(sndfile, audio_data.data(), num_samples);

    if (frames_read != sfinfo.frames) {
        std::cerr << "Error reading all frames. Read: " << frames_read
                  << " Expected: " << sfinfo.frames << std::endl;
    }

    sf_close(sndfile);

    if (sfinfo.channels != 1) {
        std::cerr << "Impulse response must have only 1 channel" << std::endl;
    }

    ir = audio_data;

    jack_options_t options = JackNullOption;
    jack_status_t status;

    jack_client_t *client;

    client = jack_client_open("sjconv-cpp", options, &status, NULL);
    if (client == NULL) {
        std::cerr << "jack_client_open() failed, status: " << status
                  << std::endl;
        if (status & JackServerFailed) {
            std::cerr << "Unable to connect to JACK server" << std::endl;
        }
        return 1;
    }

    auto jack_sample_rate = jack_get_sample_rate(client);

    if (sfinfo.samplerate != jack_sample_rate) {
        std::cerr << "Sample rate of the inpulse response must match the "
                     "sample rate of the JACK server"
                  << std::endl;
        return 1;
    }

    jack_set_process_callback(client, process, 0);
    jack_on_shutdown(client, jack_shutdown, 0);

    auto buffer_size = jack_get_buffer_size(client);

    for (int i = 0; i < port_num; i++) {
        auto in_port = jack_port_register(
            client, ("input." + std::to_string(i + 1)).data(),
            JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
        auto out_port = jack_port_register(
            client, ("output." + std::to_string(i + 1)).data(),
            JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

        if ((in_port == NULL) || (out_port == NULL)) {
            std::cerr << "no more JACK ports available" << std::endl;
            return 1;
        }

        input_ports.push_back(in_port);
        output_ports.push_back(out_port);

        FFTConvolver *convolver = new FFTConvolver();
        convolver->init(buffer_size, ir.data(), ir.size());
        convolvers.push_back(convolver);
    }

    if (jack_activate(client)) {
        std::cerr << "couldn't activate client" << std::endl;
        return 1;
    }

    std::cout << "Started" << std::endl;
    std::this_thread::sleep_for(
        std::chrono::high_resolution_clock::duration::max());

    jack_client_close(client);
    return 0;
}
