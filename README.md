# sjconv-cpp

A simple standalone convolver for JACK. It uses [FFTConvolver](https://github.com/HiFi-LoFi/FFTConvolver) for convolution. This is a C++ version of my [sjconv](https://github.com/fstxz/sjconv).

Runtime dependencies:
  - jack2
  - libsndfile

## Usage
```
sjconv-cpp [--help] [--version] --file VAR [--ports VAR]

Optional arguments:
  -h, --help     shows help message and exits
  -v, --version  prints version information and exits
  -f, --file     path to the impulse response [required]
  -p, --ports    number of input/output channels [nargs=0..1] [default: 2]
```

## Building from source

Build dependencies:
  - a C++17 compiler
  - cmake
  - jack2
  - libsndfile

On Debian-based systems, you can install these with `apt install cmake libjack-jackd2-dev libsndfile1-dev`

```sh
git clone https://github.com/fstxz/sjconv-cpp.git
cd sjconv-cpp
cmake -B build
cmake --build build
```

The `sjconv-cpp` binary will be placed in the `build` directory.

## Limitations/assumptions

* Only mono impulse responses are supported
* Sample rate of the inpulse response must match the sample rate of the JACK server

## License

This program is licenced under the [GPLv3](https://github.com/fstxz/sjconv-cpp/blob/master/LICENSE.txt).
