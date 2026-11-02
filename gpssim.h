#ifndef GPSSIM_H
#define GPSSIM_H

//#define FLOAT_CARR_PHASE // For RKT simulation. Higher computational load, but smoother carrier phase.

#define TRUE	(1)
#define FALSE	(0)

/*! \brief Maximum length of a line in a text file (RINEX, motion) */
#define MAX_CHAR (100)

/*! \brief Maximum length of a file name */
#define MAX_PATH_LEN (1024)

/*! \brief GNSS constellation identifiers */
#define SYS_GPS (0)
#define SYS_GAL (1)

/*! \brief Maximum number of satellites in RINEX file */
#define MAX_SAT (32)

/*! \brief Maximum number of Galileo satellites (SVID 1..36) */
#define MAX_SAT_GAL (36)

/*! \brief Maximum number of Galileo ephemeris records per satellite */
#define MAX_GAL_EPH (300)

/*! \brief Maximum number of channels we simulate */
#define MAX_CHAN (32)

/*! \brief Maximum number of user motion points */
#ifndef USER_MOTION_SIZE
#define USER_MOTION_SIZE (3000) // max duration at 10Hz
#endif

/*! \brief Maximum duration for static mode*/
#define STATIC_MAX_DURATION (86400) // second

/*! \brief Number of subframes */
#define N_SBF (5) // 5 subframes per frame

/*! \brief Number of words per subframe */
#define N_DWRD_SBF (10) // 10 word per subframe

/*! \brief Number of words */
#define N_DWRD ((N_SBF+1)*N_DWRD_SBF) // Subframe word buffer size

/*! \brief C/A code sequence length */
#define CA_SEQ_LEN (1023)

/*! \brief Galileo E1-B/C primary code length */
#define GAL_CODE_LEN (4092)

/*! \brief Galileo E1-C secondary code length (CS25_1) */
#define GAL_SEC_CODE_LEN (25)

/*! \brief Galileo I/NAV page part: 120 bits, 250 symbols (sync + encoded) per second */
#define GAL_PART_BITS (120)
#define GAL_PART_SYMS (250)

/*! \brief Galileo I/NAV page buffer: last page of the previous sub-frame + 15 pages */
#define GAL_N_PAGE (16)
#define GAL_SYM_BUF (GAL_N_PAGE*2*GAL_PART_SYMS)

/*! \brief GST week number at the GPS week number 1024 epoch */
#define GAL_WEEK_OFFSET (1024)

/*! \brief Galileo E1 amplitude relative to GPS C/A: sqrt(10^(1.25/10)/2)
 *  (E1-B + E1-C at -157.25 dBW vs. C/A at -158.5 dBW) */
#define GAL_E1_AMP (0.8167)

#define SECONDS_IN_WEEK 604800.0
#define SECONDS_IN_HALF_WEEK 302400.0
#define SECONDS_IN_DAY 86400.0
#define SECONDS_IN_HOUR 3600.0
#define SECONDS_IN_MINUTE 60.0

#define POW2_M5  0.03125
#define POW2_M19 1.907348632812500e-6
#define POW2_M29 1.862645149230957e-9
#define POW2_M31 4.656612873077393e-10
#define POW2_M33 1.164153218269348e-10
#define POW2_M43 1.136868377216160e-13
#define POW2_M55 2.775557561562891e-17

#define POW2_M50 8.881784197001252e-016
#define POW2_M30 9.313225746154785e-010
#define POW2_M27 7.450580596923828e-009
#define POW2_M24 5.960464477539063e-008

// Conventional values employed in GPS ephemeris model (ICD-GPS-200)
#define GM_EARTH 3.986005e14
// Galileo ephemeris model (Galileo OS SIS ICD)
#define GM_EARTH_GAL 3.986004418e14
#define OMEGA_EARTH 7.2921151467e-5
#define PI 3.1415926535898

#define WGS84_RADIUS	6378137.0
#define WGS84_ECCENTRICITY 0.0818191908426

#define R2D 57.2957795131

#define SPEED_OF_LIGHT 2.99792458e8
#define LAMBDA_L1 0.190293672798365

/*! \brief GPS L1 Carrier frequency */
#define CARR_FREQ (1575.42e6)
/*! \brief C/A code frequency */
#define CODE_FREQ (1.023e6)
#define CARR_TO_CODE (1.0/1540.0)

// Sampling data format
#define SC01 (1)
#define SC08 (8)
#define SC16 (16)

#define EPHEM_ARRAY_SIZE (15) // for daily GPS broadcast ephemers file (brdc)

/*! \brief Structure representing GPS time */
typedef struct
{
	int week;	/*!< GPS week number (since January 1980) */
	double sec; 	/*!< second inside the GPS \a week */
} gpstime_t;

/*! \brief Structure repreenting UTC time */
typedef struct
{
	int y; 		/*!< Calendar year */
	int m;		/*!< Calendar month */
	int d;		/*!< Calendar day */
	int hh;		/*!< Calendar hour */
	int mm;		/*!< Calendar minutes */
	double sec;	/*!< Calendar seconds */
} datetime_t;

/*! \brief Structure representing ephemeris of a single satellite */
typedef struct
{
	int vflg;	/*!< Valid Flag */
	datetime_t t;
	gpstime_t toc;	/*!< Time of Clock */
	gpstime_t toe;	/*!< Time of Ephemeris */
	int iodc;	/*!< Issue of Data, Clock */
	int iode;	/*!< Isuse of Data, Ephemeris */
	double deltan;	/*!< Delta-N (radians/sec) */
	double cuc;	/*!< Cuc (radians) */
	double cus;	/*!< Cus (radians) */
	double cic;	/*!< Correction to inclination cos (radians) */
	double cis;	/*!< Correction to inclination sin (radians) */
	double crc;	/*!< Correction to radius cos (meters) */
	double crs;	/*!< Correction to radius sin (meters) */
	double ecc;	/*!< e Eccentricity */
	double sqrta;	/*!< sqrt(A) (sqrt(m)) */
	double m0;	/*!< Mean anamoly (radians) */
	double omg0;	/*!< Longitude of the ascending node (radians) */
	double inc0;	/*!< Inclination (radians) */
	double aop;
	double omgdot;	/*!< Omega dot (radians/s) */
	double idot;	/*!< IDOT (radians/s) */
	double af0;	/*!< Clock offset (seconds) */
	double af1;	/*!< rate (sec/sec) */
	double af2;	/*!< acceleration (sec/sec^2) */
	double tgd;	/*!< Group delay L2 bias (Galileo: BGD E1-E5b) */
	int svhlth;
	int codeL2;	/*!< Codes on L2 (Galileo: data sources) */
	double bgda;	/*!< Galileo: BGD E1-E5a (seconds) */
	double sisa;	/*!< Galileo: signal in space accuracy (meters) */
	// Working variables follow
	double n; 	/*!< Mean motion (Average angular velocity) */
	double sq1e2;	/*!< sqrt(1-e^2) */
	double A;	/*!< Semi-major axis */
	double omgkdot; /*!< OmegaDot-OmegaEdot */
} ephem_t;

typedef struct
{
	int enable;
	int vflg;
	double alpha0,alpha1,alpha2,alpha3;
	double beta0,beta1,beta2,beta3;
	double A0,A1;
	int dtls,tot,wnt;
	int dtlsf,dn,wnlsf;
	// enable custom leap event
	int leapen;
	// enable tropospheric delay
	int tropo;
} ionoutc_t;

/*! \brief Galileo ionospheric, GST-UTC and GST-GPS parameters */
typedef struct
{
	int vflg;
	double ai0,ai1,ai2;	/*!< NeQuick effective ionisation level coefficients */
	double A0,A1;		/*!< GST-UTC polynomial */
	int tot,wnt;		/*!< GST-UTC reference time (wnt: GPS-aligned week) */
	double A0G,A1G;		/*!< GST-GPS polynomial */
	int t0g,wn0g;		/*!< GGTO reference time (wn0g: GPS-aligned week) */
} galutc_t;

/*! \brief Galileo almanac of a single satellite (derived from ephemeris) */
typedef struct
{
	int svid;	/*!< 0 = unused entry */
	double dsqrta;	/*!< sqrt(A) - sqrt(A_nominal) */
	double ecc;
	double aop;	/*!< radians */
	double di;	/*!< i - i_nominal (radians) */
	double omg0;	/*!< radians */
	double omgdot;	/*!< radians/s */
	double m0;	/*!< radians at t0a */
	double af0,af1;
	int hs;		/*!< E1-B/E5b health status */
} galalm_t;

typedef struct
{
	gpstime_t g;
	double range; // pseudorange
	double rate;
	double d; // geometric distance
	double azel[2];
	double iono_delay;
	double tropo_delay;
} range_t;

/*! \brief Structure representing a Channel */
typedef struct
{
	int prn;	/*< PRN Number (0 = free channel) */
	int sys;	/*< Constellation (SYS_GPS, SYS_GAL) */
	int code_len;	/*< Primary code length in chips */
	int ca[GAL_CODE_LEN]; /*< C/A sequence (0/1), or E1-B code (+1/-1) */
	int pilot[GAL_CODE_LEN]; /*< E1-C code (+1/-1) */
	double f_carr;	/*< Carrier frequency */
	double f_code;	/*< Code frequency */
#ifdef FLOAT_CARR_PHASE
	double carr_phase;
#else
	unsigned int carr_phase; /*< Carrier phase */
	int carr_phasestep;	/*< Carrier phasestep */
#endif
	double code_phase; /*< Code phase */
	gpstime_t g0;	/*!< GPS time at start */
	unsigned long sbf[5][N_DWRD_SBF]; /*!< current subframe */
	unsigned long dwrd[N_DWRD]; /*!< Data words of sub-frame */
	int iword;	/*!< initial word */
	int ibit;	/*!< initial bit */
	int icode;	/*!< initial code */
	int dataBit;	/*!< current data bit */
	int codeCA;	/*!< current C/A code */
	unsigned char sym[GAL_SYM_BUF]; /*!< Galileo I/NAV symbols (logic 0/1) */
	int isym;	/*!< Galileo: current symbol (= primary code period) */
	int secCode;	/*!< Galileo: current E1-C secondary code chip (+1/-1) */
	double azel[2];
	range_t rho0;
} channel_t;

#endif
