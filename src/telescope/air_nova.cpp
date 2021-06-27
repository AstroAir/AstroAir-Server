/*
 * air_nova.cpp
 * 
 * Copyright (C) 2020-2021 Max Qian
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
 
/************************************************* 
 
Copyright: 2020-2021 Max Qian. All rights reserved
 
Author:Max Qian

E-mail:astro_air@126.com
 
Date:2021-6-27
 
Description:Nova Offical Port

**************************************************/

#include "air_nova.h"

namespace AstroAir
{
    double rangeHA(double r)
    {
        double res = r;
        while (res < -12.0)
            res += 24.0;
        while (res >= 12.0)
            res -= 24.0;
        return res;
    }

    double range24(double r)
    {
        double res = r;
        while (res < 0.0)
            res += 24.0;
        while (res > 24.0)
            res -= 24.0;
        return res;
    }

    double range360(double r)
    {
        double res = r;
        while (res < 0.0)
            res += 360.0;
        while (res > 360.0)
            res -= 360.0;
        return res;
    }

    double rangeDec(double decdegrees)
    {
        if ((decdegrees >= 270.0) && (decdegrees <= 360.0))
            return (decdegrees - 360.0);
        if ((decdegrees >= 180.0) && (decdegrees < 270.0))
            return (180.0 - decdegrees);
        if ((decdegrees >= 90.0) && (decdegrees < 180.0))
            return (180.0 - decdegrees);
        return decdegrees;
    }

    double get_local_hour_angle(double sideral_time, double ra)
    {
        double HA = sideral_time - ra;
        return rangeHA(HA);
    }

    void get_alt_az_coordinates(double Ha, double Dec, double Lat, double* Alt, double *Az)
    {
        double alt, az;
        Ha *= M_PI / 180.0;
        Dec *= M_PI / 180.0;
        Lat *= M_PI / 180.0;
        alt = asin(sin(Dec) * sin(Lat) + cos(Dec) * cos(Lat) * cos(Ha));
        az = acos((sin(Dec) - sin(alt)*sin(Lat)) / (cos(alt) * cos(Lat)));
        alt *= 180.0 / M_PI;
        az *= 180.0 / M_PI;
        if (sin(Ha) >= 0.0)
            az = 360 - az;
        *Alt = alt;
        *Az = az;
    }

    double estimate_geocentric_elevation(double Lat, double El)
    {
        Lat *= M_PI / 180.0;
        Lat = sin(Lat);
        El += Lat * (EARTHRADIUSPOLAR - EARTHRADIUSEQUATORIAL);
        return El;
    }

    double estimate_field_rotation_rate(double Alt, double Az, double Lat)
    {
        Alt *= M_PI / 180.0;
        Az *= M_PI / 180.0;
        Lat *= M_PI / 180.0;
        double ret = cos(Lat) * cos(Az) / cos(Alt);
        ret *= 180.0 / M_PI;
        return ret;
    }

    double estimate_field_rotation(double HA, double rate)
    {
        HA *= rate;
        while(HA >= 360.0)
            HA -= 360.0;
        while(HA < 0)
            HA += 360.0;
        return HA;
    }

    double as2rad(double as)
    {
        return as * M_PI / (60.0*60.0*12.0);
    }

    double rad2as(double rad)
    {
        return rad * (60.0*60.0*12.0) / M_PI;
    }

    double estimate_distance(double parsecs, double parallax_radius)
    {
        return parallax_radius / sin(as2rad(parsecs));
    }

    double m2au(double m)
    {
        return m / ASTRONOMICALUNIT;
    }

    double calc_delta_magnitude(double mag_ratio, double *spectrum, double *ref_spectrum, int spectrum_size)
    {
        double delta_mag = 0;
        for(int l = 0; l < spectrum_size; l++) {
            delta_mag += spectrum[l] * mag_ratio * ref_spectrum[l] / spectrum[l];
        }
        delta_mag /= spectrum_size;
        return delta_mag;
    }

    double calc_photon_flux(double rel_magnitude, double filter_bandwidth, double wavelength, double steradian)
    {
        return pow(10, rel_magnitude*-0.4)*(LUMEN(wavelength)*(steradian/(M_PI*4))*filter_bandwidth);
    }

    double calc_rel_magnitude(double photon_flux, double filter_bandwidth, double wavelength, double steradian)
    {
        return log10(photon_flux/(LUMEN(wavelength)*(steradian/(M_PI*4))*filter_bandwidth))/-0.4;
    }

    double estimate_absolute_magnitude(double delta_dist, double delta_mag)
    {
        return sqrt(delta_dist) * delta_mag;
    }

    void baseline_2d_projection(double alt, double az, double baseline[3], double wavelength, double uvresult[2])
    {
        az *= M_PI / 180.0;
        alt *= M_PI / 180.0;
        uvresult[0] = (baseline[0] * sin(az) + baseline[1] * cos(az));
        uvresult[1] = (baseline[1] * sin(alt) * sin(az) - baseline[0] * sin(alt) * cos(az) + baseline[2] * cos(alt));
        uvresult[0] *= AIRY / wavelength;
        uvresult[1] *= AIRY / wavelength;
    }

    double baseline_delay(double alt, double az, double baseline[3])
    {
        az *= M_PI / 180.0;
        alt *= M_PI / 180.0;
        return cos(az) * baseline[1] * cos(alt) - baseline[0] * sin(az) * cos(alt) + sin(alt) * baseline[2];
    }
}