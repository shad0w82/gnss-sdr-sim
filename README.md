# GPS-SDR-SIM

GPS-SDR-SIM generates GPS L1 C/A and Galileo E1 OS baseband signal data streams, which can be converted 
to RF using software-defined radio (SDR) platforms, such as 
[ADALM-Pluto](https://wiki.analog.com/university/tools/pluto), [bladeRF](http://nuand.com/), [HackRF](https://github.com/mossmann/hackrf/wiki), and [USRP](http://www.ettus.com/).
Both constellations share the 1575.42 MHz carrier, so a single output file carries GPS and Galileo
(see [Galileo E1](#galileo-e1)).

A user guide in Italian, with examples for every SDR front-end, is in [docs/GUIDA.md](docs/GUIDA.md).

### Windows build instructions

1. Start Visual Studio.
2. Create an empty project for a console application.
3. On the Solution Explorer at right, add "gpssim.c" and "getopt.c" to the Souce Files folder.
4. Select "Release" in Solution Configurations drop-down list.
5. Build the solution.

### Building with GCC

```
$ gcc gpssim.c -lm -O3 -o gps-sdr-sim
```

### Using bigger user motion files

In order to use user motion files with more than 30000 samples (at 10Hz), the `USER_MOTION_SIZE`
variable can be set to the maximum time of the user motion file in seconds. It is advisable to do
this using make so gps-sdr-bin can update the size when needed. e.g:

```
$ make USER_MOTION_SIZE=4000
```

This variable can also be set when compiling directly with GCC:

```
$ gcc gpssim.c -lm -O3 -o gps-sdr-sim -DUSER_MOTION_SIZE=4000
```

### Generating the GPS signal file

A user-defined trajectory can be specified in either a CSV file, which contains 
the Earth-centered Earth-fixed (ECEF) user positions, or an NMEA GGA stream.
The sampling rate of the user motion has to be 10Hz.
The user is also able to assign a static location directly through the command line.

The user specifies the GPS satellite constellation through a GPS broadcast 
ephemeris file. The daily GPS broadcast ephemeris file (brdc) is a merge of the
individual site navigation files into one. The archive for the daily file can 
be downloaded from: https://cddis.nasa.gov/archive/gnss/data/daily/. Access 
to this site requires registration, which is free.

These files are then used to generate the simulated pseudorange and
Doppler for the GPS satellites in view. This simulated range data is 
then used to generate the digitized I/Q samples for the GPS signal.

The bladeRF and ADALM-Pluto command line interface requires I/Q pairs stored as signed 
16-bit integers, while the hackrf_transfer and gps-sdr-sim-uhd.py
support signed bytes.

HackRF, bladeRF and ADALM-Pluto can accept the default sample rate of 2.6MHz, 
while the USRP2 requires an even integral decimator of 100 MHz, i.e. 2.5MHz.

The simulation start time can be specified if the corresponding set of ephemerides
is available. Otherwise the first time of ephemeris in the RINEX navigation file
is selected.

The maximum simulation duration time is defined by USER_MOTION_SIZE to 
prevent the output file from getting too large.

The output file size can be reduced by using "-b 1" option to store 
four 1-bit I/Q samples into a single byte. 
You can use [bladeplayer](https://github.com/osqzss/gps-sdr-sim/tree/master/player)
for bladeRF to playback the compressed file.

```
Usage: gps-sdr-sim [options]
Options:
  -e <gps_nav>     RINEX navigation file: RINEX 2 GPS, or RINEX 3 GPS/Galileo/mixed
  -D               Download the daily GPS/Galileo broadcast ephemerides of the scenario
                   date from BKG instead of -e (needs curl and gzip)
  -C <systems>     Constellations to simulate: G (GPS), E (Galileo), e.g. GE
                   (default: every constellation found in the navigation file)
  -u <user_motion> User motion file in ECEF x, y, z format (dynamic mode)
  -x <user_motion> User motion file in lat, lon, height format (dynamic mode)
  -g <nmea_gga>    NMEA GGA stream (dynamic mode)
  -c <location>    ECEF X,Y,Z in meters (static mode) e.g. 3967283.15,1022538.18,4872414.48
  -l <location>    Lat,Lon,Hgt (static mode) e.g. 30.286502,120.032669,100
  -L <wnslf,dn,dtslf> User leap future event in GPS week number, day number, next leap second e.g. 2347,3,19
  -t <date,time>   Scenario start time YYYY/MM/DD,hh:mm:ss
  -T <date,time>   Overwrite TOC and TOE to scenario start time
  -d <duration>    Duration [sec] (dynamic mode max: 300 static mode max: 86400)
  -o <output>      I/Q sampling data file (default: gpssim.bin ; use - for stdout)
  -s <frequency>   Sampling frequency [Hz] (default: 2600000, 5000000 with Galileo)
  -b <iq_bits>     I/Q data format [1/8/16] (default: 16)
  -i               Disable ionospheric delay for spacecraft scenario
  -a               Add tropospheric delay (Saastamoinen, standard atmosphere)
  -p [fixed_gain]  Disable path loss and hold power level constant
  -v               Show details about simulated channels
```

The user motion can be specified in either dynamic or static mode:

```
> gps-sdr-sim -e brdc3540.14n -u circle.csv
```

```
> gps-sdr-sim -e brdc3540.14n -x circle_llh.csv
```

```
> gps-sdr-sim -e brdc3540.14n -g triumphv3.txt
```

```
> gps-sdr-sim -e brdc3540.14n -l 30.286502,120.032669,100
```

### Galileo E1

Galileo satellites are simulated when the navigation file is a RINEX 3 file with Galileo
ephemerides, such as the daily merged broadcast file `BRDC00IGS_R_YYYYDDD0000_01D_MN.rnx`
(available from [CDDIS](https://cddis.nasa.gov/archive/gnss/data/daily/) or
[BKG](https://igs.bkg.bund.de/root_ftp/IGS/BRDC/)). GPS and Galileo are both simulated by default
when both are present; `-C` selects the constellations:

```
> gps-sdr-sim -e BRDC00IGS_R_20263000000_01D_MN.rnx -l 45.07,7.66,250 -t 2026/10/27,10:00:00
> gps-sdr-sim -e BRDC00IGS_R_20263000000_01D_MN.rnx -C E -l 45.07,7.66,250
> gps-sdr-sim -e BRDC00IGS_R_20263000000_01D_MN.rnx -C G -l 45.07,7.66,250
```

RINEX 2 GPS files keep working as before, and GPS output is unchanged.

#### Downloading the ephemerides automatically

With `-D` instead of `-e`, the daily mixed broadcast file is downloaded from the public
[BKG archive](https://igs.bkg.bund.de/root_ftp/IGS/BRDC/) (no registration) into the current
directory, using the `curl` and `gzip` commands (on Windows, `curl` is built in and `gzip` comes
for instance with Git for Windows):

```
> gps-sdr-sim -D -l 45.07,7.66,250 -t 2026/10/30,10:00:00   # file of 2026/10/30
> gps-sdr-sim -D -l 45.07,7.66,250 -T now                   # recent file, shifted to now
> gps-sdr-sim -D -l 45.07,7.66,250                          # file of today
```

* The date is the one of `-t` or `-T`, or today. With `-T` or without a date, the file of the
  previous day is tried too when the one of the scenario date is not available yet.
* The final file (`BRDC00IGS_R_...`) is used when published, and reused on later runs. For the
  current day, the file merged in real time (`BRDC00WRD_S_...`) is downloaded on every run, as it
  grows during the day. It only holds ephemerides up to the last hour or so: start a scenario
  "now" with `-T now`, which accepts the most recent ephemerides.

The simulated Galileo signal is:

* E1-B (data, I/NAV message) and E1-C (pilot, CS25 secondary code) with the primary codes of the
  Galileo OS SIS ICD (`galcodes.h`), on a BOC(1,1) sub-carrier. The BOC(6,1) part of CBOC, which
  carries 1/11 of the power, is not generated; receivers designed for CBOC track it with a small loss.
* I/NAV nominal pages on E1-B, with FEC encoding, interleaving, CRC-24Q and secondary synchronisation
  patterns. Word types 1 to 6 carry the ephemeris, clock, ionospheric, BGD, health, GST and GST-UTC
  data of the RINEX file, word types 7 to 10 an almanac derived from the ephemerides and the GST-GPS
  offset, and word type 0 the GST. Word types 16 to 22 (reduced CED, FEC2, ARAIM) are sent as spare
  words, the OSNMA field is empty and the SAR field carries spare data.
* Satellites flagged with an E1-B signal health status other than "OK" are not simulated.

Limitations:

* Galileo System Time is assumed equal to GPS time; the GST-GPS offset of the RINEX file is only
  broadcast.
* The ionospheric delay of every satellite follows the GPS Klobuchar model. A Galileo-only receiver
  applying NeQuick sees a residual error of a few meters.
* The BOC(1,1) spectrum needs a sampling frequency of at least 4 MHz, so the default becomes 5 MHz
  when Galileo is simulated. `bladeplayer`, `hackplayer` and `plutoplayer` replay at a fixed 2.6 MHz:
  use them with `-s 2600000`, or replay 5 MHz files with `hackrf_transfer -s 5000000`,
  `bladeRF-cli` (`set samplerate 5M`) or `limeplayer -s 5000000`.

### Tropospheric delay

By default no tropospheric delay is simulated, while receivers correct it with their own model.
The position they compute is then biased by a few meters, mostly in height. `-a` adds the
Saastamoinen delay with a standard atmosphere (15 °C and 1013.25 hPa at sea level, 70% relative
humidity, 1/cos(z) mapping), the default model of RTKLIB, to every GPS and Galileo satellite.
The remaining error depends on how close the receiver model is to this one.

### Transmitting the samples

The TX port of a particular SDR platform is connected to the GPS receiver 
under test through a DC block and a fixed 50-60dB attenuator.

#### BladeRF:

The simulated GPS signal file, named "gpssim.bin", can be loaded
into the bladeRF for playback as shown below:

```
set frequency 1575.42M
set samplerate 2.6M
set bandwidth 2.5M
set txvga1 -25
cal lms
cal dc tx
tx config file=gpssim.bin format=bin
tx start
```

You can also execute these commands via the `bladeRF-cli` script option as below:
```
> bladeRF-cli -s bladerf.script
```

#### HackRF:

The output data have to be 8-bit signed I/Q samples.
```
> gps-sdr-sim -e brdc0010.22n -b 8
```
You can use `hackrf_transfer` tool in the HackRF host software. 

```
> hackrf_transfer -t gpssim.bin -f 1575420000 -s 2600000 -a 1 -x 0
```

#### UHD supported devices (tested with USRP2 only):

```
> gps-sdr-sim-uhd.py -t gpssim.bin -s 2500000 -x 0
```

You can also use `tx_samples_from_file` tool included in the UHD examples:
```
> tx_samples_from_file --file gpssim.bin --type short --rate 2500000 --freq 1575420000 --gain 0
```

#### LimeSDR (in case of 1 Msps 1-bit file, to get full BaseBand dynamic and low RF power):

```
> limeplayer -s 1000000 -b 1 -d 2047 -g 0.1 < ../circle.1b.1M.bin
```

#### ADALM-Pluto (PlutoSDR):

The ADALM-Pluto device is expected to have its network interface up and running and is accessible
via "pluto.local" by default.

Default settings:
```
> plutoplayer -t gpssim.bin
```
Set TX attenuation:
```
> plutoplayer -t gpssim.bin -a -30.0
```
Default -20.0dB. Applicable range 0.0dB to -80.0dB in 0.25dB steps.

Set RF bandwidth:
```
> plutoplayer -t gpssim.bin -b 3.0
```
Default 3.0MHz. Applicable range 1.0MHz to 5.0MHz.

### License

Copyright &copy; 2015-2025 Takuji Ebinuma  
Distributed under the [MIT License](http://www.opensource.org/licenses/mit-license.php).
