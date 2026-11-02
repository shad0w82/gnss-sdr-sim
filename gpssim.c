#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#ifdef _WIN32
#include "getopt.h"
#else
#include <unistd.h>
#endif
#include "gpssim.h"
#include "galcodes.h"

int sinTable512[] = {
	   2,   5,   8,  11,  14,  17,  20,  23,  26,  29,  32,  35,  38,  41,  44,  47,
	  50,  53,  56,  59,  62,  65,  68,  71,  74,  77,  80,  83,  86,  89,  91,  94,
	  97, 100, 103, 105, 108, 111, 114, 116, 119, 122, 125, 127, 130, 132, 135, 138,
	 140, 143, 145, 148, 150, 153, 155, 157, 160, 162, 164, 167, 169, 171, 173, 176,
	 178, 180, 182, 184, 186, 188, 190, 192, 194, 196, 198, 200, 202, 204, 205, 207,
	 209, 210, 212, 214, 215, 217, 218, 220, 221, 223, 224, 225, 227, 228, 229, 230,
	 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 241, 242, 243, 244, 244, 245,
	 245, 246, 247, 247, 248, 248, 248, 249, 249, 249, 249, 250, 250, 250, 250, 250,
	 250, 250, 250, 250, 250, 249, 249, 249, 249, 248, 248, 248, 247, 247, 246, 245,
	 245, 244, 244, 243, 242, 241, 241, 240, 239, 238, 237, 236, 235, 234, 233, 232,
	 230, 229, 228, 227, 225, 224, 223, 221, 220, 218, 217, 215, 214, 212, 210, 209,
	 207, 205, 204, 202, 200, 198, 196, 194, 192, 190, 188, 186, 184, 182, 180, 178,
	 176, 173, 171, 169, 167, 164, 162, 160, 157, 155, 153, 150, 148, 145, 143, 140,
	 138, 135, 132, 130, 127, 125, 122, 119, 116, 114, 111, 108, 105, 103, 100,  97,
	  94,  91,  89,  86,  83,  80,  77,  74,  71,  68,  65,  62,  59,  56,  53,  50,
	  47,  44,  41,  38,  35,  32,  29,  26,  23,  20,  17,  14,  11,   8,   5,   2,
	  -2,  -5,  -8, -11, -14, -17, -20, -23, -26, -29, -32, -35, -38, -41, -44, -47,
	 -50, -53, -56, -59, -62, -65, -68, -71, -74, -77, -80, -83, -86, -89, -91, -94,
	 -97,-100,-103,-105,-108,-111,-114,-116,-119,-122,-125,-127,-130,-132,-135,-138,
	-140,-143,-145,-148,-150,-153,-155,-157,-160,-162,-164,-167,-169,-171,-173,-176,
	-178,-180,-182,-184,-186,-188,-190,-192,-194,-196,-198,-200,-202,-204,-205,-207,
	-209,-210,-212,-214,-215,-217,-218,-220,-221,-223,-224,-225,-227,-228,-229,-230,
	-232,-233,-234,-235,-236,-237,-238,-239,-240,-241,-241,-242,-243,-244,-244,-245,
	-245,-246,-247,-247,-248,-248,-248,-249,-249,-249,-249,-250,-250,-250,-250,-250,
	-250,-250,-250,-250,-250,-249,-249,-249,-249,-248,-248,-248,-247,-247,-246,-245,
	-245,-244,-244,-243,-242,-241,-241,-240,-239,-238,-237,-236,-235,-234,-233,-232,
	-230,-229,-228,-227,-225,-224,-223,-221,-220,-218,-217,-215,-214,-212,-210,-209,
	-207,-205,-204,-202,-200,-198,-196,-194,-192,-190,-188,-186,-184,-182,-180,-178,
	-176,-173,-171,-169,-167,-164,-162,-160,-157,-155,-153,-150,-148,-145,-143,-140,
	-138,-135,-132,-130,-127,-125,-122,-119,-116,-114,-111,-108,-105,-103,-100, -97,
	 -94, -91, -89, -86, -83, -80, -77, -74, -71, -68, -65, -62, -59, -56, -53, -50,
	 -47, -44, -41, -38, -35, -32, -29, -26, -23, -20, -17, -14, -11,  -8,  -5,  -2
};

int cosTable512[] = {
	 250, 250, 250, 250, 250, 249, 249, 249, 249, 248, 248, 248, 247, 247, 246, 245,
	 245, 244, 244, 243, 242, 241, 241, 240, 239, 238, 237, 236, 235, 234, 233, 232,
	 230, 229, 228, 227, 225, 224, 223, 221, 220, 218, 217, 215, 214, 212, 210, 209,
	 207, 205, 204, 202, 200, 198, 196, 194, 192, 190, 188, 186, 184, 182, 180, 178,
	 176, 173, 171, 169, 167, 164, 162, 160, 157, 155, 153, 150, 148, 145, 143, 140,
	 138, 135, 132, 130, 127, 125, 122, 119, 116, 114, 111, 108, 105, 103, 100,  97,
	  94,  91,  89,  86,  83,  80,  77,  74,  71,  68,  65,  62,  59,  56,  53,  50,
	  47,  44,  41,  38,  35,  32,  29,  26,  23,  20,  17,  14,  11,   8,   5,   2,
	  -2,  -5,  -8, -11, -14, -17, -20, -23, -26, -29, -32, -35, -38, -41, -44, -47,
	 -50, -53, -56, -59, -62, -65, -68, -71, -74, -77, -80, -83, -86, -89, -91, -94,
	 -97,-100,-103,-105,-108,-111,-114,-116,-119,-122,-125,-127,-130,-132,-135,-138,
	-140,-143,-145,-148,-150,-153,-155,-157,-160,-162,-164,-167,-169,-171,-173,-176,
	-178,-180,-182,-184,-186,-188,-190,-192,-194,-196,-198,-200,-202,-204,-205,-207,
	-209,-210,-212,-214,-215,-217,-218,-220,-221,-223,-224,-225,-227,-228,-229,-230,
	-232,-233,-234,-235,-236,-237,-238,-239,-240,-241,-241,-242,-243,-244,-244,-245,
	-245,-246,-247,-247,-248,-248,-248,-249,-249,-249,-249,-250,-250,-250,-250,-250,
	-250,-250,-250,-250,-250,-249,-249,-249,-249,-248,-248,-248,-247,-247,-246,-245,
	-245,-244,-244,-243,-242,-241,-241,-240,-239,-238,-237,-236,-235,-234,-233,-232,
	-230,-229,-228,-227,-225,-224,-223,-221,-220,-218,-217,-215,-214,-212,-210,-209,
	-207,-205,-204,-202,-200,-198,-196,-194,-192,-190,-188,-186,-184,-182,-180,-178,
	-176,-173,-171,-169,-167,-164,-162,-160,-157,-155,-153,-150,-148,-145,-143,-140,
	-138,-135,-132,-130,-127,-125,-122,-119,-116,-114,-111,-108,-105,-103,-100, -97,
	 -94, -91, -89, -86, -83, -80, -77, -74, -71, -68, -65, -62, -59, -56, -53, -50,
	 -47, -44, -41, -38, -35, -32, -29, -26, -23, -20, -17, -14, -11,  -8,  -5,  -2,
	   2,   5,   8,  11,  14,  17,  20,  23,  26,  29,  32,  35,  38,  41,  44,  47,
	  50,  53,  56,  59,  62,  65,  68,  71,  74,  77,  80,  83,  86,  89,  91,  94,
	  97, 100, 103, 105, 108, 111, 114, 116, 119, 122, 125, 127, 130, 132, 135, 138,
	 140, 143, 145, 148, 150, 153, 155, 157, 160, 162, 164, 167, 169, 171, 173, 176,
	 178, 180, 182, 184, 186, 188, 190, 192, 194, 196, 198, 200, 202, 204, 205, 207,
	 209, 210, 212, 214, 215, 217, 218, 220, 221, 223, 224, 225, 227, 228, 229, 230,
	 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 241, 242, 243, 244, 244, 245,
	 245, 246, 247, 247, 248, 248, 248, 249, 249, 249, 249, 250, 250, 250, 250, 250
};

// Receiver antenna attenuation in dB for boresight angle = 0:5:180 [deg]
double ant_pat_db[37] = {
	 0.00,  0.00,  0.22,  0.44,  0.67,  1.11,  1.56,  2.00,  2.44,  2.89,  3.56,  4.22,
	 4.89,  5.56,  6.22,  6.89,  7.56,  8.22,  8.89,  9.78, 10.67, 11.56, 12.44, 13.33,
	14.44, 15.56, 16.67, 17.78, 18.89, 20.00, 21.33, 22.67, 24.00, 25.56, 27.33, 29.33,
	31.56
};

// Galileo E1-C secondary code CS25_1
const int galCs25[GAL_SEC_CODE_LEN] = {
	0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0
};

// Allocated channel per satellite: GPS PRN 1..32 at [0..31], Galileo SVID 1..36 at [32..67]
int allocatedSat[MAX_SAT+MAX_SAT_GAL];

double xyz[USER_MOTION_SIZE][3];

// Galileo ephemeris records per satellite, and the set currently in use
ephem_t galeph[MAX_SAT_GAL][MAX_GAL_EPH];
int galneph[MAX_SAT_GAL];
ephem_t galcur[MAX_SAT_GAL];

galutc_t galutc;
galalm_t galalm[MAX_SAT_GAL];
int galt0a; // Almanac reference time of week [sec]
int galwna; // Almanac reference week (GPS-aligned)

/*! \brief Subtract two vectors of double
 *  \param[out] y Result of subtraction
 *  \param[in] x1 Minuend of subtraction
 *  \param[in] x2 Subtrahend of subtraction
 */
void subVect(double *y, const double *x1, const double *x2)
{
	y[0] = x1[0]-x2[0];
	y[1] = x1[1]-x2[1];
	y[2] = x1[2]-x2[2];

	return;
}

/*! \brief Compute Norm of Vector
 *  \param[in] x Input vector
 *  \returns Length (Norm) of the input vector
 */
double normVect(const double *x)
{
	return(sqrt(x[0]*x[0]+x[1]*x[1]+x[2]*x[2]));
}

/*! \brief Compute dot-product of two vectors
 *  \param[in] x1 First multiplicand
 *  \param[in] x2 Second multiplicand
 *  \returns Dot-product of both multiplicands
 */
double dotProd(const double *x1, const double *x2)
{
	return(x1[0]*x2[0]+x1[1]*x2[1]+x1[2]*x2[2]);
}

/* !\brief generate the C/A code sequence for a given Satellite Vehicle PRN
 *  \param[in] prn PRN number of the Satellite Vehicle
 *  \param[out] ca Caller-allocated integer array of 1023 bytes
 */
void codegen(int *ca, int prn)
{
	int delay[] = {
		  5,   6,   7,   8,  17,  18, 139, 140, 141, 251,
		252, 254, 255, 256, 257, 258, 469, 470, 471, 472,
		473, 474, 509, 512, 513, 514, 515, 516, 859, 860,
		861, 862};

	int g1[CA_SEQ_LEN], g2[CA_SEQ_LEN];
	int r1[N_DWRD_SBF], r2[N_DWRD_SBF];
	int c1, c2;
	int i,j;

	if (prn<1 || prn>32)
		return;

	for (i=0; i<N_DWRD_SBF; i++)
		r1[i] = r2[i] = -1;

	for (i=0; i<CA_SEQ_LEN; i++)
	{
		g1[i] = r1[9];
		g2[i] = r2[9];
		c1 = r1[2]*r1[9];
		c2 = r2[1]*r2[2]*r2[5]*r2[7]*r2[8]*r2[9];

		for (j=9; j>0; j--)
		{
			r1[j] = r1[j-1];
			r2[j] = r2[j-1];
		}
		r1[0] = c1;
		r2[0] = c2;
	}

	for (i=0,j=CA_SEQ_LEN-delay[prn-1]; i<CA_SEQ_LEN; i++,j++)
		ca[i] = (1-g1[i]*g2[j%CA_SEQ_LEN])/2;

	return;
}

/*! \brief Convert a UTC date into a GPS date
 *  \param[in] t input date in UTC form
 *  \param[out] g output date in GPS form
 */
void date2gps(const datetime_t *t, gpstime_t *g)
{
	int doy[12] = {0,31,59,90,120,151,181,212,243,273,304,334};
	int ye;
	int de;
	int lpdays;

	ye = t->y - 1980;

	// Compute the number of leap days since Jan 5/Jan 6, 1980.
	lpdays = ye/4 + 1;
	if ((ye%4)==0 && t->m<=2)
		lpdays--;

	// Compute the number of days elapsed since Jan 5/Jan 6, 1980.
	de = ye*365 + doy[t->m-1] + t->d + lpdays - 6;

	// Convert time to GPS weeks and seconds.
	g->week = de / 7;
	g->sec = (double)(de%7)*SECONDS_IN_DAY + t->hh*SECONDS_IN_HOUR
		+ t->mm*SECONDS_IN_MINUTE + t->sec;

	return;
}

void gps2date(const gpstime_t *g, datetime_t *t)
{
	// Convert Julian day number to calendar date
	int c = (int)(7*g->week + floor(g->sec/86400.0)+2444245.0) + 1537;
	int d = (int)((c-122.1)/365.25);
	int e = 365*d + d/4;
	int f = (int)((c-e)/30.6001);

	t->d = c - e - (int)(30.6001*f);
	t->m = f - 1 - 12*(f/14);
	t->y = d - 4715 - ((7 + t->m)/10);

	t->hh = ((int)(g->sec/3600.0))%24;
	t->mm = ((int)(g->sec/60.0))%60;
	t->sec = g->sec - 60.0*floor(g->sec/60.0);

	return;
}

/*! \brief Convert Earth-centered Earth-fixed (ECEF) into Lat/Long/Height
 *  \param[in] xyz Input Array of X, Y and Z ECEF coordinates
 *  \param[out] llh Output Array of Latitude, Longitude and Height
 */
void xyz2llh(const double *xyz, double *llh)
{
	double a,eps,e,e2;
	double x,y,z;
	double rho2,dz,zdz,nh,slat,n,dz_new;

	a = WGS84_RADIUS;
	e = WGS84_ECCENTRICITY;

	eps = 1.0e-3;
	e2 = e*e;

	if (normVect(xyz)<eps)
	{
		// Invalid ECEF vector
		llh[0] = 0.0;
		llh[1] = 0.0;
		llh[2] = -a;

		return;
	}

	x = xyz[0];
	y = xyz[1];
	z = xyz[2];

	rho2 = x*x + y*y;
	dz = e2*z;

	while (1)
	{
		zdz = z + dz;
		nh = sqrt(rho2 + zdz*zdz);
		slat = zdz / nh;
		n = a / sqrt(1.0-e2*slat*slat);
		dz_new = n*e2*slat;

		if (fabs(dz-dz_new) < eps)
			break;

		dz = dz_new;
	}

	llh[0] = atan2(zdz, sqrt(rho2));
	llh[1] = atan2(y, x);
	llh[2] = nh - n;

	return;
}

/*! \brief Convert Lat/Long/Height into Earth-centered Earth-fixed (ECEF)
 *  \param[in] llh Input Array of Latitude, Longitude and Height
 *  \param[out] xyz Output Array of X, Y and Z ECEF coordinates
 */
void llh2xyz(const double *llh, double *xyz)
{
	double n;
	double a;
	double e;
	double e2;
	double clat;
	double slat;
	double clon;
	double slon;
	double d,nph;
	double tmp;

	a = WGS84_RADIUS;
	e = WGS84_ECCENTRICITY;
	e2 = e*e;

	clat = cos(llh[0]);
	slat = sin(llh[0]);
	clon = cos(llh[1]);
	slon = sin(llh[1]);
	d = e*slat;

	n = a/sqrt(1.0-d*d);
	nph = n + llh[2];

	tmp = nph*clat;
	xyz[0] = tmp*clon;
	xyz[1] = tmp*slon;
	xyz[2] = ((1.0-e2)*n + llh[2])*slat;

	return;
}

/*! \brief Compute the intermediate matrix for LLH to ECEF
 *  \param[in] llh Input position in Latitude-Longitude-Height format
 *  \param[out] t Three-by-Three output matrix
 */
void ltcmat(const double *llh, double t[3][3])
{
	double slat, clat;
	double slon, clon;

	slat = sin(llh[0]);
	clat = cos(llh[0]);
	slon = sin(llh[1]);
	clon = cos(llh[1]);

	t[0][0] = -slat*clon;
	t[0][1] = -slat*slon;
	t[0][2] = clat;
	t[1][0] = -slon;
	t[1][1] = clon;
	t[1][2] = 0.0;
	t[2][0] = clat*clon;
	t[2][1] = clat*slon;
	t[2][2] = slat;

	return;
}

/*! \brief Convert Earth-centered Earth-Fixed to ?
 *  \param[in] xyz Input position as vector in ECEF format
 *  \param[in] t Intermediate matrix computed by \ref ltcmat
 *  \param[out] neu Output position as North-East-Up format
 */
void ecef2neu(const double *xyz, double t[3][3], double *neu)
{
	neu[0] = t[0][0]*xyz[0] + t[0][1]*xyz[1] + t[0][2]*xyz[2];
	neu[1] = t[1][0]*xyz[0] + t[1][1]*xyz[1] + t[1][2]*xyz[2];
	neu[2] = t[2][0]*xyz[0] + t[2][1]*xyz[1] + t[2][2]*xyz[2];

	return;
}

/*! \brief Convert North-East-Up to Azimuth + Elevation
 *  \param[in] neu Input position in North-East-Up format
 *  \param[out] azel Output array of azimuth + elevation as double
 */
void neu2azel(double *azel, const double *neu)
{
	double ne;

	azel[0] = atan2(neu[1],neu[0]);
	if (azel[0]<0.0)
		azel[0] += (2.0*PI);

	ne = sqrt(neu[0]*neu[0] + neu[1]*neu[1]);
	azel[1] = atan2(neu[2], ne);

	return;
}

/*! \brief Compute Satellite position, velocity and clock at given time
 *  \param[in] eph Ephemeris data of the satellite
 *  \param[in] g GPS time at which position is to be computed
 *  \param[out] pos Computed position (vector)
 *  \param[out] vel Computed velocity (vector)
 *  \param[clk] clk Computed clock
 */
void satpos(ephem_t eph, gpstime_t g, double *pos, double *vel, double *clk)
{
	// Computing Satellite Velocity using the Broadcast Ephemeris
	// http://www.ngs.noaa.gov/gps-toolbox/bc_velo.htm

	double tk;
	double mk;
	double ek;
	double ekold;
	double ekdot;
	double cek,sek;
	double pk;
	double pkdot;
	double c2pk,s2pk;
	double uk;
	double ukdot;
	double cuk,suk;
	double ok;
	double sok,cok;
	double ik;
	double ikdot;
	double sik,cik;
	double rk;
	double rkdot;
	double xpk,ypk;
	double xpkdot,ypkdot;

	double relativistic, OneMinusecosE, tmp;

	tk = g.sec - eph.toe.sec;

	if(tk>SECONDS_IN_HALF_WEEK)
		tk -= SECONDS_IN_WEEK;
	else if(tk<-SECONDS_IN_HALF_WEEK)
		tk += SECONDS_IN_WEEK;

	mk = eph.m0 + eph.n*tk;
	ek = mk;
	ekold = ek + 1.0;

	OneMinusecosE = 0; // Suppress the uninitialized warning.
	while(fabs(ek-ekold)>1.0E-14)
	{
		ekold = ek;
		OneMinusecosE = 1.0-eph.ecc*cos(ekold);
		ek = ek + (mk-ekold+eph.ecc*sin(ekold))/OneMinusecosE;
	}

	sek = sin(ek);
	cek = cos(ek);

	ekdot = eph.n/OneMinusecosE;

	relativistic = -4.442807633E-10*eph.ecc*eph.sqrta*sek;

	pk = atan2(eph.sq1e2*sek,cek-eph.ecc) + eph.aop;
	pkdot = eph.sq1e2*ekdot/OneMinusecosE;

	s2pk = sin(2.0*pk);
	c2pk = cos(2.0*pk);

	uk = pk + eph.cus*s2pk + eph.cuc*c2pk;
	suk = sin(uk);
	cuk = cos(uk);
	ukdot = pkdot*(1.0 + 2.0*(eph.cus*c2pk - eph.cuc*s2pk));

	rk = eph.A*OneMinusecosE + eph.crc*c2pk + eph.crs*s2pk;
	rkdot = eph.A*eph.ecc*sek*ekdot + 2.0*pkdot*(eph.crs*c2pk - eph.crc*s2pk);

	ik = eph.inc0 + eph.idot*tk + eph.cic*c2pk + eph.cis*s2pk;
	sik = sin(ik);
	cik = cos(ik);
	ikdot = eph.idot + 2.0*pkdot*(eph.cis*c2pk - eph.cic*s2pk);

	xpk = rk*cuk;
	ypk = rk*suk;
	xpkdot = rkdot*cuk - ypk*ukdot;
	ypkdot = rkdot*suk + xpk*ukdot;

	ok = eph.omg0 + tk*eph.omgkdot - OMEGA_EARTH*eph.toe.sec;
	sok = sin(ok);
	cok = cos(ok);

	pos[0] = xpk*cok - ypk*cik*sok;
	pos[1] = xpk*sok + ypk*cik*cok;
	pos[2] = ypk*sik;

	tmp = ypkdot*cik - ypk*sik*ikdot;

	vel[0] = -eph.omgkdot*pos[1] + xpkdot*cok - tmp*sok;
	vel[1] = eph.omgkdot*pos[0] + xpkdot*sok + tmp*cok;
	vel[2] = ypk*cik*ikdot + ypkdot*sik;

	// Satellite clock correction
	tk = g.sec - eph.toc.sec;

	if(tk>SECONDS_IN_HALF_WEEK)
		tk -= SECONDS_IN_WEEK;
	else if(tk<-SECONDS_IN_HALF_WEEK)
		tk += SECONDS_IN_WEEK;

	clk[0] = eph.af0 + tk*(eph.af1 + tk*eph.af2) + relativistic - eph.tgd;
	clk[1] = eph.af1 + 2.0*tk*eph.af2;

	return;
}

/*! \brief Compute Subframe from Ephemeris
 *  \param[in] eph Ephemeris of given SV
 *  \param[out] sbf Array of five sub-frames, 10 long words each
 */
void eph2sbf(const ephem_t eph, const ionoutc_t ionoutc, unsigned long sbf[5][N_DWRD_SBF])
{
	unsigned long wn;
	unsigned long toe;
	unsigned long toc;
	unsigned long iode;
	unsigned long iodc;
	long deltan;
	long cuc;
	long cus;
	long cic;
	long cis;
	long crc;
	long crs;
	unsigned long ecc;
	unsigned long sqrta;
	long m0;
	long omg0;
	long inc0;
	long aop;
	long omgdot;
	long idot;
	long af0;
	long af1;
	long af2;
	long tgd;
	int svhlth;
	int codeL2;

	unsigned long ura = 0UL;
	unsigned long dataId = 1UL;
	unsigned long sbf4_page25_svId = 63UL;
	unsigned long sbf5_page25_svId = 51UL;

	unsigned long wna;
	unsigned long toa;

	signed long alpha0,alpha1,alpha2,alpha3;
	signed long beta0,beta1,beta2,beta3;
	signed long A0,A1;
	signed long dtls;
	unsigned long tot,wnt,wnlsf,dtlsf,dn;
	unsigned long sbf4_page18_svId = 56UL;

	// FIXED: This has to be the "transmission" week number, not for the ephemeris reference time
	//wn = (unsigned long)(eph.toe.week%1024);
	wn = 0UL;
	toe = (unsigned long)(eph.toe.sec/16.0);
	toc = (unsigned long)(eph.toc.sec/16.0);
	iode = (unsigned long)(eph.iode);
	iodc = (unsigned long)(eph.iodc);
	deltan = (long)(eph.deltan/POW2_M43/PI);
	cuc = (long)(eph.cuc/POW2_M29);
	cus = (long)(eph.cus/POW2_M29);
	cic = (long)(eph.cic/POW2_M29);
	cis = (long)(eph.cis/POW2_M29);
	crc = (long)(eph.crc/POW2_M5);
	crs = (long)(eph.crs/POW2_M5);
	ecc = (unsigned long)(eph.ecc/POW2_M33);
	sqrta = (unsigned long)(eph.sqrta/POW2_M19);
	m0 = (long)(eph.m0/POW2_M31/PI);
	omg0 = (long)(eph.omg0/POW2_M31/PI);
	inc0 = (long)(eph.inc0/POW2_M31/PI);
	aop = (long)(eph.aop/POW2_M31/PI);
	omgdot = (long)(eph.omgdot/POW2_M43/PI);
	idot = (long)(eph.idot/POW2_M43/PI);
	af0 = (long)(eph.af0/POW2_M31);
	af1 = (long)(eph.af1/POW2_M43);
	af2 = (long)(eph.af2/POW2_M55);
	tgd = (long)(eph.tgd/POW2_M31);
	svhlth = (unsigned long)(eph.svhlth);
	codeL2 = (unsigned long)(eph.codeL2);

	wna = (unsigned long)(eph.toe.week%256);
	toa = (unsigned long)(eph.toe.sec/4096.0);

	alpha0 = (signed long)round(ionoutc.alpha0/POW2_M30);
	alpha1 = (signed long)round(ionoutc.alpha1/POW2_M27);
	alpha2 = (signed long)round(ionoutc.alpha2/POW2_M24);
	alpha3 = (signed long)round(ionoutc.alpha3/POW2_M24);
	beta0 = (signed long)round(ionoutc.beta0/2048.0);
	beta1 = (signed long)round(ionoutc.beta1/16384.0);
	beta2 = (signed long)round(ionoutc.beta2/65536.0);
	beta3 = (signed long)round(ionoutc.beta3/65536.0);
	A0 = (signed long)round(ionoutc.A0/POW2_M30);
	A1 = (signed long)round(ionoutc.A1/POW2_M50);
	dtls = (signed long)(ionoutc.dtls);
	tot = (unsigned long)(ionoutc.tot/4096);
	wnt = (unsigned long)(ionoutc.wnt%256);

	// 2016/12/31 (Sat) -> WNlsf = 1929, DN = 7 (http://navigationservices.agi.com/GNSSWeb/)
	// Days are counted from 1 to 7 (Sunday is 1).
	if (ionoutc.leapen==TRUE)
	{
		wnlsf = (unsigned long)(ionoutc.wnlsf%256);
		dn = (unsigned long)(ionoutc.dn);
		dtlsf = (unsigned long)(ionoutc.dtlsf);
	}
	else
	{
		wnlsf = 1929%256;
		dn = 7;
		dtlsf = 18;
	}
	// Subframe 1
	sbf[0][0] = 0x8B0000UL<<6;
	sbf[0][1] = 0x1UL<<8;
	sbf[0][2] = ((wn&0x3FFUL)<<20) | ((codeL2&0x3UL)<<18) | ((ura&0xFUL)<<14) | ((svhlth&0x3FUL)<<8) | (((iodc>>8)&0x3UL)<<6);
	sbf[0][3] = 0UL;
	sbf[0][4] = 0UL;
	sbf[0][5] = 0UL;
	sbf[0][6] = (tgd&0xFFUL)<<6;
	sbf[0][7] = ((iodc&0xFFUL)<<22) | ((toc&0xFFFFUL)<<6);
	sbf[0][8] = ((af2&0xFFUL)<<22) | ((af1&0xFFFFUL)<<6);
	sbf[0][9] = (af0&0x3FFFFFUL)<<8;

	// Subframe 2
	sbf[1][0] = 0x8B0000UL<<6;
	sbf[1][1] = 0x2UL<<8;
	sbf[1][2] = ((iode&0xFFUL)<<22) | ((crs&0xFFFFUL)<<6);
	sbf[1][3] = ((deltan&0xFFFFUL)<<14) | (((m0>>24)&0xFFUL)<<6);
	sbf[1][4] = (m0&0xFFFFFFUL)<<6;
	sbf[1][5] = ((cuc&0xFFFFUL)<<14) | (((ecc>>24)&0xFFUL)<<6);
	sbf[1][6] = (ecc&0xFFFFFFUL)<<6;
	sbf[1][7] = ((cus&0xFFFFUL)<<14) | (((sqrta>>24)&0xFFUL)<<6);
	sbf[1][8] = (sqrta&0xFFFFFFUL)<<6;
	sbf[1][9] = (toe&0xFFFFUL)<<14;

	// Subframe 3
	sbf[2][0] = 0x8B0000UL<<6;
	sbf[2][1] = 0x3UL<<8;
	sbf[2][2] = ((cic&0xFFFFUL)<<14) | (((omg0>>24)&0xFFUL)<<6);
	sbf[2][3] = (omg0&0xFFFFFFUL)<<6;
	sbf[2][4] = ((cis&0xFFFFUL)<<14) | (((inc0>>24)&0xFFUL)<<6);
	sbf[2][5] = (inc0&0xFFFFFFUL)<<6;
	sbf[2][6] = ((crc&0xFFFFUL)<<14) | (((aop>>24)&0xFFUL)<<6);
	sbf[2][7] = (aop&0xFFFFFFUL)<<6;
	sbf[2][8] = (omgdot&0xFFFFFFUL)<<6;
	sbf[2][9] = ((iode&0xFFUL)<<22) | ((idot&0x3FFFUL)<<8);

	if (ionoutc.vflg==TRUE)
	{
		// Subframe 4, page 18
		sbf[3][0] = 0x8B0000UL<<6;
		sbf[3][1] = 0x4UL<<8;
		sbf[3][2] = (dataId<<28) | (sbf4_page18_svId<<22) | ((alpha0&0xFFUL)<<14) | ((alpha1&0xFFUL)<<6);
		sbf[3][3] = ((alpha2&0xFFUL)<<22) | ((alpha3&0xFFUL)<<14) | ((beta0&0xFFUL)<<6);
		sbf[3][4] = ((beta1&0xFFUL)<<22) | ((beta2&0xFFUL)<<14) | ((beta3&0xFFUL)<<6);
		sbf[3][5] = (A1&0xFFFFFFUL)<<6;
		sbf[3][6] = ((A0>>8)&0xFFFFFFUL)<<6;
		sbf[3][7] = ((A0&0xFFUL)<<22) | ((tot&0xFFUL)<<14) | ((wnt&0xFFUL)<<6);
		sbf[3][8] = ((dtls&0xFFUL)<<22) | ((wnlsf&0xFFUL)<<14) | ((dn&0xFFUL)<<6);
		sbf[3][9] = (dtlsf&0xFFUL)<<22;

	}
	else
	{
		// Subframe 4, page 25
		sbf[3][0] = 0x8B0000UL<<6;
		sbf[3][1] = 0x4UL<<8;
		sbf[3][2] = (dataId<<28) | (sbf4_page25_svId<<22);
		sbf[3][3] = 0UL;
		sbf[3][4] = 0UL;
		sbf[3][5] = 0UL;
		sbf[3][6] = 0UL;
		sbf[3][7] = 0UL;
		sbf[3][8] = 0UL;
		sbf[3][9] = 0UL;
	}

	// Subframe 5, page 25
	sbf[4][0] = 0x8B0000UL<<6;
	sbf[4][1] = 0x5UL<<8;
	sbf[4][2] = (dataId<<28) | (sbf5_page25_svId<<22) | ((toa&0xFFUL)<<14) | ((wna&0xFFUL)<<6);
	sbf[4][3] = 0UL;
	sbf[4][4] = 0UL;
	sbf[4][5] = 0UL;
	sbf[4][6] = 0UL;
	sbf[4][7] = 0UL;
	sbf[4][8] = 0UL;
	sbf[4][9] = 0UL;

	return;
}

/*! \brief Count number of bits set to 1
 *  \param[in] v long word in which bits are counted
 *  \returns Count of bits set to 1
 */
unsigned long countBits(unsigned long v)
{
	unsigned long c;
	const int S[] = {1, 2, 4, 8, 16};
	const unsigned long B[] = {
		0x55555555, 0x33333333, 0x0F0F0F0F, 0x00FF00FF, 0x0000FFFF};

	c = v;
	c = ((c >> S[0]) & B[0]) + (c & B[0]);
	c = ((c >> S[1]) & B[1]) + (c & B[1]);
	c = ((c >> S[2]) & B[2]) + (c & B[2]);
	c = ((c >> S[3]) & B[3]) + (c & B[3]);
	c = ((c >> S[4]) & B[4]) + (c & B[4]);

	return(c);
}

/*! \brief Compute the Checksum for one given word of a subframe
 *  \param[in] source The input data
 *  \param[in] nib Does this word contain non-information-bearing bits?
 *  \returns Computed Checksum
 */
unsigned long computeChecksum(unsigned long source, int nib)
{
	/*
	Bits 31 to 30 = 2 LSBs of the previous transmitted word, D29* and D30*
	Bits 29 to  6 = Source data bits, d1, d2, ..., d24
	Bits  5 to  0 = Empty parity bits
	*/

	/*
	Bits 31 to 30 = 2 LSBs of the previous transmitted word, D29* and D30*
	Bits 29 to  6 = Data bits transmitted by the SV, D1, D2, ..., D24
	Bits  5 to  0 = Computed parity bits, D25, D26, ..., D30
	*/

	/*
	                  1            2           3
	bit    12 3456 7890 1234 5678 9012 3456 7890
	---    -------------------------------------
	D25    11 1011 0001 1111 0011 0100 1000 0000
	D26    01 1101 1000 1111 1001 1010 0100 0000
	D27    10 1110 1100 0111 1100 1101 0000 0000
	D28    01 0111 0110 0011 1110 0110 1000 0000
	D29    10 1011 1011 0001 1111 0011 0100 0000
	D30    00 1011 0111 1010 1000 1001 1100 0000
	*/

	unsigned long bmask[6] = {
		0x3B1F3480UL, 0x1D8F9A40UL, 0x2EC7CD00UL,
		0x1763E680UL, 0x2BB1F340UL, 0x0B7A89C0UL };

	unsigned long D;
	unsigned long d = source & 0x3FFFFFC0UL;
	unsigned long D29 = (source>>31)&0x1UL;
	unsigned long D30 = (source>>30)&0x1UL;

	if (nib) // Non-information bearing bits for word 2 and 10
	{
		/*
		Solve bits 23 and 24 to preserve parity check
		with zeros in bits 29 and 30.
		*/

		if ((D30 + countBits(bmask[4] & d)) % 2)
			d ^= (0x1UL<<6);
		if ((D29 + countBits(bmask[5] & d)) % 2)
			d ^= (0x1UL<<7);
	}

	D = d;
	if (D30)
		D ^= 0x3FFFFFC0UL;

	D |= ((D29 + countBits(bmask[0] & d)) % 2) << 5;
	D |= ((D30 + countBits(bmask[1] & d)) % 2) << 4;
	D |= ((D29 + countBits(bmask[2] & d)) % 2) << 3;
	D |= ((D30 + countBits(bmask[3] & d)) % 2) << 2;
	D |= ((D30 + countBits(bmask[4] & d)) % 2) << 1;
	D |= ((D29 + countBits(bmask[5] & d)) % 2);

	D &= 0x3FFFFFFFUL;
	//D |= (source & 0xC0000000UL); // Add D29* and D30* from source data bits

	return(D);
}

/*! \brief Replace all 'E' exponential designators to 'D'
 *  \param str String in which all occurrences of 'E' are replaced with *  'D'
 *  \param len Length of input string in bytes
 *  \returns Number of characters replaced
 */
int replaceExpDesignator(char *str, int len)
{
	int i,n=0;

	for (i=0; i<len; i++)
	{
		if (str[i]=='D')
		{
			n++;
			str[i] = 'E';
		}
	}

	return(n);
}

double subGpsTime(gpstime_t g1, gpstime_t g0)
{
	double dt;

	dt = g1.sec - g0.sec;
	dt += (double)(g1.week - g0.week) * SECONDS_IN_WEEK;

	return(dt);
}

gpstime_t incGpsTime(gpstime_t g0, double dt)
{
	gpstime_t g1;

	g1.week = g0.week;
	g1.sec = g0.sec + dt;

	g1.sec = round(g1.sec*1000.0)/1000.0; // Avoid rounding error

	while (g1.sec>=SECONDS_IN_WEEK)
	{
		g1.sec -= SECONDS_IN_WEEK;
		g1.week++;
	}

	while (g1.sec<0.0)
	{
		g1.sec += SECONDS_IN_WEEK;
		g1.week--;
	}

	return(g1);
}

/*! \brief Read Ephemeris data from the RINEX Navigation file */
/*  \param[out] eph Array of Output SV ephemeris data
 *  \param[in] fname File name of the RINEX file
 *  \returns Number of sets of ephemerides in the file
 */
int readRinexNavAll(ephem_t eph[][MAX_SAT], ionoutc_t *ionoutc, const char *fname)
{
	FILE *fp;
	int ieph;

	int sv;
	char str[MAX_CHAR];
	char tmp[20];

	datetime_t t;
	gpstime_t g;
	gpstime_t g0;
	double dt;

	int flags = 0x0;

	if (NULL==(fp=fopen(fname, "rt")))
		return(-1);

	// Clear valid flag
	for (ieph=0; ieph<EPHEM_ARRAY_SIZE; ieph++)
		for (sv=0; sv<MAX_SAT; sv++)
			eph[ieph][sv].vflg = 0;

	// Read header lines
	while (1)
	{
		if (NULL==fgets(str, MAX_CHAR, fp))
			break;

		if (strncmp(str+60, "END OF HEADER", 13)==0)
			break;
		else if (strncmp(str+60, "ION ALPHA", 9)==0)
		{
			strncpy(tmp, str+2, 12);
			tmp[12] = 0;
			replaceExpDesignator(tmp, 12);
			ionoutc->alpha0 = atof(tmp);

			strncpy(tmp, str+14, 12);
			tmp[12] = 0;
			replaceExpDesignator(tmp, 12);
			ionoutc->alpha1 = atof(tmp);

			strncpy(tmp, str+26, 12);
			tmp[12] = 0;
			replaceExpDesignator(tmp, 12);
			ionoutc->alpha2 = atof(tmp);

			strncpy(tmp, str+38, 12);
			tmp[12] = 0;
			replaceExpDesignator(tmp, 12);
			ionoutc->alpha3 = atof(tmp);

			//read wntlsf, dn, and dtlsf from fil

			flags |= 0x1;
		}
		else if (strncmp(str+60, "ION BETA", 8)==0)
		{
			strncpy(tmp, str+2, 12);
			tmp[12] = 0;
			replaceExpDesignator(tmp, 12);
			ionoutc->beta0 = atof(tmp);

			strncpy(tmp, str+14, 12);
			tmp[12] = 0;
			replaceExpDesignator(tmp, 12);
			ionoutc->beta1 = atof(tmp);

			strncpy(tmp, str+26, 12);
			tmp[12] = 0;
			replaceExpDesignator(tmp, 12);
			ionoutc->beta2 = atof(tmp);

			strncpy(tmp, str+38, 12);
			tmp[12] = 0;
			replaceExpDesignator(tmp, 12);
			ionoutc->beta3 = atof(tmp);

			flags |= 0x1<<1;
		}
		else if (strncmp(str+60, "DELTA-UTC", 9)==0)
		{
			strncpy(tmp, str+3, 19);
			tmp[19] = 0;
			replaceExpDesignator(tmp, 19);
			ionoutc->A0 = atof(tmp);

			strncpy(tmp, str+22, 19);
			tmp[19] = 0;
			replaceExpDesignator(tmp, 19);
			ionoutc->A1 = atof(tmp);

			strncpy(tmp, str+41, 9);
			tmp[9] = 0;
			ionoutc->tot = atoi(tmp);

			strncpy(tmp, str+50, 9);
			tmp[9] = 0;
			ionoutc->wnt = atoi(tmp);

			if (ionoutc->tot%4096==0)
				flags |= 0x1<<2;
		}
		else if (strncmp(str+60, "LEAP SECONDS", 12)==0)
		{
			strncpy(tmp, str, 6);
			tmp[6] = 0;
			ionoutc->dtls = atoi(tmp);

			flags |= 0x1<<3;
		}
	}

	ionoutc->vflg = FALSE;
	if (flags==0xF) // Read all Iono/UTC lines
		ionoutc->vflg = TRUE;

	// Read ephemeris blocks
	g0.week = -1;
	ieph = 0;

	while (1)
	{
		if (NULL==fgets(str, MAX_CHAR, fp))
			break;

		// PRN
		strncpy(tmp, str, 2);
		tmp[2] = 0;
		sv = atoi(tmp)-1;

		// EPOCH
		strncpy(tmp, str+3, 2);
		tmp[2] = 0;
		t.y = atoi(tmp) + 2000;

		strncpy(tmp, str+6, 2);
		tmp[2] = 0;
		t.m = atoi(tmp);

		strncpy(tmp, str+9, 2);
		tmp[2] = 0;
		t.d = atoi(tmp);

		strncpy(tmp, str+12, 2);
		tmp[2] = 0;
		t.hh = atoi(tmp);

		strncpy(tmp, str+15, 2);
		tmp[2] = 0;
		t.mm = atoi(tmp);

		strncpy(tmp, str+18, 4);
		tmp[2] = 0;
		t.sec = atof(tmp);

		date2gps(&t, &g);

		if (g0.week==-1)
			g0 = g;

		// Check current time of clock
		dt = subGpsTime(g, g0);

		if (dt>SECONDS_IN_HOUR)
		{
			g0 = g;
			ieph++; // a new set of ephemerides

			if (ieph>=EPHEM_ARRAY_SIZE)
				break;
		}

		// Date and time
		eph[ieph][sv].t = t;

		// SV CLK
		eph[ieph][sv].toc = g;

		strncpy(tmp, str+22, 19);
		tmp[19] = 0;
		replaceExpDesignator(tmp, 19); // tmp[15]='E';
		eph[ieph][sv].af0 = atof(tmp);

		strncpy(tmp, str+41, 19);
		tmp[19] = 0;
		replaceExpDesignator(tmp, 19);
		eph[ieph][sv].af1 = atof(tmp);

		strncpy(tmp, str+60, 19);
		tmp[19] = 0;
		replaceExpDesignator(tmp, 19);
		eph[ieph][sv].af2 = atof(tmp);

		// BROADCAST ORBIT - 1
		if (NULL==fgets(str, MAX_CHAR, fp))
			break;

		strncpy(tmp, str+3, 19);
		tmp[19] = 0;
		replaceExpDesignator(tmp, 19);
		eph[ieph][sv].iode = (int)atof(tmp);

		strncpy(tmp, str+22, 19);
		tmp[19] = 0;
		replaceExpDesignator(tmp, 19);
		eph[ieph][sv].crs = atof(tmp);

		strncpy(tmp, str+41, 19);
		tmp[19] = 0;
		replaceExpDesignator(tmp, 19);
		eph[ieph][sv].deltan = atof(tmp);

		strncpy(tmp, str+60, 19);
		tmp[19] = 0;
		replaceExpDesignator(tmp, 19);
		eph[ieph][sv].m0 = atof(tmp);

		// BROADCAST ORBIT - 2
		if (NULL==fgets(str, MAX_CHAR, fp))
			break;

		strncpy(tmp, str+3, 19);
		tmp[19] = 0;
		replaceExpDesignator(tmp, 19);
		eph[ieph][sv].cuc = atof(tmp);

		strncpy(tmp, str+22, 19);
		tmp[19] = 0;
		replaceExpDesignator(tmp, 19);
		eph[ieph][sv].ecc = atof(tmp);

		strncpy(tmp, str+41, 19);
		tmp[19] = 0;
		replaceExpDesignator(tmp, 19);
		eph[ieph][sv].cus = atof(tmp);

		strncpy(tmp, str+60, 19);
		tmp[19] = 0;
		replaceExpDesignator(tmp, 19);
		eph[ieph][sv].sqrta = atof(tmp);

		// BROADCAST ORBIT - 3
		if (NULL==fgets(str, MAX_CHAR, fp))
			break;

		strncpy(tmp, str+3, 19);
		tmp[19] = 0;
		replaceExpDesignator(tmp, 19);
		eph[ieph][sv].toe.sec = atof(tmp);

		strncpy(tmp, str+22, 19);
		tmp[19] = 0;
		replaceExpDesignator(tmp, 19);
		eph[ieph][sv].cic = atof(tmp);

		strncpy(tmp, str+41, 19);
		tmp[19] = 0;
		replaceExpDesignator(tmp, 19);
		eph[ieph][sv].omg0 = atof(tmp);

		strncpy(tmp, str+60, 19);
		tmp[19] = 0;
		replaceExpDesignator(tmp, 19);
		eph[ieph][sv].cis = atof(tmp);

		// BROADCAST ORBIT - 4
		if (NULL==fgets(str, MAX_CHAR, fp))
			break;

		strncpy(tmp, str+3, 19);
		tmp[19] = 0;
		replaceExpDesignator(tmp, 19);
		eph[ieph][sv].inc0 = atof(tmp);

		strncpy(tmp, str+22, 19);
		tmp[19] = 0;
		replaceExpDesignator(tmp, 19);
		eph[ieph][sv].crc = atof(tmp);

		strncpy(tmp, str+41, 19);
		tmp[19] = 0;
		replaceExpDesignator(tmp, 19);
		eph[ieph][sv].aop = atof(tmp);

		strncpy(tmp, str+60, 19);
		tmp[19] = 0;
		replaceExpDesignator(tmp, 19);
		eph[ieph][sv].omgdot = atof(tmp);

		// BROADCAST ORBIT - 5
		if (NULL==fgets(str, MAX_CHAR, fp))
			break;

		strncpy(tmp, str+3, 19);
		tmp[19] = 0;
		replaceExpDesignator(tmp, 19);
		eph[ieph][sv].idot = atof(tmp);

		strncpy(tmp, str+22, 19);
		tmp[19] = 0;
		replaceExpDesignator(tmp, 19);
		eph[ieph][sv].codeL2 = (int)atof(tmp);

		strncpy(tmp, str+41, 19);
		tmp[19] = 0;
		replaceExpDesignator(tmp, 19);
		eph[ieph][sv].toe.week = (int)atof(tmp);

		// BROADCAST ORBIT - 6
		if (NULL==fgets(str, MAX_CHAR, fp))
			break;

		strncpy(tmp, str+22, 19);
		tmp[19] = 0;
		replaceExpDesignator(tmp, 19);
		eph[ieph][sv].svhlth = (int)atof(tmp);
		if ((eph[ieph][sv].svhlth>0) && (eph[ieph][sv].svhlth<32))
			eph[ieph][sv].svhlth += 32; // Set MSB to 1

		strncpy(tmp, str+41, 19);
		tmp[19] = 0;
		replaceExpDesignator(tmp, 19);
		eph[ieph][sv].tgd = atof(tmp);

		strncpy(tmp, str+60, 19);
		tmp[19] = 0;
		replaceExpDesignator(tmp, 19);
		eph[ieph][sv].iodc = (int)atof(tmp);

		// BROADCAST ORBIT - 7
		if (NULL==fgets(str, MAX_CHAR, fp))
			break;

		// Set valid flag
		eph[ieph][sv].vflg = 1;

		// Update the working variables
		eph[ieph][sv].A = eph[ieph][sv].sqrta * eph[ieph][sv].sqrta;
		eph[ieph][sv].n = sqrt(GM_EARTH/(eph[ieph][sv].A*eph[ieph][sv].A*eph[ieph][sv].A)) + eph[ieph][sv].deltan;
		eph[ieph][sv].sq1e2 = sqrt(1.0 - eph[ieph][sv].ecc*eph[ieph][sv].ecc);
		eph[ieph][sv].omgkdot = eph[ieph][sv].omgdot - OMEGA_EARTH;
	}

	fclose(fp);

	if (g0.week>=0)
		ieph += 1; // Number of sets of ephemerides

	return(ieph);
}

/*! \brief Read a numeric field of a RINEX line
 *  \param[in] str Text line
 *  \param[in] col Start column (0-based)
 *  \param[in] width Field width (max 19)
 *  \returns Field value, 0 if the line is too short
 */
double rinexField(const char *str, int col, int width)
{
	char tmp[20];

	if ((int)strlen(str)<=col)
		return(0.0);

	strncpy(tmp, str+col, width);
	tmp[width] = 0;
	replaceExpDesignator(tmp, width);

	return(atof(tmp));
}

/*! \brief Get the RINEX version number from the file header
 *  \returns Version number, or -1 if the file cannot be opened
 */
double rinexVersion(const char *fname)
{
	FILE *fp;
	char str[MAX_CHAR];
	double ver = 0.0;

	if (NULL==(fp=fopen(fname, "rt")))
		return(-1.0);

	if (NULL!=fgets(str, MAX_CHAR, fp))
		ver = rinexField(str, 0, 9);

	fclose(fp);

	return(ver);
}

/*! \brief Fill an ephemeris structure from a RINEX 3 broadcast orbit record
 *  \param[out] eph Ephemeris
 *  \param[in] v Record values: v[1..3] clock, v[4*k..4*k+3] broadcast orbit k (k=1..7)
 *  \param[in] t Epoch of the record (time of clock)
 *  \param[in] sys Constellation of the record
 */
void rinex3ToEph(ephem_t *eph, const double *v, datetime_t t, int sys)
{
	eph->t = t;
	date2gps(&t, &eph->toc);

	eph->af0 = v[1];
	eph->af1 = v[2];
	eph->af2 = v[3];
	eph->iode = (int)v[4];
	eph->crs = v[5];
	eph->deltan = v[6];
	eph->m0 = v[7];
	eph->cuc = v[8];
	eph->ecc = v[9];
	eph->cus = v[10];
	eph->sqrta = v[11];
	eph->toe.sec = v[12];
	eph->cic = v[13];
	eph->omg0 = v[14];
	eph->cis = v[15];
	eph->inc0 = v[16];
	eph->crc = v[17];
	eph->aop = v[18];
	eph->omgdot = v[19];
	eph->idot = v[20];
	eph->codeL2 = (int)v[21];
	eph->toe.week = (int)v[22];
	eph->svhlth = (int)v[25];

	if (sys==SYS_GAL)
	{
		eph->iodc = eph->iode; // IODnav
		eph->sisa = v[24];
		eph->bgda = v[26];
		eph->tgd = v[27]; // BGD E1-E5b, applies to single-frequency E1
	}
	else
	{
		if ((eph->svhlth>0) && (eph->svhlth<32))
			eph->svhlth += 32; // Set MSB to 1
		eph->tgd = v[26];
		eph->iodc = (int)v[27];
	}

	eph->vflg = 1;

	// Update the working variables
	eph->A = eph->sqrta * eph->sqrta;
	eph->n = sqrt(((sys==SYS_GAL)?GM_EARTH_GAL:GM_EARTH)/(eph->A*eph->A*eph->A)) + eph->deltan;
	eph->sq1e2 = sqrt(1.0 - eph->ecc*eph->ecc);
	eph->omgkdot = eph->omgdot - OMEGA_EARTH;

	return;
}

/*! \brief GPS ephemeris record with its satellite index, used for sorting */
typedef struct
{
	int sv;
	ephem_t eph;
} ephrec_t;

int compareEphToc(const void *a, const void *b)
{
	double dt = subGpsTime(((const ephrec_t *)a)->eph.toc, ((const ephrec_t *)b)->eph.toc);

	return((dt>0.0)-(dt<0.0));
}

/*! \brief Read GPS and Galileo ephemerides from a RINEX 3 navigation file
 *  \param[out] eph Array of GPS ephemeris sets (same layout as readRinexNavAll)
 *  \param[out] ionoutc GPS ionospheric and UTC parameters
 *  \param[in] fname File name of the RINEX 3 file
 *  \returns Number of GPS sets of ephemerides, -1 on error
 *
 *  Galileo I/NAV records are stored per satellite in galeph[][] and the
 *  Galileo parameters in galutc.
 */
int readRinex3Nav(ephem_t eph[][MAX_SAT], ionoutc_t *ionoutc, const char *fname)
{
	FILE *fp;
	int ieph;
	int sv,k,j,i;
	char str[MAX_CHAR];
	char sys;
	double v[32];
	datetime_t t;
	gpstime_t g0;
	ephem_t tmp;
	ephrec_t *geph = NULL; // GPS records, sorted by time of clock before grouping
	int ngeph = 0, maxgeph = 0;
	int flags = 0x0;

	if (NULL==(fp=fopen(fname, "rt")))
		return(-1);

	// Clear valid flag
	for (ieph=0; ieph<EPHEM_ARRAY_SIZE; ieph++)
		for (sv=0; sv<MAX_SAT; sv++)
			eph[ieph][sv].vflg = 0;

	for (sv=0; sv<MAX_SAT_GAL; sv++)
		galneph[sv] = 0;

	galutc.vflg = FALSE;

	// Read header lines
	while (1)
	{
		if (NULL==fgets(str, MAX_CHAR, fp))
			break;

		if (strncmp(str+60, "END OF HEADER", 13)==0)
			break;
		else if (strncmp(str+60, "IONOSPHERIC CORR", 16)==0)
		{
			if (strncmp(str, "GPSA", 4)==0)
			{
				ionoutc->alpha0 = rinexField(str, 5, 12);
				ionoutc->alpha1 = rinexField(str, 17, 12);
				ionoutc->alpha2 = rinexField(str, 29, 12);
				ionoutc->alpha3 = rinexField(str, 41, 12);
				flags |= 0x1;
			}
			else if (strncmp(str, "GPSB", 4)==0)
			{
				ionoutc->beta0 = rinexField(str, 5, 12);
				ionoutc->beta1 = rinexField(str, 17, 12);
				ionoutc->beta2 = rinexField(str, 29, 12);
				ionoutc->beta3 = rinexField(str, 41, 12);
				flags |= 0x1<<1;
			}
			else if (strncmp(str, "GAL ", 4)==0)
			{
				galutc.ai0 = rinexField(str, 5, 12);
				galutc.ai1 = rinexField(str, 17, 12);
				galutc.ai2 = rinexField(str, 29, 12);
				galutc.vflg = TRUE;
			}
		}
		else if (strncmp(str+60, "TIME SYSTEM CORR", 16)==0)
		{
			if (strncmp(str, "GPUT", 4)==0)
			{
				ionoutc->A0 = rinexField(str, 5, 17);
				ionoutc->A1 = rinexField(str, 22, 16);
				ionoutc->tot = (int)rinexField(str, 38, 7);
				ionoutc->wnt = (int)rinexField(str, 45, 5);

				if (ionoutc->tot%4096==0)
					flags |= 0x1<<2;
			}
			else if (strncmp(str, "GAUT", 4)==0)
			{
				galutc.A0 = rinexField(str, 5, 17);
				galutc.A1 = rinexField(str, 22, 16);
				galutc.tot = (int)rinexField(str, 38, 7);
				galutc.wnt = (int)rinexField(str, 45, 5);
			}
			else if (strncmp(str, "GAGP", 4)==0)
			{
				galutc.A0G = rinexField(str, 5, 17);
				galutc.A1G = rinexField(str, 22, 16);
				galutc.t0g = (int)rinexField(str, 38, 7);
				galutc.wn0g = (int)rinexField(str, 45, 5);
			}
		}
		else if (strncmp(str+60, "LEAP SECONDS", 12)==0)
		{
			ionoutc->dtls = (int)rinexField(str, 0, 6);

			flags |= 0x1<<3;
		}
	}

	ionoutc->vflg = FALSE;
	if (flags==0xF) // Read all Iono/UTC lines
		ionoutc->vflg = TRUE;

	// Read ephemeris records
	if (NULL==fgets(str, MAX_CHAR, fp))
		str[0] = 0;

	while (str[0]!=0)
	{
		// Skip continuation lines of records we do not use
		if (str[0]==' ' || str[0]=='\n' || str[0]=='\r')
		{
			if (NULL==fgets(str, MAX_CHAR, fp))
				break;
			continue;
		}

		sys = str[0];

		if (sys!='G' && sys!='E')
		{
			if (NULL==fgets(str, MAX_CHAR, fp))
				break;
			continue;
		}

		sv = (int)rinexField(str, 1, 2)-1;

		// Epoch
		t.y = (int)rinexField(str, 4, 4);
		t.m = (int)rinexField(str, 9, 2);
		t.d = (int)rinexField(str, 12, 2);
		t.hh = (int)rinexField(str, 15, 2);
		t.mm = (int)rinexField(str, 18, 2);
		t.sec = rinexField(str, 21, 2);

		// SV clock
		v[0] = 0.0;
		for (j=1; j<4; j++)
			v[j] = rinexField(str, 4+19*j, 19);

		// Broadcast orbit 1 to 7
		for (k=1; k<8; k++)
		{
			if (NULL==fgets(str, MAX_CHAR, fp))
				break;

			for (j=0; j<4; j++)
				v[4*k+j] = rinexField(str, 4+19*j, 19);
		}

		if (k<8)
			break; // Truncated record

		if (sys=='G' && sv>=0 && sv<MAX_SAT)
		{
			if (ngeph>=maxgeph)
			{
				ephrec_t *p;

				maxgeph = (maxgeph==0) ? 1024 : maxgeph*2;
				p = (ephrec_t *)realloc(geph, maxgeph*sizeof(ephrec_t));
				if (p==NULL)
				{
					fprintf(stderr, "ERROR: Failed to allocate ephemeris buffer.\n");
					exit(1);
				}
				geph = p;
			}

			geph[ngeph].sv = sv;
			rinex3ToEph(&geph[ngeph].eph, v, t, SYS_GPS);
			ngeph++;
		}
		else if (sys=='E' && sv>=0 && sv<MAX_SAT_GAL)
		{
			// Keep I/NAV records only (data source bit 0: E1-B, bit 2: E5b)
			if (((int)v[21] & 0x5) && galneph[sv]<MAX_GAL_EPH)
			{
				rinex3ToEph(&tmp, v, t, SYS_GAL);

				// Skip duplicates of the same batch
				for (i=0; i<galneph[sv]; i++)
				{
					if (galeph[sv][i].iode==tmp.iode && subGpsTime(galeph[sv][i].toe, tmp.toe)==0.0)
						break;
				}

				if (i==galneph[sv])
					galeph[sv][galneph[sv]++] = tmp;
			}
		}

		if (NULL==fgets(str, MAX_CHAR, fp))
			break;
	}

	fclose(fp);

	// Group the GPS records into sets of ephemerides as in readRinexNavAll()
	if (ngeph>0)
		qsort(geph, ngeph, sizeof(ephrec_t), compareEphToc);

	g0.week = -1;
	ieph = 0;

	for (i=0; i<ngeph; i++)
	{
		if (g0.week==-1)
			g0 = geph[i].eph.toc;

		if (subGpsTime(geph[i].eph.toc, g0)>SECONDS_IN_HOUR)
		{
			g0 = geph[i].eph.toc;
			ieph++; // a new set of ephemerides

			if (ieph>=EPHEM_ARRAY_SIZE)
			{
				ieph--; // Keep the sets read so far
				break;
			}
		}

		eph[ieph][geph[i].sv] = geph[i].eph;
	}

	free(geph);

	if (g0.week>=0)
		ieph += 1; // Number of sets of ephemerides

	return(ieph);
}

/*! \brief Select the Galileo ephemerides in use at a given time
 *  \param[in] g GST (GPS-aligned week)
 *
 *  For each satellite the most recent batch with t0e <= g is chosen, within
 *  the 4-hour validity of Galileo ephemerides. If the scenario starts before
 *  the first batch, a batch less than 4 hours in the future is accepted.
 */
void selectGalEph(gpstime_t g)
{
	int sv,i,best;
	double dt,bestdt;

	for (sv=0; sv<MAX_SAT_GAL; sv++)
	{
		best = -1;
		bestdt = 0.0;

		for (i=0; i<galneph[sv]; i++)
		{
			dt = subGpsTime(g, galeph[sv][i].toe);

			if (dt<-4.0*SECONDS_IN_HOUR || dt>4.0*SECONDS_IN_HOUR)
				continue;

			// Prefer past batches (dt >= 0) with the smallest age
			if (best<0 || (dt>=0.0 && (bestdt<0.0 || dt<bestdt)) || (dt<0.0 && bestdt<0.0 && dt>bestdt))
			{
				best = i;
				bestdt = dt;
			}
		}

		if (best>=0)
			galcur[sv] = galeph[sv][best];
		else
			galcur[sv].vflg = 0;
	}

	return;
}

double ionosphericDelay(const ionoutc_t *ionoutc, gpstime_t g, double *llh, double *azel)
{
	double iono_delay = 0.0;
	double E,phi_u,lam_u,F;

	if (ionoutc->enable==FALSE)
		return (0.0); // No ionospheric delay

	E = azel[1]/PI;
	phi_u = llh[0]/PI;
	lam_u = llh[1]/PI;

	// Obliquity factor
	F = 1.0 + 16.0*pow((0.53 - E),3.0);

	if (ionoutc->vflg==FALSE)
		iono_delay = F*5.0e-9*SPEED_OF_LIGHT;
	else
	{
		double t,psi,phi_i,lam_i,phi_m,phi_m2,phi_m3;
		double AMP,PER,X,X2,X4;

		// Earth's central angle between the user position and the earth projection of
		// ionospheric intersection point (semi-circles)
		psi = 0.0137/(E + 0.11) - 0.022;

		// Geodetic latitude of the earth projection of the ionospheric intersection point
		// (semi-circles)
		phi_i = phi_u + psi*cos(azel[0]);
		if(phi_i>0.416)
			phi_i = 0.416;
		else if(phi_i<-0.416)
			phi_i = -0.416;

		// Geodetic longitude of the earth projection of the ionospheric intersection point
		// (semi-circles)
		lam_i = lam_u + psi*sin(azel[0])/cos(phi_i*PI);

		// Geomagnetic latitude of the earth projection of the ionospheric intersection
		// point (mean ionospheric height assumed 350 km) (semi-circles)
		phi_m = phi_i + 0.064*cos((lam_i - 1.617)*PI);
		phi_m2 = phi_m*phi_m;
		phi_m3 = phi_m2*phi_m;

		AMP = ionoutc->alpha0 + ionoutc->alpha1*phi_m
			+ ionoutc->alpha2*phi_m2 + ionoutc->alpha3*phi_m3;
		if (AMP<0.0)
			AMP = 0.0;

		PER = ionoutc->beta0 + ionoutc->beta1*phi_m
			+ ionoutc->beta2*phi_m2 + ionoutc->beta3*phi_m3;
		if (PER<72000.0)
			PER = 72000.0;

		// Local time (sec)
		t = SECONDS_IN_DAY/2.0*lam_i + g.sec;
		while(t>=SECONDS_IN_DAY)
			t -= SECONDS_IN_DAY;
		while(t<0)
			t += SECONDS_IN_DAY;

		// Phase (radians)
		X = 2.0*PI*(t - 50400.0)/PER;

		if(fabs(X)<1.57)
		{
			X2 = X*X;
			X4 = X2*X2;
			iono_delay = F*(5.0e-9 + AMP*(1.0 - X2/2.0 + X4/24.0))*SPEED_OF_LIGHT;
		}
		else
			iono_delay = F*5.0e-9*SPEED_OF_LIGHT;
	}

	return (iono_delay);
}

/*! \brief Compute the tropospheric delay with the Saastamoinen model
 *  \param[in] llh Receiver latitude, longitude [rad] and ellipsoidal height [m]
 *  \param[in] azel Satellite azimuth and elevation [rad]
 *  \returns Tropospheric delay [m]
 *
 *  Standard atmosphere (15 deg C and 1013.25 hPa at sea level, relative
 *  humidity 70%) and 1/cos(z) mapping, as the default model of RTKLIB.
 */
double tropoDelay(const double *llh, const double *azel)
{
	const double humi = 0.7;
	double hgt,pres,temp,e,z;

	if (llh[2]<-100.0 || llh[2]>1.0e4 || azel[1]<=0.0)
		return(0.0);

	hgt = (llh[2]<0.0) ? 0.0 : llh[2];

	pres = 1013.25*pow(1.0-2.2557e-5*hgt, 5.2568);
	temp = 15.0 - 6.5e-3*hgt + 273.16;
	e = 6.108*humi*exp((17.15*temp-4684.0)/(temp-38.45));

	z = PI/2.0 - azel[1];

	return(0.0022768*pres/(1.0-0.00266*cos(2.0*llh[0])-0.00028*hgt/1.0e3)/cos(z)  // Hydrostatic
		+ 0.002277*(1255.0/temp+0.05)*e/cos(z)); // Wet
}

/*! \brief Compute range between a satellite and the receiver
 *  \param[out] rho The computed range
 *  \param[in] eph Ephemeris data of the satellite
 *  \param[in] g GPS time at time of receiving the signal
 *  \param[in] xyz position of the receiver
 */
void computeRange(range_t *rho, ephem_t eph, ionoutc_t *ionoutc, gpstime_t g, double xyz[])
{
	double pos[3],vel[3],clk[2];
	double los[3];
	double tau;
	double range,rate;
	double xrot,yrot;

	double llh[3],neu[3];
	double tmat[3][3];

	// SV position at time of the pseudorange observation.
	satpos(eph, g, pos, vel, clk);

	// Receiver to satellite vector and light-time.
	subVect(los, pos, xyz);
	tau = normVect(los)/SPEED_OF_LIGHT;

	// Extrapolate the satellite position backwards to the transmission time.
	pos[0] -= vel[0]*tau;
	pos[1] -= vel[1]*tau;
	pos[2] -= vel[2]*tau;

	// Earth rotation correction. The change in velocity can be neglected.
	xrot = pos[0] + pos[1]*OMEGA_EARTH*tau;
	yrot = pos[1] - pos[0]*OMEGA_EARTH*tau;
	pos[0] = xrot;
	pos[1] = yrot;

	// New observer to satellite vector and satellite range.
	subVect(los, pos, xyz);
	range = normVect(los);
	rho->d = range;

	// Pseudorange.
	rho->range = range - SPEED_OF_LIGHT*clk[0];

	// Relative velocity of SV and receiver.
	rate = dotProd(vel, los)/range;

	// Pseudorange rate.
	rho->rate = rate; // - SPEED_OF_LIGHT*clk[1];

	// Time of application.
	rho->g = g;

	// Azimuth and elevation angles.
	xyz2llh(xyz, llh);
	ltcmat(llh, tmat);
	ecef2neu(los, tmat, neu);
	neu2azel(rho->azel, neu);

	// Add ionospheric delay
	rho->iono_delay = ionosphericDelay(ionoutc, g, llh, rho->azel);
	rho->range += rho->iono_delay;

	// Add tropospheric delay
	rho->tropo_delay = (ionoutc->tropo==TRUE) ? tropoDelay(llh, rho->azel) : 0.0;
	rho->range += rho->tropo_delay;

	return;
}

/*! \brief Compute the code phase for a given channel (satellite)
 *  \param chan Channel on which we operate (is updated)
 *  \param[in] rho1 Current range, after \a dt has expired
 *  \param[in dt delta-t (time difference) in seconds
 */
void computeCodePhase(channel_t *chan, range_t rho1, double dt)
{
	double ms;
	int ims;
	double rhorate;

	// Pseudorange rate.
	rhorate = (rho1.range - chan->rho0.range)/dt;

	// Carrier and code frequency.
	chan->f_carr = -rhorate/LAMBDA_L1;
	chan->f_code = CODE_FREQ + chan->f_carr*CARR_TO_CODE;

	if (chan->sys==SYS_GAL)
	{
		// Time since the start of the symbol buffer (1 s before g0), 1 symbol = 1 code = 4 ms
		ms = ((subGpsTime(chan->rho0.g,chan->g0)+1.0) - chan->rho0.range/SPEED_OF_LIGHT)*1000.0;

		chan->isym = (int)(ms/4.0);
		chan->code_phase = (ms/4.0-(double)chan->isym)*GAL_CODE_LEN; // in chip

		if (chan->isym<0 || chan->isym>=GAL_SYM_BUF)
		{
			fprintf(stderr, "\nWARNING: Galileo symbol index out of range.\n");
			chan->isym = (chan->isym<0) ? 0 : GAL_SYM_BUF-1;
		}

		chan->dataBit = 1-2*chan->sym[chan->isym];
		chan->secCode = 1-2*galCs25[chan->isym%GAL_SEC_CODE_LEN];

		// Save current pseudorange
		chan->rho0 = rho1;

		return;
	}

	// Initial code phase and data bit counters.
	ms = ((subGpsTime(chan->rho0.g,chan->g0)+6.0) - chan->rho0.range/SPEED_OF_LIGHT)*1000.0;

	ims = (int)ms;
	chan->code_phase = (ms-(double)ims)*CA_SEQ_LEN; // in chip

	chan->iword = ims/600; // 1 word = 30 bits = 600 ms
	ims -= chan->iword*600;

	chan->ibit = ims/20; // 1 bit = 20 code = 20 ms
	ims -= chan->ibit*20;

	chan->icode = ims; // 1 code = 1 ms

	chan->codeCA = chan->ca[(int)chan->code_phase]*2-1;
	chan->dataBit = (int)((chan->dwrd[chan->iword]>>(29-chan->ibit)) & 0x1UL)*2-1;

	// Save current pseudorange
	chan->rho0 = rho1;

	return;
}

/*! \brief Read the list of user motions from the input file
 *  \param[out] xyz Output array of ECEF vectors for user motion
 *  \param[[in] filename File name of the text input file
 *  \returns Number of user data motion records read, -1 on error
 */
int readUserMotion(double xyz[USER_MOTION_SIZE][3], const char *filename)
{
	FILE *fp;
	int numd;
	char str[MAX_CHAR];
	double t,x,y,z;

	if (NULL==(fp=fopen(filename,"rt")))
		return(-1);

	for (numd=0; numd<USER_MOTION_SIZE; numd++)
	{
		if (fgets(str, MAX_CHAR, fp)==NULL)
			break;

		if (EOF==sscanf(str, "%lf,%lf,%lf,%lf", &t, &x, &y, &z)) // Read CSV line
			break;

		xyz[numd][0] = x;
		xyz[numd][1] = y;
		xyz[numd][2] = z;
	}

	fclose(fp);

	return (numd);
}

/*! \brief Read the list of user motions from the input file
 *  \param[out] xyz Output array of LatLonHei coordinates for user motion
 *  \param[[in] filename File name of the text input file with format Lat,Lon,Hei
 *  \returns Number of user data motion records read, -1 on error
 *
 * Added by romalvarezllorens@gmail.com
 */
int readUserMotionLLH(double xyz[USER_MOTION_SIZE][3], const char *filename)
{
	FILE *fp;
	int numd;
	double t,llh[3];
	char str[MAX_CHAR];

	if (NULL==(fp=fopen(filename,"rt")))
		return(-1);

	for (numd=0; numd<USER_MOTION_SIZE; numd++)
	{
		if (fgets(str, MAX_CHAR, fp)==NULL)
			break;

		if (EOF==sscanf(str, "%lf,%lf,%lf,%lf", &t, &llh[0], &llh[1], &llh[2])) // Read CSV line
			break;

		if (llh[0] > 90.0 || llh[0] < -90.0 || llh[1]>180.0 || llh[1] < -180.0)
		{
			fprintf(stderr, "ERROR: Invalid file format (time[s], latitude[deg], longitude[deg], height [m].\n");
			numd = 0; // Empty user motion
			break;
		}

		llh[0] /= R2D; // convert to RAD
		llh[1] /= R2D; // convert to RAD

		llh2xyz(llh, xyz[numd]);
	}

	fclose(fp);

	return (numd);
}

int readNmeaGGA(double xyz[USER_MOTION_SIZE][3], const char *filename)
{
	FILE *fp;
	int numd = 0;
	char str[MAX_CHAR];
	char *token;
	double llh[3],pos[3];
	char tmp[8];

	if (NULL==(fp=fopen(filename,"rt")))
		return(-1);

	while (1)
	{
		if (fgets(str, MAX_CHAR, fp)==NULL)
			break;

		token = strtok(str, ",");

		if (strncmp(token+3, "GGA", 3)==0)
		{
			token = strtok(NULL, ","); // Date and time

			token = strtok(NULL, ","); // Latitude
			strncpy(tmp, token, 2);
			tmp[2] = 0;

			llh[0] = atof(tmp) + atof(token+2)/60.0;

			token = strtok(NULL, ","); // North or south
			if (token[0]=='S')
				llh[0] *= -1.0;

			llh[0] /= R2D; // in radian

			token = strtok(NULL, ","); // Longitude
			strncpy(tmp, token, 3);
			tmp[3] = 0;

			llh[1] = atof(tmp) + atof(token+3)/60.0;

			token = strtok(NULL, ","); // East or west
			if (token[0]=='W')
				llh[1] *= -1.0;

			llh[1] /= R2D; // in radian

			token = strtok(NULL, ","); // GPS fix
			token = strtok(NULL, ","); // Number of satellites
			token = strtok(NULL, ","); // HDOP

			token = strtok(NULL, ","); // Altitude above meas sea level

			llh[2] = atof(token);

			token = strtok(NULL, ","); // in meter

			token = strtok(NULL, ","); // Geoid height above WGS84 ellipsoid

			llh[2] += atof(token);

			// Convert geodetic position into ECEF coordinates
			llh2xyz(llh, pos);

			xyz[numd][0] = pos[0];
			xyz[numd][1] = pos[1];
			xyz[numd][2] = pos[2];

			// Update the number of track points
			numd++;

			if (numd>=USER_MOTION_SIZE)
				break;
		}
	}

	fclose(fp);

	return (numd);
}

int generateNavMsg(gpstime_t g, channel_t *chan, int init)
{
	int iwrd,isbf;
	gpstime_t g0;
	unsigned long wn,tow;
	unsigned sbfwrd;
	unsigned long prevwrd;
	int nib;

	g0.week = g.week;
	g0.sec = (double)(((unsigned long)(g.sec+0.5))/30UL) * 30.0; // Align with the full frame length = 30 sec
	chan->g0 = g0; // Data bit reference time

	wn = (unsigned long)(g0.week%1024);
	tow = ((unsigned long)g0.sec)/6UL;

	if (init==1) // Initialize subframe 5
	{
		prevwrd = 0UL;

		for (iwrd=0; iwrd<N_DWRD_SBF; iwrd++)
		{
			sbfwrd = chan->sbf[4][iwrd];

			// Add TOW-count message into HOW
			if (iwrd==1)
				sbfwrd |= ((tow&0x1FFFFUL)<<13);

			// Compute checksum
			sbfwrd |= (prevwrd<<30) & 0xC0000000UL; // 2 LSBs of the previous transmitted word
			nib = ((iwrd==1)||(iwrd==9))?1:0; // Non-information bearing bits for word 2 and 10
			chan->dwrd[iwrd] = computeChecksum(sbfwrd, nib);

			prevwrd = chan->dwrd[iwrd];
		}
	}
	else // Save subframe 5
	{
		for (iwrd=0; iwrd<N_DWRD_SBF; iwrd++)
		{
			chan->dwrd[iwrd] = chan->dwrd[N_DWRD_SBF*N_SBF+iwrd];

			prevwrd = chan->dwrd[iwrd];
		}
		/*
		// Sanity check
		if (((chan->dwrd[1])&(0x1FFFFUL<<13)) != ((tow&0x1FFFFUL)<<13))
		{
			fprintf(stderr, "\nWARNING: Invalid TOW in subframe 5.\n");
			return(0);
		}
		*/
	}

	for (isbf=0; isbf<N_SBF; isbf++)
	{
		tow++;

		for (iwrd=0; iwrd<N_DWRD_SBF; iwrd++)
		{
			sbfwrd = chan->sbf[isbf][iwrd];

			// Add transmission week number to Subframe 1
			if ((isbf==0)&&(iwrd==2))
				sbfwrd |= (wn&0x3FFUL)<<20;

			// Add TOW-count message into HOW
			if (iwrd==1)
				sbfwrd |= ((tow&0x1FFFFUL)<<13);

			// Compute checksum
			sbfwrd |= (prevwrd<<30) & 0xC0000000UL; // 2 LSBs of the previous transmitted word
			nib = ((iwrd==1)||(iwrd==9))?1:0; // Non-information bearing bits for word 2 and 10
			chan->dwrd[(isbf+1)*N_DWRD_SBF+iwrd] = computeChecksum(sbfwrd, nib);

			prevwrd = chan->dwrd[(isbf+1)*N_DWRD_SBF+iwrd];
		}
	}

	return(1);
}

/*! \brief Generate the Galileo E1-B and E1-C primary codes
 *  \param[out] e1b E1-B code chips (+1/-1)
 *  \param[out] e1c E1-C code chips (+1/-1)
 *  \param[in] prn PRN number (1..50)
 */
void galcodegen(int *e1b, int *e1c, int prn)
{
	const char *hex[2];
	int *code[2];
	int i,j,k,nib;
	char c;

	if (prn<1 || prn>50)
		return;

	hex[0] = galE1BCodeHex[prn-1];
	hex[1] = galE1CCodeHex[prn-1];
	code[0] = e1b;
	code[1] = e1c;

	for (k=0; k<2; k++)
	{
		for (i=0; i<GAL_CODE_LEN/4; i++)
		{
			c = hex[k][i];
			nib = (c<='9') ? (c-'0') : (c-'A'+10);

			for (j=0; j<4; j++)
				code[k][i*4+j] = ((nib>>(3-j))&0x1) ? -1 : 1; // Logic 1 -> -1, logic 0 -> +1
		}
	}

	return;
}

/*! \brief Write a value MSB first into an array of bits
 *  \returns Position following the written field
 */
int setBits(unsigned char *bits, int pos, unsigned long long val, int nbits)
{
	int i;

	for (i=0; i<nbits; i++)
		bits[pos+i] = (unsigned char)((val>>(nbits-1-i))&0x1ULL);

	return(pos+nbits);
}

/*! \brief Round x/2^-n to an integer (two's complement when written with setBits) */
unsigned long long galScale(double x, int n)
{
	return((unsigned long long)(long long)floor(ldexp(x, n)+0.5));
}

/*! \brief CRC-24Q of an array of bits (Galileo OS SIS ICD 5.1.9.4) */
unsigned long crc24q(const unsigned char *bits, int nbits)
{
	unsigned long crc = 0UL;
	int i;

	for (i=0; i<nbits; i++)
	{
		crc ^= ((unsigned long)bits[i])<<23;
		crc <<= 1;
		if (crc&0x1000000UL)
			crc ^= 0x1864CFBUL;
	}

	return(crc&0xFFFFFFUL);
}

/*! \brief Convert SISA in meters to the SISA index (Galileo OS SIS ICD Table 89) */
int galSisaIndex(double sisa)
{
	int idx;

	if (sisa<0.0)
		idx = 255; // No accuracy prediction available
	else if (sisa<0.5)
		idx = (int)floor(sisa/0.01+0.5);
	else if (sisa<1.0)
		idx = 50 + (int)floor((sisa-0.5)/0.02+0.5);
	else if (sisa<2.0)
		idx = 75 + (int)floor((sisa-1.0)/0.04+0.5);
	else if (sisa<=6.0)
		idx = 100 + (int)floor((sisa-2.0)/0.16+0.5);
	else
		idx = 255;

	return(idx);
}

/*! \brief Wrap an angle to [-pi, pi) */
double wrapPi(double a)
{
	a = fmod(a+PI, 2.0*PI);
	if (a<0.0)
		a += 2.0*PI;

	return(a-PI);
}

/*! \brief Build the Galileo almanac from the ephemerides in use
 *  \param[in] g Time used to set the almanac reference time t0a
 */
void galAlmanac(gpstime_t g)
{
	const double sqrtanom = sqrt(29600000.0); // Nominal semi-major axis
	const double inom = 56.0/R2D; // Nominal inclination
	ephem_t *e;
	galalm_t *a;
	double dt;
	int sv;

	galt0a = ((int)g.sec/600)*600;
	galwna = g.week;

	for (sv=0; sv<MAX_SAT_GAL; sv++)
	{
		e = &galcur[sv];
		a = &galalm[sv];
		a->svid = 0;

		if (e->vflg!=1)
			continue;

		a->dsqrta = e->sqrta - sqrtanom;
		a->ecc = e->ecc;

		// Skip satellites the almanac cannot represent (e.g. E14, E18 in eccentric orbits)
		if (fabs(a->dsqrta)>=8.0 || a->ecc>=0.03)
			continue;

		dt = (double)galt0a - e->toe.sec + (galwna-e->toe.week)*SECONDS_IN_WEEK;

		a->svid = sv+1;
		a->aop = e->aop;
		a->di = e->inc0 + e->idot*dt - inom;
		a->omg0 = wrapPi(e->omg0 + e->omgdot*dt);
		a->omgdot = e->omgdot;
		a->m0 = wrapPi(e->m0 + e->n*dt);

		dt = (double)galt0a - e->toc.sec + (galwna-e->toc.week)*SECONDS_IN_WEEK;
		a->af0 = e->af0 + e->af1*dt;
		a->af1 = e->af1;
		a->hs = (e->svhlth>>1)&0x3; // E1-B health status
	}

	return;
}

/*! \brief Write the orbital part of a Galileo almanac entry (SVID to Omega dot) */
int setGalAlmOrbit(unsigned char *w, int pos, const galalm_t *a)
{
	pos = setBits(w, pos, a->svid, 6);
	pos = setBits(w, pos, galScale(a->dsqrta, 9), 13);
	pos = setBits(w, pos, galScale(a->ecc, 16), 11);
	pos = setBits(w, pos, galScale(a->aop/PI, 15), 16);
	pos = setBits(w, pos, galScale(a->di/PI, 14), 11);
	pos = setBits(w, pos, galScale(a->omg0/PI, 15), 16);
	pos = setBits(w, pos, galScale(a->omgdot/PI, 33), 11);

	return(pos);
}

/*! \brief Write the clock and health part of a Galileo almanac entry */
int setGalAlmClock(unsigned char *w, int pos, const galalm_t *a)
{
	pos = setBits(w, pos, galScale(a->af0, 19), 16);
	pos = setBits(w, pos, galScale(a->af1, 38), 13);
	pos = setBits(w, pos, a->hs, 2); // E5bHS
	pos = setBits(w, pos, a->hs, 2); // E1BHS

	return(pos);
}

/*! \brief Generate a Galileo I/NAV word
 *  \param[out] w 128 bits of the word
 *  \param[in] type Word type (0 to 10)
 *  \param[in] eph Ephemeris of the transmitting satellite
 *  \param[in] svid SVID of the transmitting satellite
 *  \param[in] ionoutc GPS iono/UTC parameters (leap seconds)
 *  \param[in] gp GST at the start of the page carrying the word
 *  \param[in] alm Index of the first of the three almanacs of word types 7 to 10
 */
void galWord(unsigned char *w, int type, const ephem_t *eph, int svid, const ionoutc_t *ionoutc, gpstime_t gp, int alm)
{
	unsigned long wn,tow;
	unsigned long wnlsf,dn,dtlsf;
	const galalm_t *a1,*a2,*a3;
	int iodnav,iod_a,wna,hs,dvs;
	int pos = 0;

	memset(w, 0, 128);

	wn = (unsigned long)(gp.week-GAL_WEEK_OFFSET) & 0xFFFUL;
	tow = (unsigned long)(gp.sec+0.5);
	iodnav = eph->iode & 0x3FF;
	iod_a = (galt0a/600) & 0xF;
	wna = (galwna-GAL_WEEK_OFFSET) & 0x3;
	hs = (eph->svhlth>>1)&0x3; // RINEX: bit 0 E1-B DVS, bits 1-2 E1-B HS
	dvs = eph->svhlth&0x1;

	a1 = &galalm[alm%MAX_SAT_GAL];
	a2 = &galalm[(alm+1)%MAX_SAT_GAL];
	a3 = &galalm[(alm+2)%MAX_SAT_GAL];

	pos = setBits(w, pos, type, 6);

	switch (type)
	{
	case 1: // Ephemeris (1/4)
		pos = setBits(w, pos, iodnav, 10);
		pos = setBits(w, pos, (unsigned long)(eph->toe.sec/60.0+0.5), 14);
		pos = setBits(w, pos, galScale(eph->m0/PI, 31), 32);
		pos = setBits(w, pos, galScale(eph->ecc, 33), 32);
		pos = setBits(w, pos, galScale(eph->sqrta, 19), 32);
		break;
	case 2: // Ephemeris (2/4)
		pos = setBits(w, pos, iodnav, 10);
		pos = setBits(w, pos, galScale(eph->omg0/PI, 31), 32);
		pos = setBits(w, pos, galScale(eph->inc0/PI, 31), 32);
		pos = setBits(w, pos, galScale(eph->aop/PI, 31), 32);
		pos = setBits(w, pos, galScale(eph->idot/PI, 43), 14);
		break;
	case 3: // Ephemeris (3/4) and SISA
		pos = setBits(w, pos, iodnav, 10);
		pos = setBits(w, pos, galScale(eph->omgdot/PI, 43), 24);
		pos = setBits(w, pos, galScale(eph->deltan/PI, 43), 16);
		pos = setBits(w, pos, galScale(eph->cuc, 29), 16);
		pos = setBits(w, pos, galScale(eph->cus, 29), 16);
		pos = setBits(w, pos, galScale(eph->crc, 5), 16);
		pos = setBits(w, pos, galScale(eph->crs, 5), 16);
		pos = setBits(w, pos, galSisaIndex(eph->sisa), 8);
		break;
	case 4: // SVID, ephemeris (4/4) and clock correction
		pos = setBits(w, pos, iodnav, 10);
		pos = setBits(w, pos, svid, 6);
		pos = setBits(w, pos, galScale(eph->cic, 29), 16);
		pos = setBits(w, pos, galScale(eph->cis, 29), 16);
		pos = setBits(w, pos, (unsigned long)(eph->toc.sec/60.0+0.5), 14);
		pos = setBits(w, pos, galScale(eph->af0, 34), 31);
		pos = setBits(w, pos, galScale(eph->af1, 46), 21);
		pos = setBits(w, pos, galScale(eph->af2, 59), 6);
		break;
	case 5: // Ionospheric correction, BGD, health, GST
		pos = setBits(w, pos, galScale(galutc.ai0, 2), 11);
		pos = setBits(w, pos, galScale(galutc.ai1, 8), 11);
		pos = setBits(w, pos, galScale(galutc.ai2, 15), 14);
		pos = setBits(w, pos, 0, 5); // Ionospheric disturbance flags
		pos = setBits(w, pos, galScale(eph->bgda, 32), 10);
		pos = setBits(w, pos, galScale(eph->tgd, 32), 10);
		pos = setBits(w, pos, (eph->svhlth>>7)&0x3, 2); // E5bHS
		pos = setBits(w, pos, hs, 2); // E1BHS
		pos = setBits(w, pos, (eph->svhlth>>6)&0x1, 1); // E5bDVS
		pos = setBits(w, pos, dvs, 1); // E1BDVS
		pos = setBits(w, pos, wn, 12);
		pos = setBits(w, pos, tow, 20);
		break;
	case 6: // GST-UTC conversion
		if (ionoutc->leapen==TRUE)
		{
			wnlsf = (unsigned long)(ionoutc->wnlsf-GAL_WEEK_OFFSET);
			dn = (unsigned long)(ionoutc->dn);
			dtlsf = (unsigned long)(ionoutc->dtlsf);
		}
		else
		{
			wnlsf = 1929-GAL_WEEK_OFFSET; // Same default as the GPS LNAV message
			dn = 7;
			dtlsf = 18;
		}
		pos = setBits(w, pos, galScale(galutc.A0, 30), 32);
		pos = setBits(w, pos, galScale(galutc.A1, 50), 24);
		pos = setBits(w, pos, (unsigned long)ionoutc->dtls, 8);
		pos = setBits(w, pos, (unsigned long)(galutc.tot/3600), 8);
		pos = setBits(w, pos, (unsigned long)(galutc.wnt-GAL_WEEK_OFFSET), 8);
		pos = setBits(w, pos, wnlsf, 8);
		pos = setBits(w, pos, dn, 3);
		pos = setBits(w, pos, dtlsf, 8);
		pos = setBits(w, pos, tow, 20);
		break;
	case 7: // Almanac for SVID1 (1/2)
		pos = setBits(w, pos, iod_a, 4);
		pos = setBits(w, pos, wna, 2);
		pos = setBits(w, pos, (unsigned long)(galt0a/600), 10);
		pos = setGalAlmOrbit(w, pos, a1);
		pos = setBits(w, pos, galScale(a1->m0/PI, 15), 16);
		break;
	case 8: // Almanac for SVID1 (2/2) and SVID2 (1/2)
		pos = setBits(w, pos, iod_a, 4);
		pos = setGalAlmClock(w, pos, a1);
		pos = setGalAlmOrbit(w, pos, a2);
		break;
	case 9: // Almanac for SVID2 (2/2) and SVID3 (1/2)
		pos = setBits(w, pos, iod_a, 4);
		pos = setBits(w, pos, wna, 2);
		pos = setBits(w, pos, (unsigned long)(galt0a/600), 10);
		pos = setBits(w, pos, galScale(a2->m0/PI, 15), 16);
		pos = setGalAlmClock(w, pos, a2);
		pos = setBits(w, pos, a3->svid, 6);
		pos = setBits(w, pos, galScale(a3->dsqrta, 9), 13);
		pos = setBits(w, pos, galScale(a3->ecc, 16), 11);
		pos = setBits(w, pos, galScale(a3->aop/PI, 15), 16);
		pos = setBits(w, pos, galScale(a3->di/PI, 14), 11);
		break;
	case 10: // Almanac for SVID3 (2/2) and GST-GPS conversion
		pos = setBits(w, pos, iod_a, 4);
		pos = setBits(w, pos, galScale(a3->omg0/PI, 15), 16);
		pos = setBits(w, pos, galScale(a3->omgdot/PI, 33), 11);
		pos = setBits(w, pos, galScale(a3->m0/PI, 15), 16);
		pos = setGalAlmClock(w, pos, a3);
		pos = setBits(w, pos, galScale(galutc.A0G, 35), 16);
		pos = setBits(w, pos, galScale(galutc.A1G, 51), 12);
		pos = setBits(w, pos, (unsigned long)(galutc.t0g/3600), 8);
		pos = setBits(w, pos, (unsigned long)(galutc.wn0g-GAL_WEEK_OFFSET), 6);
		break;
	default: // Spare word with GST
		pos = setBits(w, pos, 2, 2); // Time = '10': WN and TOW are valid
		pos = setBits(w, pos, 0, 88);
		pos = setBits(w, pos, wn, 12);
		pos = setBits(w, pos, tow, 20);
		break;
	}

	return;
}

/*! \brief FEC encode and interleave a 120-bit I/NAV page part, and add the sync pattern
 *  \param[in] bits Page part (including the 6 tail bits)
 *  \param[out] sym 250 symbols
 */
void galEncodePagePart(const unsigned char *bits, unsigned char *sym)
{
	static const unsigned char sync[10] = {0,1,0,1,1,0,0,0,0,0};
	unsigned char enc[2*GAL_PART_BITS];
	unsigned int reg = 0;
	int i,r,c;

	// Convolutional code: K=7, G1=171o, G2=133o (inverted), G1 first
	for (i=0; i<GAL_PART_BITS; i++)
	{
		reg = ((reg>>1) | ((unsigned int)bits[i]<<6)) & 0x7F;
		enc[2*i] = (unsigned char)(countBits(reg&0x79)&0x1);
		enc[2*i+1] = (unsigned char)(1-(countBits(reg&0x5B)&0x1));
	}

	memcpy(sym, sync, 10);

	// Block interleaver: written in 30 columns of 8 symbols, read in 8 rows
	for (r=0; r<8; r++)
		for (c=0; c<30; c++)
			sym[10+r*30+c] = enc[c*8+r];

	return;
}

/*! \brief Generate the I/NAV symbols of a Galileo E1-B channel
 *  \param[in] g Current GST
 *  \param[in] ionoutc GPS iono/UTC parameters (leap seconds)
 *  \param chan Channel (sym and g0 are updated)
 *
 *  The buffer holds the E1-B page started 1 s before the 30 s sub-frame
 *  boundary g0, followed by the 15 pages of the sub-frame (Table 38 of the
 *  Galileo OS SIS ICD). Word types without content in this simulator
 *  (16 to 22) are replaced by spare words.
 */
int generateGalNavMsg(gpstime_t g, const ionoutc_t *ionoutc, channel_t *chan)
{
	// Word type per E1-B page; 7 stands for 7 or 9, 8 for 8 or 10
	static const int pageWord[GAL_N_PAGE] = {0, 2, 4, 6, 7, 8, 0, 0, 0, 0, 0, 1, 3, 5, 0, 0};
	static const int ssp[3] = {0x2F, 0x04, 0x2B}; // SSP3, SSP1, SSP2 at GST mod 6 = 0, 2, 4
	unsigned char w[128],even[GAL_PART_BITS],odd[GAL_PART_BITS];
	unsigned char crcbits[196];
	const ephem_t *eph;
	gpstime_t g0,gp;
	unsigned long crc;
	int k,type,sfid,alm,tow;

	g0.week = g.week;
	g0.sec = (double)(((unsigned long)(g.sec+0.5))/30UL) * 30.0; // Align with the sub-frame length = 30 sec
	chan->g0 = g0;

	eph = &galcur[chan->prn-1];

	// Sub-frame ID within the 720 s frame, and almanacs it carries (Table 39)
	sfid = (((unsigned long)g0.sec)%720UL)/30 + 1;
	alm = (18 + 3*((sfid-1)/2)) % MAX_SAT_GAL;

	for (k=0; k<GAL_N_PAGE; k++)
	{
		gp = incGpsTime(g0, -1.0+2.0*k); // Page start
		tow = (int)(gp.sec+0.5);

		type = pageWord[k];
		if ((type==7 || type==8) && (sfid%2==0))
			type += 2;

		galWord(w, type, eph, chan->prn, ionoutc, gp, alm);

		// Even part: even/odd, page type, data (1/2), tail
		memset(even, 0, GAL_PART_BITS);
		even[0] = 0;
		even[1] = 0;
		memcpy(even+2, w, 112);

		// Odd part: even/odd, page type, data (2/2), OSNMA, SAR, spare, CRC, SSP, tail
		memset(odd, 0, GAL_PART_BITS);
		odd[0] = 1;
		odd[1] = 0;
		memcpy(odd+2, w+112, 16);
		odd[58] = 1; // Spare SAR data: start bit = 1

		memcpy(crcbits, even, 114);
		memcpy(crcbits+114, odd, 82);
		crc = crc24q(crcbits, 196);
		setBits(odd, 82, crc, 24);
		setBits(odd, 106, ssp[((tow+1)%6)/2], 8);

		galEncodePagePart(even, chan->sym+(2*k)*GAL_PART_SYMS);
		galEncodePagePart(odd, chan->sym+(2*k+1)*GAL_PART_SYMS);
	}

	return(1);
}

int checkSatVisibility(ephem_t eph, gpstime_t g, double *xyz, double elvMask, double *azel)
{
	double llh[3],neu[3];
	double pos[3],vel[3],clk[3],los[3];
	double tmat[3][3];

	if (eph.vflg != 1)
		return (-1); // Invalid

	xyz2llh(xyz,llh);
	ltcmat(llh, tmat);

	satpos(eph, g, pos, vel, clk);
	subVect(los, pos, xyz);
	ecef2neu(los, tmat, neu);
	neu2azel(azel, neu);

	if (azel[1]*R2D > elvMask)
		return (1); // Visible
	// else
	return (0); // Invisible
}

/*! \brief Allocate channels to visible satellites and release the others
 *  \param chan Channels
 *  \param[in] eph Current set of GPS ephemerides (Galileo: galcur)
 *  \param[in] sysmask Enabled constellations (bit SYS_GPS, bit SYS_GAL)
 *  \returns Number of visible satellites
 */
int allocateChannel(channel_t *chan, ephem_t *eph, ionoutc_t ionoutc, gpstime_t grx, double *xyz, double elvMask, int sysmask)
{
	int nsat=0;
	int i,sv,idx,sys,visible;
	double azel[2];
	ephem_t *seph;

	range_t rho;
	double ref[3]={0.0};
	double r_ref,r_xyz;
	double phase_ini;

	for (idx=0; idx<MAX_SAT+MAX_SAT_GAL; idx++)
	{
		if (idx<MAX_SAT)
		{
			sys = SYS_GPS;
			sv = idx;
			seph = &eph[sv];
		}
		else
		{
			sys = SYS_GAL;
			sv = idx-MAX_SAT;
			seph = &galcur[sv];
		}

		visible = 0;
		if (sysmask & (1<<sys))
		{
			visible = (checkSatVisibility(*seph, grx, xyz, 0.0, azel)==1);

			// Skip Galileo satellites flagged with E1-B/C out of service, in test, etc.
			if (visible && sys==SYS_GAL && ((seph->svhlth>>1)&0x3)!=0)
				visible = 0;
		}

		if (visible)
		{
			nsat++; // Number of visible satellites

			if (allocatedSat[idx]==-1) // Visible but not allocated
			{
				// Allocated new satellite
				for (i=0; i<MAX_CHAN; i++)
				{
					if (chan[i].prn==0)
					{
						// Initialize channel
						chan[i].prn = sv+1;
						chan[i].sys = sys;
						chan[i].azel[0] = azel[0];
						chan[i].azel[1] = azel[1];

						if (sys==SYS_GAL)
						{
							chan[i].code_len = GAL_CODE_LEN;

							// E1-B and E1-C code generation
							galcodegen(chan[i].ca, chan[i].pilot, chan[i].prn);

							// Generate I/NAV symbols
							generateGalNavMsg(grx, &ionoutc, &chan[i]);
						}
						else
						{
							chan[i].code_len = CA_SEQ_LEN;

							// C/A code generation
							codegen(chan[i].ca, chan[i].prn);

							// Generate subframe
							eph2sbf(*seph, ionoutc, chan[i].sbf);

							// Generate navigation message
							generateNavMsg(grx, &chan[i], 1);
						}

						// Initialize pseudorange
						computeRange(&rho, *seph, &ionoutc, grx, xyz);
						chan[i].rho0 = rho;

						// Initialize carrier phase
						r_xyz = rho.range;

						computeRange(&rho, *seph, &ionoutc, grx, ref);
						r_ref = rho.range;

						phase_ini = 0.0; // TODO: Must initialize properly
						//phase_ini = (2.0*r_ref - r_xyz)/LAMBDA_L1;
#ifdef FLOAT_CARR_PHASE
						chan[i].carr_phase = phase_ini - floor(phase_ini);
#else
						phase_ini -= floor(phase_ini);
						chan[i].carr_phase = (unsigned int)(512.0 * 65536.0 * phase_ini);
#endif
						// Done.
						break;
					}
				}

				// Set satellite allocation channel
				if (i<MAX_CHAN)
					allocatedSat[idx] = i;
			}
		}
		else if (allocatedSat[idx]>=0) // Not visible but allocated
		{
			// Clear channel
			chan[allocatedSat[idx]].prn = 0;

			// Clear satellite allocation flag
			allocatedSat[idx] = -1;
		}
	}

	return(nsat);
}

/*! \brief Download the daily broadcast ephemeris file (RINEX 3, mixed) from BKG
 *  \param[in] g Scenario date
 *  \param[in] nday Number of days to try, going back from the scenario date
 *  \param[out] navfile Name of the downloaded file
 *  \returns 0 on success, -1 on failure
 *
 *  For each day the final merged IGS file is tried first, then the files
 *  merged in real time during the day. Final files already in the current
 *  directory are reused; real-time files are downloaded again because they
 *  grow during the day. Requires the curl and gzip commands.
 */
int downloadRinexNav(gpstime_t g, int nday, char *navfile)
{
	static const char *prefix[3] = {"BRDC00IGS_R", "BRDC00WRD_S", "BRDC00WRD_R"};
	static const int cumday[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
	char fname[MAX_PATH_LEN],gzname[MAX_PATH_LEN],url[MAX_PATH_LEN],cmd[3*MAX_PATH_LEN];
	datetime_t t;
	gpstime_t gd;
	FILE *fp;
	int iday,k,doy,ok;

	for (iday=0; iday<nday; iday++)
	{
		gd = incGpsTime(g, -SECONDS_IN_DAY*iday);
		gps2date(&gd, &t);

		doy = cumday[t.m-1] + t.d;
		if (t.m>2 && ((t.y%4==0 && t.y%100!=0) || t.y%400==0))
			doy++;

		for (k=0; k<3; k++)
		{
			snprintf(fname, MAX_PATH_LEN, "%s_%04d%03d0000_01D_MN.rnx", prefix[k], t.y, doy);

			// Reuse a final file downloaded before
			if (k==0 && (fp=fopen(fname, "rt"))!=NULL)
			{
				fclose(fp);
				if (rinexVersion(fname)>=3.0)
				{
					fprintf(stderr, "Using cached ephemeris file %s\n", fname);
					strcpy(navfile, fname);
					return(0);
				}
			}

			snprintf(url, MAX_PATH_LEN, "https://igs.bkg.bund.de/root_ftp/IGS/BRDC/%04d/%03d/%s.gz", t.y, doy, fname);
			fprintf(stderr, "Downloading %s\n", url);

			snprintf(gzname, MAX_PATH_LEN, "%s.gz", fname);

			snprintf(cmd, sizeof(cmd), "curl -sfL --connect-timeout 20 -m 300 -o \"%s\" \"%s\"", gzname, url);
			if (system(cmd)!=0)
			{
				fprintf(stderr, "  not available\n");
				remove(gzname);
				continue;
			}

			snprintf(cmd, sizeof(cmd), "gzip -dc \"%s\" > \"%s\"", gzname, fname);
			ok = (system(cmd)==0);
			remove(gzname);

			if (!ok)
			{
				fprintf(stderr, "ERROR: Failed to decompress %s.gz (is gzip installed?)\n", fname);
				remove(fname);
				return(-1);
			}

			if (rinexVersion(fname)<3.0)
			{
				fprintf(stderr, "WARNING: %s is not a RINEX 3 file.\n", fname);
				remove(fname);
				continue;
			}

			strcpy(navfile, fname);
			return(0);
		}
	}

	return(-1);
}

void usage(void)
{
	fprintf(stderr, "Usage: gps-sdr-sim [options]\n"
		"Options:\n"
		"  -e <gps_nav>     RINEX navigation file: RINEX 2 GPS, or RINEX 3 GPS/Galileo/mixed\n"
		"  -D               Download the daily GPS/Galileo broadcast ephemerides of the scenario\n"
		"                   date from BKG instead of -e (needs curl and gzip)\n"
		"  -C <systems>     Constellations to simulate: G (GPS), E (Galileo), e.g. GE\n"
		"                   (default: every constellation found in the navigation file)\n"
		"  -u <user_motion> User motion file in ECEF x, y, z format (dynamic mode)\n"
		"  -x <user_motion> User motion file in lat, lon, height format (dynamic mode)\n"
		"  -g <nmea_gga>    NMEA GGA stream (dynamic mode)\n"
		"  -c <location>    ECEF X,Y,Z in meters (static mode) e.g. 3967283.154,1022538.181,4872414.484\n"
		"  -l <location>    Lat, lon, height (static mode) e.g. 35.681298,139.766247,10.0\n"
		"  -L <wnslf,dn,dtslf> User leap future event in GPS week number, day number, next leap second e.g. 2347,3,19\n"
		"  -t <date,time>   Scenario start time YYYY/MM/DD,hh:mm:ss\n"
		"  -T <date,time>   Overwrite TOC and TOE to scenario start time\n"
		"  -d <duration>    Duration [sec] (dynamic mode max: %.0f, static mode max: %d)\n"
		"  -o <output>      I/Q sampling data file (default: gpssim.bin)\n"
		"  -s <frequency>   Sampling frequency [Hz] (default: 2600000, 5000000 with Galileo)\n"
		"  -b <iq_bits>     I/Q data format [1/8/16] (default: 16)\n"
		"  -i               Disable ionospheric delay for spacecraft scenario\n"
		"  -a               Add tropospheric delay (Saastamoinen, standard atmosphere)\n"
		"  -p [fixed_gain]  Disable path loss and hold power level constant\n"
		"  -v               Show details about simulated channels\n",
		((double)USER_MOTION_SIZE) / 10.0, STATIC_MAX_DURATION);

	return;
}

int main(int argc, char *argv[])
{
	clock_t tstart,tend;

	FILE *fp;

	int sv;
	int neph,ieph;
	ephem_t eph[EPHEM_ARRAY_SIZE][MAX_SAT];
	gpstime_t g0;

	double llh[3];

	int i;
	static channel_t chan[MAX_CHAN]; // Too large for the stack
	double elvmask = 0.0; // in degree
	ephem_t *seph;

	int ip,qp;
	int iTable;
	short *iq_buff = NULL;
	signed char *iq8_buff = NULL;

	gpstime_t grx;
	double delt;
	int isamp;

	int iumd;
	int numd;
	char umfile[MAX_PATH_LEN];


	int staticLocationMode = FALSE;
	int nmeaGGA = FALSE;
	int umLLH = FALSE;

	char navfile[MAX_PATH_LEN];
	char outfile[MAX_PATH_LEN];

	double samp_freq;
	int samp_freq_set = FALSE;
	int iq_buff_size;
	int data_format;

	char sysopt[MAX_CHAR];
	int sysmask; // Enabled constellations (bit SYS_GPS, bit SYS_GAL)
	int ngal;
	double rnxver;

	int result;

	int gain[MAX_CHAN];
	double path_loss;
	double ant_gain;
	int fixed_gain = 128;
	double ant_pat[37];
	int ibs; // boresight angle index

	datetime_t t0,tmin,tmax;
	gpstime_t gmin,gmax;
	double dt;
	int igrx;

	double duration;
	int iduration;
	int verb;

	int timeoverwrite = FALSE; // Overwrite the TOC and TOE in the RINEX file
	int download = FALSE; // Download the broadcast ephemerides

	ionoutc_t ionoutc;
	int path_loss_enable = TRUE;

	////////////////////////////////////////////////////////////
	// Read options
	////////////////////////////////////////////////////////////

	// Default options
	navfile[0] = 0;
	umfile[0] = 0;
	sysopt[0] = 0;
	strcpy(outfile, "gpssim.bin");
	samp_freq = 2.6e6;
	data_format = SC16;
	g0.week = -1; // Invalid start time
	iduration = USER_MOTION_SIZE;
	duration = (double)iduration/10.0; // Default duration
	verb = FALSE;
	ionoutc.enable = TRUE;
	ionoutc.tropo = FALSE;
	ionoutc.leapen = FALSE;

	if (argc<3)
	{
		usage();
		exit(1);
	}

	while ((result=getopt(argc,argv,"e:DC:u:x:g:c:l:o:s:b:L:T:t:d:iapv"))!=-1)
	{
		switch (result)
		{
		case 'e':
			snprintf(navfile, MAX_PATH_LEN, "%s", optarg);
			break;
		case 'C':
			strncpy(sysopt, optarg, MAX_CHAR-1);
			sysopt[MAX_CHAR-1] = 0;
			break;
		case 'u':
			snprintf(umfile, MAX_PATH_LEN, "%s", optarg);
			nmeaGGA = FALSE;
			umLLH = FALSE;
			break;
		case 'x':
			// Added by romalvarezllorens@gmail.com
			snprintf(umfile, MAX_PATH_LEN, "%s", optarg);
			umLLH = TRUE;
			break;
		case 'g':
			snprintf(umfile, MAX_PATH_LEN, "%s", optarg);
			nmeaGGA = TRUE;
			break;
		case 'c':
			// Static ECEF coordinates input mode
			staticLocationMode = TRUE;
			sscanf(optarg,"%lf,%lf,%lf",&xyz[0][0],&xyz[0][1],&xyz[0][2]);
			break;
		case 'l':
			// Static geodetic coordinates input mode
			// Added by scateu@gmail.com
			staticLocationMode = TRUE;
			sscanf(optarg,"%lf,%lf,%lf",&llh[0],&llh[1],&llh[2]);
			llh[0] = llh[0] / R2D; // convert to RAD
			llh[1] = llh[1] / R2D; // convert to RAD
			llh2xyz(llh,xyz[0]); // Convert llh to xyz
			break;
		case 'o':
			snprintf(outfile, MAX_PATH_LEN, "%s", optarg);
			break;
		case 's':
			samp_freq = atof(optarg);
			if (samp_freq<1.0e6)
			{
				fprintf(stderr, "ERROR: Invalid sampling frequency.\n");
				exit(1);
			}
			samp_freq_set = TRUE;
			break;
		case 'b':
			data_format = atoi(optarg);
			if (data_format!=SC01 && data_format!=SC08 && data_format!=SC16)
			{
				fprintf(stderr, "ERROR: Invalid I/Q data format.\n");
				exit(1);
			}
			break;
		case 'L':
			// enable custom Leap Event
			ionoutc.leapen = TRUE;
			sscanf(optarg,"%d,%d,%d", &ionoutc.wnlsf, &ionoutc.dn, &ionoutc.dtlsf);
			if (ionoutc.dn<1 && ionoutc.dn>7)
			{
				fprintf(stderr, "ERROR: Invalid GPS day number");
				exit(1);
			}
			if (ionoutc.wnlsf<0)
			{
				fprintf(stderr, "ERROR: Invalid GPS week number");
				exit(1);
			}
			if (ionoutc.dtlsf<-128 && ionoutc.dtlsf>127)
			{
				fprintf(stderr, "ERROR: Invalid delta leap second");
				exit(1);
			}
			break;
		case 'T':
			timeoverwrite = TRUE;
			if (strncmp(optarg, "now", 3)==0)
			{
				time_t timer;
				struct tm *gmt;

				time(&timer);
				gmt = gmtime(&timer);

				t0.y = gmt->tm_year+1900;
				t0.m = gmt->tm_mon+1;
				t0.d = gmt->tm_mday;
				t0.hh = gmt->tm_hour;
				t0.mm = gmt->tm_min;
				t0.sec = (double)gmt->tm_sec;

				date2gps(&t0, &g0);

				break;
			}
		case 't':
			sscanf(optarg, "%d/%d/%d,%d:%d:%lf", &t0.y, &t0.m, &t0.d, &t0.hh, &t0.mm, &t0.sec);
			if (t0.y<=1980 || t0.m<1 || t0.m>12 || t0.d<1 || t0.d>31 ||
				t0.hh<0 || t0.hh>23 || t0.mm<0 || t0.mm>59 || t0.sec<0.0 || t0.sec>=60.0)
			{
				fprintf(stderr, "ERROR: Invalid date and time.\n");
				exit(1);
			}
			t0.sec = floor(t0.sec);
			date2gps(&t0, &g0);
			break;
		case 'd':
			duration = atof(optarg);
			break;
		case 'i':
			ionoutc.enable = FALSE; // Disable ionospheric correction
			break;
		case 'a':
			ionoutc.tropo = TRUE; // Enable tropospheric delay
			break;
		case 'p':
			if (optind < argc && argv[optind][0] != '-') // Check if next item is an argument
			{
				fixed_gain = atoi(argv[optind]);
				if (fixed_gain < 1 || fixed_gain > 128)
				{
					fprintf(stderr, "ERROR: Fixed gain must be between 1 and 128.\n");
					exit(1);
				}
				optind++;  // Move past this argument for next iteration
			}
			path_loss_enable = FALSE; // Disable path loss
			break;
		case 'v':
			verb = TRUE;
			break;
		case 'D':
			download = TRUE;
			break;
		case ':':
		case '?':
			usage();
			exit(1);
		default:
			break;
		}
	}

	if (download==TRUE)
	{
		gpstime_t gdl;
		int nday;

		if (navfile[0]!=0)
		{
			fprintf(stderr, "ERROR: Use either -e or -D.\n");
			exit(1);
		}

		if (g0.week>=0)
		{
			// Scenario date. With -T any recent day will do, as TOC and TOE are overwritten.
			gdl = g0;
			nday = (timeoverwrite==TRUE) ? 2 : 1;
		}
		else
		{
			// Today, or yesterday early in the day
			time_t timer;
			struct tm *gmt;
			datetime_t tnow;

			time(&timer);
			gmt = gmtime(&timer);
			tnow.y = gmt->tm_year+1900;
			tnow.m = gmt->tm_mon+1;
			tnow.d = gmt->tm_mday;
			tnow.hh = gmt->tm_hour;
			tnow.mm = gmt->tm_min;
			tnow.sec = (double)gmt->tm_sec;
			date2gps(&tnow, &gdl);
			nday = 2;
		}

		if (downloadRinexNav(gdl, nday, navfile)!=0)
		{
			fprintf(stderr, "ERROR: Failed to download the broadcast ephemerides.\n");
			exit(1);
		}
	}

	if (navfile[0]==0)
	{
		fprintf(stderr, "ERROR: Navigation file is not specified (use -e or -D).\n");
		exit(1);
	}

	if (umfile[0]==0 && !staticLocationMode)
	{
		// Default static location; Tokyo
		staticLocationMode = TRUE;
		llh[0] = 35.681298 / R2D;
		llh[1] = 139.766247 / R2D;
		llh[2] = 10.0;
	}

	if (duration<0.0 || (duration>((double)USER_MOTION_SIZE)/10.0 && !staticLocationMode) || (duration>STATIC_MAX_DURATION && staticLocationMode))
	{
		fprintf(stderr, "ERROR: Invalid duration.\n");
		exit(1);
	}
	iduration = (int)(duration*10.0 + 0.5);

	////////////////////////////////////////////////////////////
	// Receiver position
	////////////////////////////////////////////////////////////

	if (!staticLocationMode)
	{
		// Read user motion file
		if (nmeaGGA==TRUE)
			numd = readNmeaGGA(xyz, umfile);
		else if (umLLH == TRUE)
			numd = readUserMotionLLH(xyz, umfile);
		else
			numd = readUserMotion(xyz, umfile);

		if (numd==-1)
		{
			fprintf(stderr, "ERROR: Failed to open user motion / NMEA GGA file.\n");
			exit(1);
		}
		else if (numd==0)
		{
			fprintf(stderr, "ERROR: Failed to read user motion / NMEA GGA data.\n");
			exit(1);
		}

		// Set simulation duration
		if (numd>iduration)
			numd = iduration;

		// Set user initial position
		xyz2llh(xyz[0], llh);
	}
	else
	{
		// Static geodetic coordinates input mode: "-l"
		// Added by scateu@gmail.com
		fprintf(stderr, "Using static location mode.\n");

		// Set simulation duration
		numd = iduration;

		// Set user initial position
		llh2xyz(llh, xyz[0]);
	}

	fprintf(stderr, "xyz = %11.1f, %11.1f, %11.1f\n", xyz[0][0], xyz[0][1], xyz[0][2]);
	fprintf(stderr, "llh = %11.6f, %11.6f, %11.1f\n", llh[0]*R2D, llh[1]*R2D, llh[2]);

	////////////////////////////////////////////////////////////
	// Read ephemeris
	////////////////////////////////////////////////////////////

	rnxver = rinexVersion(navfile);

	if (rnxver>=3.0)
	{
		neph = readRinex3Nav(eph, &ionoutc, navfile);
	}
	else
	{
		neph = readRinexNavAll(eph, &ionoutc, navfile);

		for (sv=0; sv<MAX_SAT_GAL; sv++)
			galneph[sv] = 0;
	}

	if (neph==-1)
	{
		fprintf(stderr, "ERROR: ephemeris file not found.\n");
		exit(1);
	}

	ngal = 0;
	for (sv=0; sv<MAX_SAT_GAL; sv++)
		ngal += galneph[sv];

	// Enabled constellations
	if (sysopt[0]==0)
	{
		sysmask = 0;
		if (neph>0)
			sysmask |= (1<<SYS_GPS);
		if (ngal>0)
			sysmask |= (1<<SYS_GAL);
	}
	else
	{
		sysmask = 0;
		for (i=0; sysopt[i]!=0; i++)
		{
			if (sysopt[i]=='G' || sysopt[i]=='g')
				sysmask |= (1<<SYS_GPS);
			else if (sysopt[i]=='E' || sysopt[i]=='e')
				sysmask |= (1<<SYS_GAL);
			else if (sysopt[i]!=',')
			{
				fprintf(stderr, "ERROR: Unknown constellation '%c'.\n", sysopt[i]);
				exit(1);
			}
		}

		if ((sysmask & (1<<SYS_GPS)) && neph==0)
		{
			fprintf(stderr, "ERROR: No GPS ephemeris in the navigation file.\n");
			exit(1);
		}
		if ((sysmask & (1<<SYS_GAL)) && ngal==0)
		{
			fprintf(stderr, "ERROR: No Galileo I/NAV ephemeris in the navigation file.\n");
			exit(1);
		}
	}

	if (sysmask==0)
	{
		fprintf(stderr, "ERROR: No ephemeris available.\n");
		exit(1);
	}

	fprintf(stderr, "Constellations: %s%s\n",
		(sysmask & (1<<SYS_GPS)) ? "GPS " : "", (sysmask & (1<<SYS_GAL)) ? "Galileo" : "");

	// Buffer size
	if ((sysmask & (1<<SYS_GAL)) && samp_freq_set==FALSE)
		samp_freq = 5.0e6; // Room for the BOC(1,1) main lobes of Galileo E1

	if ((sysmask & (1<<SYS_GAL)) && samp_freq<4.0e6)
		fprintf(stderr, "WARNING: Sampling frequency below 4 MHz truncates the Galileo E1 BOC(1,1) spectrum.\n");

	samp_freq = floor(samp_freq/10.0);
	iq_buff_size = (int)samp_freq; // samples per 0.1sec
	samp_freq *= 10.0;

	delt = 1.0/samp_freq;

	if ((verb==TRUE)&&(ionoutc.vflg==TRUE))
	{
		fprintf(stderr, "  %12.3e %12.3e %12.3e %12.3e\n",
			ionoutc.alpha0, ionoutc.alpha1, ionoutc.alpha2, ionoutc.alpha3);
		fprintf(stderr, "  %12.3e %12.3e %12.3e %12.3e\n",
			ionoutc.beta0, ionoutc.beta1, ionoutc.beta2, ionoutc.beta3);
		fprintf(stderr, "   %19.11e %19.11e  %9d %9d\n",
			ionoutc.A0, ionoutc.A1, ionoutc.tot, ionoutc.wnt);
		fprintf(stderr, "%6d\n", ionoutc.dtls);
	}

	gmin.sec = 0;
	gmin.week = 0;
	gmax.sec = 0;
	gmax.week = 0;
	tmax.sec = 0;
	tmax.mm = 0;
	tmax.hh = 0;
	tmax.d = 0;
	tmax.m = 0;
	tmax.y = 0;

	if (sysmask & (1<<SYS_GPS))
	{
		for (sv=0; sv<MAX_SAT; sv++)
		{
			if (eph[0][sv].vflg==1)
			{
				gmin = eph[0][sv].toc;
				tmin = eph[0][sv].t;
				break;
			}
		}

		for (sv=0; sv<MAX_SAT; sv++)
		{
			if (eph[neph-1][sv].vflg == 1)
			{
				gmax = eph[neph-1][sv].toc;
				tmax = eph[neph-1][sv].t;
				break;
			}
		}
	}
	else // Galileo only: span of the Galileo records
	{
		gmin.week = -1;

		for (sv=0; sv<MAX_SAT_GAL; sv++)
		{
			for (i=0; i<galneph[sv]; i++)
			{
				if (gmin.week<0 || subGpsTime(galeph[sv][i].toc, gmin)<0.0)
				{
					gmin = galeph[sv][i].toc;
					tmin = galeph[sv][i].t;
				}
				if (subGpsTime(galeph[sv][i].toc, gmax)>0.0)
				{
					gmax = galeph[sv][i].toc;
					tmax = galeph[sv][i].t;
				}
			}
		}
	}

	if (g0.week>=0) // Scenario start time has been set.
	{
		if (timeoverwrite==TRUE)
		{
			gpstime_t gtmp;
			datetime_t ttmp;
			double dsec;

			gtmp.week = g0.week;
			gtmp.sec = (double)(((int)(g0.sec))/7200)*7200.0;

			dsec = subGpsTime(gtmp,gmin);

			// Overwrite the UTC reference week number
			ionoutc.wnt = gtmp.week;
			ionoutc.tot = (int)gtmp.sec;

			// Iono/UTC parameters may no longer valid
			//ionoutc.vflg = FALSE;

			// Overwrite the TOC and TOE to the scenario start time
			for (sv=0; sv<MAX_SAT; sv++)
			{
				for (i=0; i<neph; i++)
				{
					if (eph[i][sv].vflg == 1)
					{
						gtmp = incGpsTime(eph[i][sv].toc, dsec);
						gps2date(&gtmp,&ttmp);
						eph[i][sv].toc = gtmp;
						eph[i][sv].t = ttmp;

						gtmp = incGpsTime(eph[i][sv].toe, dsec);
						eph[i][sv].toe = gtmp;
					}
				}
			}

			for (sv=0; sv<MAX_SAT_GAL; sv++)
			{
				for (i=0; i<galneph[sv]; i++)
				{
					gtmp = incGpsTime(galeph[sv][i].toc, dsec);
					gps2date(&gtmp,&ttmp);
					galeph[sv][i].toc = gtmp;
					galeph[sv][i].t = ttmp;

					galeph[sv][i].toe = incGpsTime(galeph[sv][i].toe, dsec);
				}
			}

			// Galileo UTC and GGTO reference times follow the shifted ephemerides
			galutc.wnt = g0.week;
			galutc.tot = ((int)g0.sec/3600)*3600;
			galutc.wn0g = g0.week;
			galutc.t0g = ((int)g0.sec/3600)*3600;
		}
		else
		{
			if (subGpsTime(g0, gmin)<0.0 || subGpsTime(gmax, g0)<0.0)
			{
				fprintf(stderr, "ERROR: Invalid start time.\n");
				fprintf(stderr, "tmin = %4d/%02d/%02d,%02d:%02d:%02.0f (%d:%.0f)\n",
					tmin.y, tmin.m, tmin.d, tmin.hh, tmin.mm, tmin.sec,
					gmin.week, gmin.sec);
				fprintf(stderr, "tmax = %4d/%02d/%02d,%02d:%02d:%02.0f (%d:%.0f)\n",
					tmax.y, tmax.m, tmax.d, tmax.hh, tmax.mm, tmax.sec,
					gmax.week, gmax.sec);
				exit(1);
			}
		}
	}
	else
	{
		g0 = gmin;
		t0 = tmin;
	}

	fprintf(stderr, "Start time = %4d/%02d/%02d,%02d:%02d:%02.0f (%d:%.0f)\n",
		t0.y, t0.m, t0.d, t0.hh, t0.mm, t0.sec, g0.week, g0.sec);
	fprintf(stderr, "Duration = %.1f [sec]\n", ((double)numd)/10.0);

	// Select the current set of ephemerides
	ieph = -1;

	if (!(sysmask & (1<<SYS_GPS)))
		ieph = 0; // No GPS satellites to simulate

	for (i=0; i<neph && ieph<0; i++)
	{
		for (sv=0; sv<MAX_SAT; sv++)
		{
			if (eph[i][sv].vflg == 1)
			{
				dt = subGpsTime(g0, eph[i][sv].toc);
				if (dt>=-SECONDS_IN_HOUR && dt<SECONDS_IN_HOUR)
				{
					ieph = i;
					break;
				}
			}
		}

		if (ieph>=0) // ieph has been set
			break;
	}

	if (ieph == -1)
	{
		fprintf(stderr, "ERROR: No current set of ephemerides has been found.\n");
		exit(1);
	}

	if (sysmask & (1<<SYS_GAL))
	{
		// Galileo ephemerides in use and almanac at the start time
		selectGalEph(g0);
		galAlmanac(g0);

		ngal = 0;
		for (sv=0; sv<MAX_SAT_GAL; sv++)
			ngal += galcur[sv].vflg;

		if (ngal==0)
		{
			fprintf(stderr, "ERROR: No Galileo ephemeris valid at the start time.\n");
			exit(1);
		}
	}

	////////////////////////////////////////////////////////////
	// Baseband signal buffer and output file
	////////////////////////////////////////////////////////////

	// Allocate I/Q buffer
	iq_buff = calloc(2*iq_buff_size, 2);

	if (iq_buff==NULL)
	{
		fprintf(stderr, "ERROR: Failed to allocate 16-bit I/Q buffer.\n");
		exit(1);
	}

	if (data_format==SC08)
	{
		iq8_buff = calloc(2*iq_buff_size, 1);
		if (iq8_buff==NULL)
		{
			fprintf(stderr, "ERROR: Failed to allocate 8-bit I/Q buffer.\n");
			exit(1);
		}
	}
	else if (data_format==SC01)
	{
		iq8_buff = calloc(iq_buff_size/4, 1); // byte = {I0, Q0, I1, Q1, I2, Q2, I3, Q3}
		if (iq8_buff==NULL)
		{
			fprintf(stderr, "ERROR: Failed to allocate compressed 1-bit I/Q buffer.\n");
			exit(1);
		}
	}

	// Open output file
	// "-" can be used as name for stdout
	if(strcmp("-", outfile)){
		if (NULL==(fp=fopen(outfile,"wb")))
		{
			fprintf(stderr, "ERROR: Failed to open output file.\n");
			exit(1);
		}
	}else{
		fp = stdout;
	}

	////////////////////////////////////////////////////////////
	// Initialize channels
	////////////////////////////////////////////////////////////

	// Clear all channels
	for (i=0; i<MAX_CHAN; i++)
		chan[i].prn = 0;

	// Clear satellite allocation flag
	for (sv=0; sv<MAX_SAT+MAX_SAT_GAL; sv++)
		allocatedSat[sv] = -1;

	// Initial reception time
	grx = incGpsTime(g0, 0.0);

	// Allocate visible satellites
	allocateChannel(chan, eph[ieph], ionoutc, grx, xyz[0], elvmask, sysmask);

	for(i=0; i<MAX_CHAN; i++)
	{
		if (chan[i].prn>0)
			fprintf(stderr, "%c%02d %6.1f %5.1f %11.1f %5.1f %5.1f\n", (chan[i].sys==SYS_GAL)?'E':'G', chan[i].prn,
				chan[i].azel[0]*R2D, chan[i].azel[1]*R2D, chan[i].rho0.d, chan[i].rho0.iono_delay, chan[i].rho0.tropo_delay);
	}

	////////////////////////////////////////////////////////////
	// Receiver antenna gain pattern
	////////////////////////////////////////////////////////////

	for (i=0; i<37; i++)
		ant_pat[i] = pow(10.0, -ant_pat_db[i]/20.0);

	////////////////////////////////////////////////////////////
	// Generate baseband signals
	////////////////////////////////////////////////////////////

	tstart = clock();

	// Update receiver time
	grx = incGpsTime(grx, 0.1);

	for (iumd=1; iumd<numd; iumd++)
	{
		for (i=0; i<MAX_CHAN; i++)
		{
			if (chan[i].prn>0)
			{
				// Refresh code phase and data bit counters
				range_t rho;
				sv = chan[i].prn-1;
				seph = (chan[i].sys==SYS_GAL) ? &galcur[sv] : &eph[ieph][sv];

				// Current pseudorange
				if (!staticLocationMode)
					computeRange(&rho, *seph, &ionoutc, grx, xyz[iumd]);
				else
					computeRange(&rho, *seph, &ionoutc, grx, xyz[0]);

				chan[i].azel[0] = rho.azel[0];
				chan[i].azel[1] = rho.azel[1];

				// Update code phase and data bit counters
				computeCodePhase(&chan[i], rho, 0.1);
#ifndef FLOAT_CARR_PHASE
				chan[i].carr_phasestep = (int)round(512.0 * 65536.0 * chan[i].f_carr * delt);
#endif
				// Receiver antenna gain
				ibs = (int)((90.0-rho.azel[1]*R2D)/5.0); // covert elevation to boresight
				ant_gain = ant_pat[ibs];

				if (chan[i].sys==SYS_GAL)
				{
					// Path loss, normalized to the Galileo orbit altitude
					path_loss = 23222000.0/rho.d;

					// Signal gain. The E1-B minus E1-C chip sum has twice the power of
					// a C/A chip: GAL_E1_AMP sets E1 OS 1.25 dB above GPS C/A.
					if (path_loss_enable == TRUE)
						gain[i] = (int)(path_loss * ant_gain * 128.0 * GAL_E1_AMP); // scaled by 2^7
					else
						gain[i] = (int)(fixed_gain * GAL_E1_AMP); // hold the power level constant
				}
				else
				{
					// Path loss
					path_loss = 20200000.0/rho.d;

					// Signal gain
					if (path_loss_enable == TRUE)
						gain[i] = (int)(path_loss * ant_gain * 128.0); // scaled by 2^7
					else
						gain[i] = fixed_gain; // hold the power level constant
				}
			}
		}

		for (isamp=0; isamp<iq_buff_size; isamp++)
		{
			int i_acc = 0;
			int q_acc = 0;

			for (i=0; i<MAX_CHAN; i++)
			{
				if (chan[i].prn>0)
				{
#ifdef FLOAT_CARR_PHASE
					iTable = (int)floor(chan[i].carr_phase*512.0);
#else
					iTable = (chan[i].carr_phase >> 16) & 0x1ff; // 9-bit index
#endif
					if (chan[i].sys==SYS_GAL)
					{
						// Galileo E1 OS: (E1-B - E1-C) on a BOC(1,1) sub-carrier
						int chip = (int)chan[i].code_phase;
						int boc = ((int)(chan[i].code_phase*2.0) & 0x1) ? -1 : 1;
						int e1 = (chan[i].dataBit*chan[i].ca[chip] - chan[i].secCode*chan[i].pilot[chip])*boc;

						ip = e1 * cosTable512[iTable] * gain[i];
						qp = e1 * sinTable512[iTable] * gain[i];

						// Accumulate for all visible satellites
						i_acc += ip;
						q_acc += qp;

						// Update code phase
						chan[i].code_phase += chan[i].f_code * delt;

						if (chan[i].code_phase>=GAL_CODE_LEN)
						{
							chan[i].code_phase -= GAL_CODE_LEN;

							// 1 primary code = 1 navigation symbol = 1 secondary code chip
							if (chan[i].isym<GAL_SYM_BUF-1)
								chan[i].isym++;

							chan[i].dataBit = 1-2*chan[i].sym[chan[i].isym];
							chan[i].secCode = 1-2*galCs25[chan[i].isym%GAL_SEC_CODE_LEN];
						}
					}
					else
					{
						ip = chan[i].dataBit * chan[i].codeCA * cosTable512[iTable] * gain[i];
						qp = chan[i].dataBit * chan[i].codeCA * sinTable512[iTable] * gain[i];

						// Accumulate for all visible satellites
						i_acc += ip;
						q_acc += qp;

						// Update code phase
						chan[i].code_phase += chan[i].f_code * delt;

						if (chan[i].code_phase>=CA_SEQ_LEN)
						{
							chan[i].code_phase -= CA_SEQ_LEN;

							chan[i].icode++;

							if (chan[i].icode>=20) // 20 C/A codes = 1 navigation data bit
							{
								chan[i].icode = 0;
								chan[i].ibit++;

								if (chan[i].ibit>=30) // 30 navigation data bits = 1 word
								{
									chan[i].ibit = 0;
									chan[i].iword++;
									/*
									if (chan[i].iword>=N_DWRD)
										fprintf(stderr, "\nWARNING: Subframe word buffer overflow.\n");
									*/
								}

								// Set new navigation data bit
								chan[i].dataBit = (int)((chan[i].dwrd[chan[i].iword]>>(29-chan[i].ibit)) & 0x1UL)*2-1;
							}
						}

						// Set current code chip
						chan[i].codeCA = chan[i].ca[(int)chan[i].code_phase]*2-1;
					}

					// Update carrier phase
#ifdef FLOAT_CARR_PHASE
					chan[i].carr_phase += chan[i].f_carr * delt;

					if (chan[i].carr_phase >= 1.0)
						chan[i].carr_phase -= 1.0;
					else if (chan[i].carr_phase<0.0)
						chan[i].carr_phase += 1.0;
#else
					chan[i].carr_phase += chan[i].carr_phasestep;
#endif
				}
			}

			// Scaled by 2^7
			i_acc = (i_acc+64)>>7;
			q_acc = (q_acc+64)>>7;

			// Store I/Q samples into buffer
			iq_buff[isamp*2] = (short)i_acc;
			iq_buff[isamp*2+1] = (short)q_acc;
		}

		if (data_format==SC01)
		{
			for (isamp=0; isamp<2*iq_buff_size; isamp++)
			{
				if (isamp%8==0)
					iq8_buff[isamp/8] = 0x00;

				iq8_buff[isamp/8] |= (iq_buff[isamp]>0?0x01:0x00)<<(7-isamp%8);
			}

			fwrite(iq8_buff, 1, iq_buff_size/4, fp);
		}
		else if (data_format==SC08)
		{
			for (isamp=0; isamp<2*iq_buff_size; isamp++)
			{
				iq8_buff[isamp] = iq_buff[isamp]>>4; // 12-bit bladeRF -> 8-bit HackRF
				//iq8_buff[isamp] = iq_buff[isamp] >> 8; // for PocketSDR
			}

			fwrite(iq8_buff, 1, 2*iq_buff_size, fp);
		}
		else // data_format==SC16
		{
			fwrite(iq_buff, 2, 2*iq_buff_size, fp);
		}

		//
		// Update navigation message and channel allocation every 30 seconds
		//

		igrx = (int)(grx.sec*10.0+0.5);

		if (igrx%300==0) // Every 30 seconds
		{
			// Select the Galileo ephemerides for the next sub-frame
			if (sysmask & (1<<SYS_GAL))
				selectGalEph(grx);

			// Update navigation message
			for (i=0; i<MAX_CHAN; i++)
			{
				if (chan[i].prn>0)
				{
					if (chan[i].sys==SYS_GAL)
						generateGalNavMsg(grx, &ionoutc, &chan[i]);
					else
						generateNavMsg(grx, &chan[i], 0);
				}
			}

			// Refresh ephemeris and subframes
			// Quick and dirty fix. Need more elegant way.
			for (sv=0; sv<MAX_SAT && ieph+1<neph; sv++)
			{
				if (eph[ieph+1][sv].vflg==1)
				{
					dt = subGpsTime(eph[ieph+1][sv].toc, grx);
					if (dt<SECONDS_IN_HOUR)
					{
						ieph++;

						for (i=0; i<MAX_CHAN; i++)
						{
							// Generate new subframes if allocated
							if (chan[i].prn!=0 && chan[i].sys==SYS_GPS)
								eph2sbf(eph[ieph][chan[i].prn-1], ionoutc, chan[i].sbf);
						}
					}

					break;
				}
			}

			// Update channel allocation
			if (!staticLocationMode)
				allocateChannel(chan, eph[ieph], ionoutc, grx, xyz[iumd], elvmask, sysmask);
			else
				allocateChannel(chan, eph[ieph], ionoutc, grx, xyz[0], elvmask, sysmask);

			// Show details about simulated channels
			if (verb==TRUE)
			{
				fprintf(stderr, "\n");
				for (i=0; i<MAX_CHAN; i++)
				{
					if (chan[i].prn>0)
						fprintf(stderr, "%c%02d %6.1f %5.1f %11.1f %5.1f %5.1f\n", (chan[i].sys==SYS_GAL)?'E':'G', chan[i].prn,
							chan[i].azel[0]*R2D, chan[i].azel[1]*R2D, chan[i].rho0.d, chan[i].rho0.iono_delay, chan[i].rho0.tropo_delay);
				}
			}
		}

		// Update receiver time
		grx = incGpsTime(grx, 0.1);

		// Update time counter
		fprintf(stderr, "\rTime into run = %4.1f", subGpsTime(grx, g0));
		fflush(stdout);
	}

	tend = clock();

	fprintf(stderr, "\nDone!\n");

	// Free I/Q buffer
	free(iq_buff);

	// Close file
	fclose(fp);

	// Process time
	fprintf(stderr, "Process time = %.1f [sec]\n", (double)(tend-tstart)/CLOCKS_PER_SEC);

	return(0);
}
