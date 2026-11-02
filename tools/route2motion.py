#!/usr/bin/env python3
"""Convert a route (GPX, KML or GeoJSON) into a 10 Hz user motion file for gnss-sdr-sim.

The route is resampled every metre and smoothed so that corners become curves, the speed
is limited by the requested speed, the lateral acceleration in curves and the longitudinal
acceleration, and the trajectory is sampled at 10 Hz. The output is the "time,lat,lon,height"
CSV read by gnss-sdr-sim -x (ellipsoidal height), or the ECEF CSV read by -u.

Heights come from the route file, from a constant (--height) or from a terrain model
(Open-Meteo elevation API, Copernicus DEM). Heights above mean sea level are converted to
ellipsoidal heights with the EGM96 geoid (tools/geoid_egm96.js).

The algorithm is the same as in route_core.js (used by route_planner.html): both tools give
the same trajectory for the same route.

Examples:
  route2motion.py percorso.gpx -o percorso.csv --speed 50
  route2motion.py percorso.kml -o percorso.csv --speed 5 --height 250
  route2motion.py route.geojson -o route.csv          # file saved by route_planner.html
  gnss-sdr-sim -D -T now -x percorso.csv -a -b 8
"""

import argparse
import base64
import json
import math
import os
import re
import struct
import sys
import urllib.parse
import urllib.request
import xml.etree.ElementTree as ET

# WGS 84
WGS84_A = 6378137.0
WGS84_E2 = 0.0818191908426 ** 2

DEFAULTS = {
    'speed_kmh': 50.0,   # speed when the route does not give one
    'accel': 1.5,        # longitudinal acceleration/deceleration [m/s^2]
    'lat_accel': 2.0,    # lateral acceleration in curves [m/s^2]
    'smooth': 20.0,      # smoothing window [m]
    'hold_start': 60.0,  # stop at the start [s]
    'hold_end': 10.0,    # stop at the end [s]
    'agl': 0.0,          # antenna height above the route heights [m]
}

RESAMPLE_STEP = 1.0      # resampling step [m]
CURVATURE_BASE = 1       # samples on each side for the curvature estimate
CURVE_WINDOW = 2         # samples on each side where a curve speed limit also applies
DEM_STEP = 30.0          # terrain sampling step [m]
RATE = 10                # output rate [Hz]


def iround(x):
    """Round half up, as Math.round() in route_core.js (Python round() rounds half to even)."""
    return int(math.floor(x + 0.5))


# ---------------------------------------------------------------- geodesy

def llh2ecef(lat, lon, h):
    """lat, lon in degrees, h in metres."""
    la, lo = math.radians(lat), math.radians(lon)
    n = WGS84_A / math.sqrt(1.0 - WGS84_E2 * math.sin(la) ** 2)
    return ((n + h) * math.cos(la) * math.cos(lo),
            (n + h) * math.cos(la) * math.sin(lo),
            (n * (1.0 - WGS84_E2) + h) * math.sin(la))


def ecef2llh(x, y, z):
    """Returns lat, lon in degrees and h in metres."""
    lon = math.atan2(y, x)
    p = math.hypot(x, y)
    lat = math.atan2(z, p * (1.0 - WGS84_E2))
    h = 0.0
    for _ in range(10):
        n = WGS84_A / math.sqrt(1.0 - WGS84_E2 * math.sin(lat) ** 2)
        h = p / math.cos(lat) - n
        lat = math.atan2(z, p * (1.0 - WGS84_E2 * n / (n + h)))
    return math.degrees(lat), math.degrees(lon), h


class LocalFrame:
    """East-North-Up frame at a reference point (exact rotation of ECEF, not a projection)."""

    def __init__(self, lat, lon):
        self.o = llh2ecef(lat, lon, 0.0)
        la, lo = math.radians(lat), math.radians(lon)
        sla, cla, slo, clo = math.sin(la), math.cos(la), math.sin(lo), math.cos(lo)
        self.r = ((-slo, clo, 0.0), (-sla * clo, -sla * slo, cla), (cla * clo, cla * slo, sla))

    def to_enu(self, lat, lon):
        x, y, z = llh2ecef(lat, lon, 0.0)
        d = (x - self.o[0], y - self.o[1], z - self.o[2])
        return tuple(sum(self.r[i][j] * d[j] for j in range(3)) for i in range(3))

    def to_latlon(self, e):
        x = [self.o[j] + sum(self.r[i][j] * e[i] for i in range(3)) for j in range(3)]
        lat, lon, _ = ecef2llh(*x)
        return lat, lon


# ---------------------------------------------------------------- geoid

class Geoid:
    """EGM96 geoid undulation from the 1 degree grid of geoid_egm96.js (bilinear interpolation)."""

    def __init__(self, path):
        text = open(path).read()
        hdr = dict((k, float(v)) for k, v in re.findall(r'(lat0|lon0|step|rows|cols):\s*(-?[\d.]+)', text))
        data = ''.join(re.findall(r'"([A-Za-z0-9+/=]+)"', text))
        raw = base64.b64decode(data)
        self.rows, self.cols = int(hdr['rows']), int(hdr['cols'])
        self.lat0, self.lon0, self.step = hdr['lat0'], hdr['lon0'], hdr['step']
        self.v = struct.unpack('<%dh' % (self.rows * self.cols), raw)

    def undulation(self, lat, lon):
        r = (self.lat0 - lat) / self.step
        c = ((lon - self.lon0) % 360.0) / self.step
        r0 = min(max(int(math.floor(r)), 0), self.rows - 2)
        c0 = int(math.floor(c)) % self.cols
        c1 = (c0 + 1) % self.cols
        fr, fc = r - r0, c - math.floor(c)
        v = self.v
        n = (v[r0 * self.cols + c0] * (1 - fr) * (1 - fc) + v[r0 * self.cols + c1] * (1 - fr) * fc +
             v[(r0 + 1) * self.cols + c0] * fr * (1 - fc) + v[(r0 + 1) * self.cols + c1] * fr * fc)
        return n * 0.01


# ---------------------------------------------------------------- route files

def _local(tag):
    return tag.rsplit('}', 1)[-1]


def read_gpx(path):
    root = ET.parse(path).getroot()
    pts = []
    for kind in ('trkpt', 'rtept', 'wpt'):
        for el in root.iter():
            if _local(el.tag) == kind:
                ele = [c for c in el if _local(c.tag) == 'ele']
                h = float(ele[0].text) if ele and ele[0].text else None
                pts.append((float(el.get('lat')), float(el.get('lon')), h))
        if pts:
            break
    return {'points': pts}


def read_kml(path):
    root = ET.parse(path).getroot()
    pts, lines = [], 0
    for el in root.iter():
        tag = _local(el.tag)
        if tag == 'coordinates' and el.text:
            lines += 1
            for tok in el.text.split():
                v = [float(x) for x in tok.split(',')]
                pts.append((v[1], v[0], v[2] if len(v) > 2 else None))
        elif tag == 'coord' and el.text:  # gx:Track
            v = [float(x) for x in el.text.split()]
            pts.append((v[1], v[0], v[2] if len(v) > 2 else None))
    if lines > 1:
        print('WARNING: %d lines in the KML file, joined in file order' % lines, file=sys.stderr)
    return {'points': pts}


def read_geojson(path):
    gj = json.load(open(path))
    feats = gj['features'] if gj.get('type') == 'FeatureCollection' else [gj]
    pts, props = [], {}
    for f in feats:
        g = f.get('geometry', f)
        p = f.get('properties') or {}
        if g['type'] == 'LineString':
            coords = g['coordinates']
            props = p or props
        elif g['type'] == 'MultiLineString':
            coords = [c for line in g['coordinates'] for c in line]
            props = p or props
        elif g['type'] == 'Point':
            coords = [g['coordinates']]
        else:
            continue
        for c in coords:
            pts.append((c[1], c[0], c[2] if len(c) > 2 else None))
    out = {'points': pts}
    if 'speeds_kmh' in props:
        out['speeds_kmh'] = props['speeds_kmh']
    if 'height_ref' in props:
        out['height_ref'] = props['height_ref']
    return out


def read_route(path):
    ext = os.path.splitext(path)[1].lower()
    if ext == '.gpx':
        r = read_gpx(path)
    elif ext == '.kml':
        r = read_kml(path)
    elif ext in ('.geojson', '.json'):
        r = read_geojson(path)
    else:
        sys.exit('ERROR: unknown route format %s (use .gpx, .kml or .geojson)' % ext)
    if len(r['points']) < 2:
        sys.exit('ERROR: the route needs at least two points')
    return r


# ---------------------------------------------------------------- terrain

def dem_heights(latlons, timeout=30):
    """Terrain heights above mean sea level from the Open-Meteo elevation API (100 points per request)."""
    out = []
    for i in range(0, len(latlons), 100):
        chunk = latlons[i:i + 100]
        q = urllib.parse.urlencode({'latitude': ','.join('%.6f' % p[0] for p in chunk),
                                    'longitude': ','.join('%.6f' % p[1] for p in chunk)})
        req = urllib.request.Request('https://api.open-meteo.com/v1/elevation?' + q,
                                     headers={'User-Agent': 'gnss-sdr-sim route2motion'})
        with urllib.request.urlopen(req, timeout=timeout) as resp:
            out += json.load(resp)['elevation']
    return out


# ---------------------------------------------------------------- trajectory

def build_trajectory(points, speeds, opts, geoid=None, dem=None):
    """Same algorithm as buildTrajectory() in route_core.js.

    points: list of (lat, lon, h); h: height of the route (msl or ellipsoidal, see opts['height_ref']),
            None when unknown
    speeds: speed of each segment [m/s] (len(points)-1)
    opts:   accel, lat_accel, smooth, hold_start, hold_end, agl, height_ref ('msl' | 'ellipsoid')
    dem:    function(list of (lat, lon)) -> heights above msl, used when route heights are missing
    Returns a list of (t, lat, lon, h_ellipsoidal) at 10 Hz and a summary dictionary.
    """
    # Drop repeated points
    pts, spd = [points[0]], []
    frame = LocalFrame(points[0][0], points[0][1])
    enu = [frame.to_enu(points[0][0], points[0][1])]
    for i in range(1, len(points)):
        e = frame.to_enu(points[i][0], points[i][1])
        if math.dist(e, enu[-1]) > 0.01:
            pts.append(points[i]); enu.append(e); spd.append(speeds[i - 1])
    if len(pts) < 2:
        raise ValueError('the route has no length')

    # Resample every RESAMPLE_STEP metres along the polyline
    cum = [0.0]
    for i in range(1, len(enu)):
        cum.append(cum[-1] + math.dist(enu[i - 1], enu[i]))
    length = cum[-1]
    n = int(math.ceil(length / RESAMPLE_STEP)) + 1
    pos, hgt, vmax = [], [], []
    seg = 0
    for k in range(n):
        s = length * k / (n - 1)
        while seg < len(enu) - 2 and s > cum[seg + 1]:
            seg += 1
        f = (s - cum[seg]) / (cum[seg + 1] - cum[seg])
        a, b = enu[seg], enu[seg + 1]
        pos.append(tuple(a[j] + f * (b[j] - a[j]) for j in range(3)))
        ha, hb = pts[seg][2], pts[seg + 1][2]
        hgt.append(None if ha is None or hb is None else ha + f * (hb - ha))
        vmax.append(spd[seg])

    # Heights above msl from the terrain model where the route has none
    msl = opts.get('height_ref', 'msl') == 'msl'
    if any(h is None for h in hgt):
        if dem is None:
            raise ValueError('the route has no heights: give a constant height or enable the terrain model')
        idx = list(range(0, n, max(1, iround(DEM_STEP / RESAMPLE_STEP))))
        if idx[-1] != n - 1:
            idx.append(n - 1)
        hd = dem([frame.to_latlon(pos[i]) for i in idx])
        j = 0
        for k in range(n):
            while j < len(idx) - 2 and k > idx[j + 1]:
                j += 1
            f = (k - idx[j]) / (idx[j + 1] - idx[j])
            hgt[k] = hd[j] + f * (hd[j + 1] - hd[j])
        msl = True

    # Smoothing: centred moving average, window shrinking at the ends so that they do not move
    half = iround(opts['smooth'] / RESAMPLE_STEP / 2)
    if half > 0:
        cs = [[0.0] * (n + 1) for _ in range(4)]
        for k in range(n):
            for j in range(3):
                cs[j][k + 1] = cs[j][k] + pos[k][j]
            cs[3][k + 1] = cs[3][k] + hgt[k]
        sp, sh = [], []
        for k in range(n):
            w = min(half, k, n - 1 - k)
            a, b = k - w, k + w + 1
            sp.append(tuple((cs[j][b] - cs[j][a]) / (b - a) for j in range(3)))
            sh.append((cs[3][b] - cs[3][a]) / (b - a))
        pos, hgt = sp, sh

    # Resample the smoothed path at uniform arc length (smoothing brings samples closer in curves)
    s0 = [0.0]
    for k in range(1, n):
        s0.append(s0[-1] + math.hypot(math.dist(pos[k - 1], pos[k]), hgt[k] - hgt[k - 1]))
    n2 = int(math.ceil(s0[-1] / RESAMPLE_STEP)) + 1
    up, uh, uv, s = [], [], [], []
    j = 0
    for k in range(n2):
        sk = s0[-1] * k / (n2 - 1)
        while j < n - 2 and sk > s0[j + 1]:
            j += 1
        f = (sk - s0[j]) / (s0[j + 1] - s0[j]) if s0[j + 1] > s0[j] else 0.0
        up.append(tuple(pos[j][i] + f * (pos[j + 1][i] - pos[j][i]) for i in range(3)))
        uh.append(hgt[j] + f * (hgt[j + 1] - hgt[j]))
        uv.append(vmax[j] if f < 0.5 else vmax[j + 1])
        s.append(sk)
    pos, hgt, vmax, n = up, uh, uv, n2

    # Speed limit in curves: v = sqrt(a_lat / curvature), curvature from the circle through
    # the samples CURVATURE_BASE before and after. The limit also applies to the CURVE_WINDOW
    # neighbouring samples, as the curvature peak may fall between samples.
    m = CURVATURE_BASE
    vcurve = vmax[:]
    for k in range(m, n - m):
        a, b, c = pos[k - m], pos[k], pos[k + m]
        ab, bc, ca = math.dist(a, b), math.dist(b, c), math.dist(c, a)
        u = [b[j] - a[j] for j in range(3)]
        w = [c[j] - a[j] for j in range(3)]
        cross = math.sqrt((u[1] * w[2] - u[2] * w[1]) ** 2 + (u[2] * w[0] - u[0] * w[2]) ** 2 +
                          (u[0] * w[1] - u[1] * w[0]) ** 2)
        if ab * bc * ca > 0.0 and cross > 0.0:
            kappa = 2.0 * cross / (ab * bc * ca)
            vc = math.sqrt(opts['lat_accel'] / kappa)
            for q in range(max(k - CURVE_WINDOW, 0), min(k + CURVE_WINDOW, n - 1) + 1):
                vcurve[q] = min(vcurve[q], vc)
    vmax = vcurve

    # Acceleration limits: start and stop at rest
    v = vmax[:]
    v[0] = 0.0
    v[-1] = 0.0
    for k in range(1, n):
        v[k] = min(v[k], math.sqrt(v[k - 1] ** 2 + 2.0 * opts['accel'] * (s[k] - s[k - 1])))
    for k in range(n - 2, -1, -1):
        v[k] = min(v[k], math.sqrt(v[k + 1] ** 2 + 2.0 * opts['accel'] * (s[k + 1] - s[k])))

    # Time along the path
    t = [0.0]
    for k in range(1, n):
        vm = v[k - 1] + v[k]
        t.append(t[-1] + (2.0 * (s[k] - s[k - 1]) / vm if vm > 0.0 else 0.0))
    tmove = t[-1]

    # Sample at 10 Hz: stop, motion, stop. Positions follow a Catmull-Rom spline through the
    # samples, so that the heading does not change in steps at the samples.
    def sample(k, f):
        if f > 0.0:
            p1, p2 = pos[k], pos[k + 1]
            # At the ends, extend the path linearly (reflected point)
            p0 = pos[k - 1] if k > 0 else tuple(2.0 * p1[j] - p2[j] for j in range(3))
            p3 = pos[k + 2] if k + 2 < n else tuple(2.0 * p2[j] - p1[j] for j in range(3))
            f2, f3 = f * f, f * f * f
            p = tuple(0.5 * (2.0 * p1[j] + (p2[j] - p0[j]) * f + (2.0 * p0[j] - 5.0 * p1[j] + 4.0 * p2[j] - p3[j]) * f2 +
                             (3.0 * p1[j] - p0[j] - 3.0 * p2[j] + p3[j]) * f3) for j in range(3))
        else:
            p = pos[k]
        h = hgt[k] + f * (hgt[k + 1] - hgt[k]) if f > 0.0 else hgt[k]
        lat, lon = frame.to_latlon(p)
        if msl and geoid is not None:
            h += geoid.undulation(lat, lon)
        return lat, lon, h + opts['agl']

    rows = []
    nstart = iround(opts['hold_start'] * RATE)
    nmove = int(math.floor(tmove * RATE)) + 1
    nend = iround(opts['hold_end'] * RATE)
    first = sample(0, 0.0)
    for i in range(nstart):
        rows.append(first)
    k = 0
    for i in range(nmove):
        ti = i / RATE
        while k < n - 2 and t[k + 1] < ti:
            k += 1
        # Constant acceleration within the sample interval, so that the speed is continuous
        dt = t[k + 1] - t[k]
        if dt <= 0.0:
            f = 0.0
        else:
            tau = min(max(ti - t[k], 0.0), dt)
            f = (v[k] * tau + 0.5 * (v[k + 1] - v[k]) / dt * tau * tau) / (s[k + 1] - s[k])
            f = min(max(f, 0.0), 1.0)
        rows.append(sample(k, f))
    last = sample(n - 1, 0.0)
    for i in range(nend):
        rows.append(last)

    out = [(i / RATE,) + r for i, r in enumerate(rows)]
    summary = {'length_m': s[-1], 'moving_s': tmove, 'total_s': len(out) / RATE,
               'max_speed_kmh': max(v) * 3.6, 'mean_speed_kmh': s[-1] / tmove * 3.6 if tmove > 0 else 0.0,
               'rows': len(out)}
    return out, summary


def write_csv(path, rows, ecef=False):
    with open(path, 'w') as f:
        for t, lat, lon, h in rows:
            if ecef:
                x, y, z = llh2ecef(lat, lon, h)
                f.write('%.1f,%.3f,%.3f,%.3f\n' % (t, x, y, z))
            else:
                f.write('%.1f,%.9f,%.9f,%.3f\n' % (t, lat, lon, h))


def write_kml(path, rows):
    step = max(1, len(rows) // 5000)
    coords = ' '.join('%.7f,%.7f,%.1f' % (r[2], r[1], r[3]) for r in rows[::step])
    with open(path, 'w') as f:
        f.write('<?xml version="1.0" encoding="UTF-8"?>\n<kml xmlns="http://www.opengis.net/kml/2.2">'
                '<Document><Placemark><name>gnss-sdr-sim trajectory</name><LineString>'
                '<altitudeMode>absolute</altitudeMode><coordinates>%s</coordinates></LineString>'
                '</Placemark></Document></kml>\n' % coords)


def main():
    d = DEFAULTS
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('route', help='route file: .gpx, .kml or .geojson')
    ap.add_argument('-o', '--output', default='route.csv', help='output CSV (default: route.csv)')
    ap.add_argument('--ecef', action='store_true', help='write ECEF x,y,z for gnss-sdr-sim -u instead of lat,lon,h for -x')
    ap.add_argument('--speed', type=float, help='speed [km/h] (default: speeds of the route file, else %g)' % d['speed_kmh'])
    ap.add_argument('--duration', type=float, help='moving time [s]: scales the speeds to last this long')
    ap.add_argument('--accel', type=float, default=d['accel'], help='acceleration and braking [m/s^2] (default: %(default)s)')
    ap.add_argument('--lat-accel', type=float, default=d['lat_accel'], help='lateral acceleration in curves [m/s^2] (default: %(default)s)')
    ap.add_argument('--smooth', type=float, default=d['smooth'], help='smoothing window [m], 0 = none (default: %(default)s)')
    ap.add_argument('--hold-start', type=float, default=d['hold_start'], help='stop at the start [s] (default: %(default)s)')
    ap.add_argument('--hold-end', type=float, default=d['hold_end'], help='stop at the end [s] (default: %(default)s)')
    ap.add_argument('--height', type=float, help='constant height above mean sea level [m], instead of the route heights')
    ap.add_argument('--dem', action='store_true', help='take the heights from the terrain model even when the route has them')
    ap.add_argument('--no-dem', action='store_true', help='never query the terrain model (needs heights in the route or --height)')
    ap.add_argument('--agl', type=float, default=d['agl'], help='antenna height above ground/route [m] (default: %(default)s)')
    ap.add_argument('--ellipsoidal', action='store_true', help='route heights are ellipsoidal (default: above mean sea level)')
    ap.add_argument('--kml', help='also write the trajectory as KML (e.g. for Google Earth)')
    args = ap.parse_args()

    r = read_route(args.route)
    pts = r['points']
    if args.height is not None:
        pts = [(p[0], p[1], args.height) for p in pts]
    elif args.dem:
        pts = [(p[0], p[1], None) for p in pts]

    nseg = len(pts) - 1
    if args.speed is not None:
        speeds = [args.speed / 3.6] * nseg
    elif 'speeds_kmh' in r and len(r['speeds_kmh']) == nseg:
        speeds = [v / 3.6 for v in r['speeds_kmh']]
    else:
        speeds = [d['speed_kmh'] / 3.6] * nseg
    if min(speeds) <= 0.0:
        sys.exit('ERROR: speeds must be positive')

    opts = {'accel': args.accel, 'lat_accel': args.lat_accel, 'smooth': args.smooth,
            'hold_start': args.hold_start, 'hold_end': args.hold_end, 'agl': args.agl,
            'height_ref': 'ellipsoid' if args.ellipsoidal or r.get('height_ref') == 'ellipsoid' else 'msl'}
    geoid = Geoid(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'geoid_egm96.js'))
    dem = None if args.no_dem else dem_heights

    try:
        rows, sm = build_trajectory(pts, speeds, opts, geoid, dem)
        if args.duration:
            # Scale the speeds so that the moving time matches (a few iterations: curves and
            # acceleration limits do not scale linearly)
            for _ in range(8):
                scale = sm['moving_s'] / args.duration
                speeds = [v * scale for v in speeds]
                rows, sm = build_trajectory(pts, speeds, opts, geoid, dem)
                if abs(sm['moving_s'] - args.duration) < 0.5:
                    break
    except ValueError as e:
        sys.exit('ERROR: %s' % e)
    except OSError as e:
        sys.exit('ERROR: terrain model not reachable (%s); use --height or --no-dem' % e)

    write_csv(args.output, rows, args.ecef)
    if args.kml:
        write_kml(args.kml, rows)

    print('Route: %.2f km, moving %.0f s (mean %.1f km/h, max %.1f km/h), total %.0f s, %d rows -> %s' %
          (sm['length_m'] / 1000, sm['moving_s'], sm['mean_speed_kmh'], sm['max_speed_kmh'],
           sm['total_s'], sm['rows'], args.output), file=sys.stderr)
    print('Example: gnss-sdr-sim -D -T now %s %s -a -b 8' % ('-u' if args.ecef else '-x', args.output), file=sys.stderr)


if __name__ == '__main__':
    main()
