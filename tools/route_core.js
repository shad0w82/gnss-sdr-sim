// Trajectory generation for gnss-sdr-sim: route -> 10 Hz user motion.
//
// Same algorithm as build_trajectory() in route2motion.py: both tools give the same
// trajectory for the same route. Used by route_planner.html; also loadable with Node.js
// (module.exports) for testing.
(function (root) {
	'use strict';

	var WGS84_A = 6378137.0;
	var WGS84_E2 = 0.0818191908426 * 0.0818191908426;

	var DEFAULTS = {
		speed_kmh: 50.0,  // speed when the route does not give one
		accel: 1.5,       // longitudinal acceleration/deceleration [m/s^2]
		lat_accel: 2.0,   // lateral acceleration in curves [m/s^2]
		smooth: 20.0,     // smoothing window [m]
		hold_start: 60.0, // stop at the start [s]
		hold_end: 10.0,   // stop at the end [s]
		agl: 0.0          // antenna height above the route heights [m]
	};

	var RESAMPLE_STEP = 1.0; // resampling step [m]
	var CURVATURE_BASE = 1;  // samples on each side for the curvature estimate
	var CURVE_WINDOW = 2;    // samples on each side where a curve speed limit also applies
	var DEM_STEP = 30.0;     // terrain sampling step [m]
	var RATE = 10;           // output rate [Hz]

	var D2R = Math.PI / 180.0;

	// ------------------------------------------------------------ geodesy

	function llh2ecef(lat, lon, h) {
		var la = lat * D2R, lo = lon * D2R;
		var n = WGS84_A / Math.sqrt(1.0 - WGS84_E2 * Math.sin(la) * Math.sin(la));
		return [(n + h) * Math.cos(la) * Math.cos(lo),
			(n + h) * Math.cos(la) * Math.sin(lo),
			(n * (1.0 - WGS84_E2) + h) * Math.sin(la)];
	}

	function ecef2llh(x, y, z) {
		var lon = Math.atan2(y, x);
		var p = Math.hypot(x, y);
		var lat = Math.atan2(z, p * (1.0 - WGS84_E2));
		var h = 0.0;
		for (var i = 0; i < 10; i++) {
			var n = WGS84_A / Math.sqrt(1.0 - WGS84_E2 * Math.sin(lat) * Math.sin(lat));
			h = p / Math.cos(lat) - n;
			lat = Math.atan2(z, p * (1.0 - WGS84_E2 * n / (n + h)));
		}
		return [lat / D2R, lon / D2R, h];
	}

	// East-North-Up frame at a reference point (exact rotation of ECEF, not a projection)
	function LocalFrame(lat, lon) {
		this.o = llh2ecef(lat, lon, 0.0);
		var la = lat * D2R, lo = lon * D2R;
		var sla = Math.sin(la), cla = Math.cos(la), slo = Math.sin(lo), clo = Math.cos(lo);
		this.r = [[-slo, clo, 0.0], [-sla * clo, -sla * slo, cla], [cla * clo, cla * slo, sla]];
	}

	LocalFrame.prototype.toEnu = function (lat, lon) {
		var x = llh2ecef(lat, lon, 0.0), r = this.r;
		var d = [x[0] - this.o[0], x[1] - this.o[1], x[2] - this.o[2]];
		return [0, 1, 2].map(function (i) { return r[i][0] * d[0] + r[i][1] * d[1] + r[i][2] * d[2]; });
	};

	LocalFrame.prototype.toLatLon = function (e) {
		var o = this.o, r = this.r;
		var x = [0, 1, 2].map(function (j) { return o[j] + r[0][j] * e[0] + r[1][j] * e[1] + r[2][j] * e[2]; });
		var llh = ecef2llh(x[0], x[1], x[2]);
		return [llh[0], llh[1]];
	};

	function dist(a, b) {
		return Math.hypot(a[0] - b[0], a[1] - b[1], a[2] - b[2]);
	}

	function lerp3(a, b, f) {
		return [a[0] + f * (b[0] - a[0]), a[1] + f * (b[1] - a[1]), a[2] + f * (b[2] - a[2])];
	}

	// ------------------------------------------------------------ geoid

	// EGM96 geoid undulation from the grid of geoid_egm96.js (bilinear interpolation)
	function Geoid(grid) {
		var bin = (typeof atob === 'function') ? atob(grid.data) : Buffer.from(grid.data, 'base64').toString('binary');
		var n = grid.rows * grid.cols;
		this.v = new Int16Array(n);
		for (var i = 0; i < n; i++) {
			var u = bin.charCodeAt(2 * i) | (bin.charCodeAt(2 * i + 1) << 8);
			this.v[i] = u >= 32768 ? u - 65536 : u;
		}
		this.rows = grid.rows; this.cols = grid.cols;
		this.lat0 = grid.lat0; this.lon0 = grid.lon0; this.step = grid.step;
	}

	Geoid.prototype.undulation = function (lat, lon) {
		var r = (this.lat0 - lat) / this.step;
		var c = (((lon - this.lon0) % 360.0) + 360.0) % 360.0 / this.step;
		var r0 = Math.min(Math.max(Math.floor(r), 0), this.rows - 2);
		var c0 = Math.floor(c) % this.cols;
		var c1 = (c0 + 1) % this.cols;
		var fr = r - r0, fc = c - Math.floor(c), v = this.v, nc = this.cols;
		var n = v[r0 * nc + c0] * (1 - fr) * (1 - fc) + v[r0 * nc + c1] * (1 - fr) * fc +
			v[(r0 + 1) * nc + c0] * fr * (1 - fc) + v[(r0 + 1) * nc + c1] * fr * fc;
		return n * 0.01;
	};

	// ------------------------------------------------------------ trajectory

	// points: [[lat, lon, h], ...], h null when unknown (height above msl, or ellipsoidal
	//         with opts.height_ref = 'ellipsoid')
	// speeds: speed of each segment [m/s] (points.length - 1)
	// demHeights: terrain heights above msl at the points requested by a previous call, or null.
	//         When the route has no heights and demHeights is null, the function throws
	//         {needDem: [[lat, lon], ...]}: fetch the heights of these points and call again.
	// Returns {rows: [[t, lat, lon, h_ellipsoidal], ...] at 10 Hz, summary: {...}}.
	function buildTrajectory(points, speeds, opts, geoid, demHeights) {
		var i, j, k, f, a, b;

		// Drop repeated points
		var frame = new LocalFrame(points[0][0], points[0][1]);
		var pts = [points[0]], spd = [], enu = [frame.toEnu(points[0][0], points[0][1])];
		for (i = 1; i < points.length; i++) {
			var e = frame.toEnu(points[i][0], points[i][1]);
			if (dist(e, enu[enu.length - 1]) > 0.01) {
				pts.push(points[i]); enu.push(e); spd.push(speeds[i - 1]);
			}
		}
		if (pts.length < 2)
			throw new Error('the route has no length');

		// Resample every RESAMPLE_STEP metres along the polyline
		var cum = [0.0];
		for (i = 1; i < enu.length; i++)
			cum.push(cum[i - 1] + dist(enu[i - 1], enu[i]));
		var length = cum[cum.length - 1];
		var n = Math.ceil(length / RESAMPLE_STEP) + 1;
		var pos = [], hgt = [], vmax = [];
		var seg = 0;
		for (k = 0; k < n; k++) {
			var sk = length * k / (n - 1);
			while (seg < enu.length - 2 && sk > cum[seg + 1])
				seg++;
			f = (sk - cum[seg]) / (cum[seg + 1] - cum[seg]);
			pos.push(lerp3(enu[seg], enu[seg + 1], f));
			var ha = pts[seg][2], hb = pts[seg + 1][2];
			hgt.push(ha == null || hb == null ? null : ha + f * (hb - ha));
			vmax.push(spd[seg]);
		}

		// Heights above msl from the terrain model where the route has none
		var msl = (opts.height_ref || 'msl') === 'msl';
		if (hgt.some(function (h) { return h == null; })) {
			var idx = [], stepk = Math.max(1, Math.round(DEM_STEP / RESAMPLE_STEP));
			for (k = 0; k < n; k += stepk)
				idx.push(k);
			if (idx[idx.length - 1] !== n - 1)
				idx.push(n - 1);
			if (!demHeights || demHeights.length !== idx.length)
				throw { needDem: idx.map(function (q) { return frame.toLatLon(pos[q]); }) };
			j = 0;
			for (k = 0; k < n; k++) {
				while (j < idx.length - 2 && k > idx[j + 1])
					j++;
				f = (k - idx[j]) / (idx[j + 1] - idx[j]);
				hgt[k] = demHeights[j] + f * (demHeights[j + 1] - demHeights[j]);
			}
			msl = true;
		}

		// Smoothing: centred moving average, window shrinking at the ends so that they do not move
		var half = Math.round(opts.smooth / RESAMPLE_STEP / 2);
		if (half > 0) {
			var cs = [new Float64Array(n + 1), new Float64Array(n + 1), new Float64Array(n + 1), new Float64Array(n + 1)];
			for (k = 0; k < n; k++) {
				for (j = 0; j < 3; j++)
					cs[j][k + 1] = cs[j][k] + pos[k][j];
				cs[3][k + 1] = cs[3][k] + hgt[k];
			}
			var sp = [], sh = [];
			for (k = 0; k < n; k++) {
				var w = Math.min(half, k, n - 1 - k);
				a = k - w; b = k + w + 1;
				sp.push([(cs[0][b] - cs[0][a]) / (b - a), (cs[1][b] - cs[1][a]) / (b - a), (cs[2][b] - cs[2][a]) / (b - a)]);
				sh.push((cs[3][b] - cs[3][a]) / (b - a));
			}
			pos = sp; hgt = sh;
		}

		// Resample the smoothed path at uniform arc length (smoothing brings samples closer in curves)
		var s0 = [0.0];
		for (k = 1; k < n; k++)
			s0.push(s0[k - 1] + Math.hypot(dist(pos[k - 1], pos[k]), hgt[k] - hgt[k - 1]));
		var n2 = Math.ceil(s0[n - 1] / RESAMPLE_STEP) + 1;
		var up = [], uh = [], uv = [], s = [];
		j = 0;
		for (k = 0; k < n2; k++) {
			var sk2 = s0[n - 1] * k / (n2 - 1);
			while (j < n - 2 && sk2 > s0[j + 1])
				j++;
			f = s0[j + 1] > s0[j] ? (sk2 - s0[j]) / (s0[j + 1] - s0[j]) : 0.0;
			up.push(lerp3(pos[j], pos[j + 1], f));
			uh.push(hgt[j] + f * (hgt[j + 1] - hgt[j]));
			uv.push(f < 0.5 ? vmax[j] : vmax[j + 1]);
			s.push(sk2);
		}
		pos = up; hgt = uh; vmax = uv; n = n2;

		// Speed limit in curves: v = sqrt(a_lat / curvature), curvature from the circle through
		// the samples CURVATURE_BASE before and after. The limit also applies to the CURVE_WINDOW
		// neighbouring samples, as the curvature peak may fall between samples.
		var m = CURVATURE_BASE;
		var vcurve = vmax.slice();
		for (k = m; k < n - m; k++) {
			var pa = pos[k - m], pb = pos[k], pc = pos[k + m];
			var ab = dist(pa, pb), bc = dist(pb, pc), ca = dist(pc, pa);
			var u = [pb[0] - pa[0], pb[1] - pa[1], pb[2] - pa[2]];
			var ww = [pc[0] - pa[0], pc[1] - pa[1], pc[2] - pa[2]];
			var cross = Math.sqrt(Math.pow(u[1] * ww[2] - u[2] * ww[1], 2) + Math.pow(u[2] * ww[0] - u[0] * ww[2], 2) +
				Math.pow(u[0] * ww[1] - u[1] * ww[0], 2));
			if (ab * bc * ca > 0.0 && cross > 0.0) {
				var kappa = 2.0 * cross / (ab * bc * ca);
				var vc = Math.sqrt(opts.lat_accel / kappa);
				for (var q = Math.max(k - CURVE_WINDOW, 0); q <= Math.min(k + CURVE_WINDOW, n - 1); q++)
					vcurve[q] = Math.min(vcurve[q], vc);
			}
		}
		vmax = vcurve;

		// Acceleration limits: start and stop at rest
		var v = vmax.slice();
		v[0] = 0.0;
		v[n - 1] = 0.0;
		for (k = 1; k < n; k++)
			v[k] = Math.min(v[k], Math.sqrt(v[k - 1] * v[k - 1] + 2.0 * opts.accel * (s[k] - s[k - 1])));
		for (k = n - 2; k >= 0; k--)
			v[k] = Math.min(v[k], Math.sqrt(v[k + 1] * v[k + 1] + 2.0 * opts.accel * (s[k + 1] - s[k])));

		// Time along the path
		var t = [0.0];
		for (k = 1; k < n; k++) {
			var vm = v[k - 1] + v[k];
			t.push(t[k - 1] + (vm > 0.0 ? 2.0 * (s[k] - s[k - 1]) / vm : 0.0));
		}
		var tmove = t[n - 1];

		// Sample at 10 Hz: stop, motion, stop. Positions follow a Catmull-Rom spline through the
		// samples, so that the heading does not change in steps at the samples.
		function sample(k, f) {
			var p, h;
			if (f > 0.0) {
				var p1 = pos[k], p2 = pos[k + 1];
				// At the ends, extend the path linearly (reflected point)
				var p0 = k > 0 ? pos[k - 1] : [2.0 * p1[0] - p2[0], 2.0 * p1[1] - p2[1], 2.0 * p1[2] - p2[2]];
				var p3 = k + 2 < n ? pos[k + 2] : [2.0 * p2[0] - p1[0], 2.0 * p2[1] - p1[1], 2.0 * p2[2] - p1[2]];
				var f2 = f * f, f3 = f * f * f;
				p = [0, 1, 2].map(function (j) {
					return 0.5 * (2.0 * p1[j] + (p2[j] - p0[j]) * f + (2.0 * p0[j] - 5.0 * p1[j] + 4.0 * p2[j] - p3[j]) * f2 +
						(3.0 * p1[j] - p0[j] - 3.0 * p2[j] + p3[j]) * f3);
				});
				h = hgt[k] + f * (hgt[k + 1] - hgt[k]);
			} else {
				p = pos[k];
				h = hgt[k];
			}
			var ll = frame.toLatLon(p);
			if (msl && geoid)
				h += geoid.undulation(ll[0], ll[1]);
			return [ll[0], ll[1], h + opts.agl];
		}

		var rows = [];
		var nstart = Math.round(opts.hold_start * RATE);
		var nmove = Math.floor(tmove * RATE) + 1;
		var nend = Math.round(opts.hold_end * RATE);
		var first = sample(0, 0.0);
		for (i = 0; i < nstart; i++)
			rows.push(first);
		k = 0;
		for (i = 0; i < nmove; i++) {
			var ti = i / RATE;
			while (k < n - 2 && t[k + 1] < ti)
				k++;
			// Constant acceleration within the sample interval, so that the speed is continuous
			var dt = t[k + 1] - t[k];
			if (dt <= 0.0) {
				f = 0.0;
			} else {
				var tau = Math.min(Math.max(ti - t[k], 0.0), dt);
				f = (v[k] * tau + 0.5 * (v[k + 1] - v[k]) / dt * tau * tau) / (s[k + 1] - s[k]);
				f = Math.min(Math.max(f, 0.0), 1.0);
			}
			rows.push(sample(k, f));
		}
		var last = sample(n - 1, 0.0);
		for (i = 0; i < nend; i++)
			rows.push(last);

		var out = rows.map(function (r, i) { return [i / RATE, r[0], r[1], r[2]]; });
		var vmx = 0.0;
		for (k = 0; k < n; k++)
			vmx = Math.max(vmx, v[k]);
		return {
			rows: out,
			summary: {
				length_m: s[n - 1], moving_s: tmove, total_s: out.length / RATE,
				max_speed_kmh: vmx * 3.6, mean_speed_kmh: tmove > 0 ? s[n - 1] / tmove * 3.6 : 0.0,
				rows: out.length
			}
		};
	}

	// Output files, same format as route2motion.py
	function toCsv(rows, ecef) {
		return rows.map(function (r) {
			if (ecef) {
				var x = llh2ecef(r[1], r[2], r[3]);
				return r[0].toFixed(1) + ',' + x[0].toFixed(3) + ',' + x[1].toFixed(3) + ',' + x[2].toFixed(3);
			}
			return r[0].toFixed(1) + ',' + r[1].toFixed(9) + ',' + r[2].toFixed(9) + ',' + r[3].toFixed(3);
		}).join('\n') + '\n';
	}

	var api = {
		DEFAULTS: DEFAULTS, LocalFrame: LocalFrame, Geoid: Geoid, llh2ecef: llh2ecef, ecef2llh: ecef2llh,
		buildTrajectory: buildTrajectory, toCsv: toCsv
	};

	if (typeof module !== 'undefined' && module.exports)
		module.exports = api;
	else
		root.RouteCore = api;
})(this);
