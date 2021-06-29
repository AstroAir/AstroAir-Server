/*
 * air_nova.h
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

#ifndef _AIR_NOVA_H_
#define _AIR_NOVA_H_

#include <math.h>

#define J2000       2451545.0
#define ERRMSG_SIZE 1024

#define STELLAR_DAY        86164.098903691
#define TRACKRATE_SIDEREAL ((360.0 * 3600.0) / STELLAR_DAY)
#define SOLAR_DAY          86400
#define TRACKRATE_SOLAR    ((360.0 * 3600.0) / SOLAR_DAY)
#define TRACKRATE_LUNAR    14.511415
#define EARTHRADIUSEQUATORIAL 6378137.0
#define EARTHRADIUSPOLAR 6356752.0
#define EARTHRADIUSMEAN 6372797.0
#define SUNMASS 1.98847E+30
#define PLANK_H 6.62607015E-34
#define DIRAC_H (PLANK_H/(2*M_PI))
#define EINSTEIN_G 6.67408E-11
#define EULER 2.71828182845904523536028747135266249775724709369995
#define ROOT2 1.41421356237309504880168872420969807856967187537694
#define AIRY 1.21966
#define CIRCLE_DEG 360
#define CIRCLE_AM (CIRCLE_DEG * 60)
#define CIRCLE_AS (CIRCLE_AM * 60)
#define RAD_AS (CIRCLE_AS/(M_PI*2))
#define ASTRONOMICALUNIT 1.495978707E+11
#define PARSEC (ASTRONOMICALUNIT*RAD_AS)
#define LIGHTSPEED 299792458.0
#define JULIAN_LY (LIGHTSPEED * SOLAR_DAY * 365)
#define STELLAR_LY (LIGHTSPEED * STELLAR_DAY * 365)
#define FLUX(wavelength) (wavelength/(PLANK_H*LIGHTSPEED))
#define CANDLE ((1.0/683.0)*FLUX(555))
#define LUMEN(wavelength) (CANDLE/(4*M_PI)*pow((FLUX(wavelength)/FLUX(555)), 0.25))
#define REDSHIFT(wavelength, reference) (1.0-(reference/wavelength))
#define DOPPLER(shift, speed) (speed*shift)

namespace AstroAir
{

    int fs_sexa(char *out, double a, int w, int fracbase);
    
    /**
     * @brief rangeHA Limits the hour angle value to be between -12 ---> 12
     * @param r current hour angle value
     * @return Limited value (-12,12)
     */
    double rangeHA(double r);

    /**
     * @brief range24 Limits a number to be between 0-24 range.
     * @param r number to be limited
     * @return Limited number
     */
    double range24(double r);

    /**
     * @brief range360 Limits an angle to be between 0-360 degrees.
     * @param r angle
     * @return Limited angle
     */
    double range360(double r);

    /**
     * @brief rangeDec Limits declination value to be in -90 to 90 range.
     * @param r declination angle
     * @return limited declination
     */
    double rangeDec(double r);

    /**
     * @brief get_local_sidereal_time Returns local sideral time given longitude and system clock.
     * @param longitude Longitude in INDI format (0 to 360) increasing eastward.
     * @return Local Sidereal Time.
     */
    double get_local_sidereal_time(double longitude);

    /**
     * @brief get_local_hour_angle Returns local hour angle of an object
     * @param local_sideral_time Local Sideral Time
     * @param ra RA of object
     * @return Hour angle in hours (-12 to 12)
     */
    double get_local_hour_angle(double local_sideral_time, double ra);


    /**
     * @brief get_hrz_from_equ Calculate horizontal coordinates from equatorial coordinates.
     * @param object Equatorial Object Coordinates
     * @param observer Observer Location
     * @param JD Julian Date
     * @param position Calculated Horizontal Coordinates.
     * @note Use this instead of libnova ln_get_hrz_from_equ since it corrects libnova Azimuth (0 = North and not South).
     */
    void get_hrz_from_equ(struct ln_equ_posn *object, struct ln_lnlat_posn *observer, double JD, struct ln_hrz_posn *position);

    /**
     * @brief ln_get_equ_from_hrz Calculate Equatorial EOD Coordinates from horizontal coordinates
     * @param object Horizontal Object Coordinates
     * @param observer Observer Location
     * @param JD Julian Date
     * @param position Calculated Equatorial Coordinates.
     * @note Use this instead of libnova ln_get_equ_from_hrz since it corrects libnova Azimuth (0 = North and not South).
     */
    void get_equ_from_hrz(struct ln_hrz_posn *object, struct ln_lnlat_posn *observer, double JD,
                        struct ln_equ_posn *position);
    /**
     * @brief get_alt_az_coordinates Returns alt-azimuth coordinates of an object
     * @param hour_angle Hour angle in hours (-12 to 12)
     * @param dec DEC of object
     * @param latitude latitude in INDI format (-90 to +90)
     * @param alt ALT of object will be returned here
     * @param az AZ of object will be returned here
     */
    void get_alt_az_coordinates(double hour_angle, double dec, double latitude, double* alt, double *az);

    /**
     * @brief estimate_geocentric_elevation Returns an estimation of the actual geocentric elevation
     * @param latitude latitude in INDI format (-90 to +90)
     * @param sea_level_elevation sea level elevation
     * @return Aproximated geocentric elevation
     */
    double estimate_geocentric_elevation(double latitude, double sea_level_elevation);

    /**
     * @brief estimate_field_rotation_rate Returns an estimation of the field rotation rate of the object
     * @param Alt altitude coordinate of the object
     * @param Az azimuth coordinate of the object
     * @param latitude latitude in INDI format (-90 to +90)
     * @return Aproximation of the field rotation rate
     */
    double estimate_field_rotation_rate(double Alt, double Az, double latitude);

    /**
     * @brief estimate_field_rotation Returns an estimation of the field rotation rate of the object
     * @param hour_angle Hour angle in hours (-12 to 12)
     * @param field_rotation_rate the field rotation rate
     * @return Aproximation of the absolute field rotation
     */
    double estimate_field_rotation(double hour_angle, double field_rotation_rate);

    /**
     * @brief as2rad Convert arcseconds into radians
     * @param as the arcseconds to convert
     * @return radians corresponding as angle value
     */
    double as2rad(double as);

    /**
     * @brief rad2as Convert radians into arcseconds
     * @param as the radians to convert
     * @return arcseconds corresponding as angle value
     */
    double rad2as(double rad);

    /**
     * @brief estimate_distance Convert parallax arcseconds into meters
     * @param parsec the parallax arcseconds to convert
     * @return Estimation of the distance in measure units used in parallax_radius
     */
    double estimate_distance(double parsecs, double parallax_radius);

    /**
     * @brief m2au Convert meters into astronomical units
     * @param m the distance in meters to convert
     * @return Estimation of the distance in astronomical units
     */
    double m2au(double m);

    /**
     * @brief calc_delta_magnitude Returns the difference of magnitudes given two spectra
     * @param mag_ratio Reference magnitude
     * @param spectrum The spectrum of the star under exam
     * @param ref_spectrum The spectrum of the reference star
     * @param spectrum_size The size of the spectrum array in elements
     * @return the magnitude difference
     */
    double calc_delta_magnitude(double mag_ratio, double *spectrum, double *ref_spectrum, int spectrum_size);

    /**
     * @brief calc_photon_flux Returns the photon flux of the object with the given magnitude observed at a determined wavelenght using a passband filter through a steradian expressed cone
     * @param rel_magnitude Relative magnitude of the object observed
     * @param filter_bandwidth Filter bandwidth in meters
     * @param wavelength Wavelength in meters
     * @param steradian The light cone in steradians
     * @return the photon flux in Lumen
     */
    double calc_photon_flux(double rel_magnitude, double filter_bandwidth, double wavelength, double steradian);

    /**
     * @brief calc_rel_magnitude Returns the relative magnitude of the object with the given photon flux measured at a determined wavelenght using a passband filter over an incident surface
     * @param photon_flux The photon flux in Lumen
     * @param filter_bandwidth Filter bandwidth in meters
     * @param wavelength Wavelength in meters
     * @param incident_surface The incident surface in square meters
     * @return the relative magnitude of the object observed
     */
    double calc_rel_magnitude(double photon_flux, double filter_bandwidth, double wavelength, double steradian);

    /**
     * @brief estimate_absolute_magnitude Returns an estimation of the absolute magnitude of an object given its distance and the difference of its magnitude with a reference object
     * @param dist The distance in parallax radiuses
     * @param delta_mag The difference of magnitudes
     * @return Aproximation of the absolute magnitude in Δmag
     */
    double estimate_absolute_magnitude(double dist, double delta_mag);

    /**
     * @brief estimate the star mass in ref_size units e.g. sun masses or kgs
     * @param delta_mag The absolute magnitude ratio between the reference object used as unit in ref_size.
     * @param ref_mass The mass of the reference object used to calculate the magnitude difference.
     */
    double estimate_star_mass(double delta_mag, double ref_mass);

    /**
     * @brief estimate the orbit radius of an object with known mass orbiting around a star.
     * @param obs_lambda The observed wavelength of a spectral line observed on the star affected by redshift or blueshift.
     * @param ref_lambda The reference wavelength of the spectral line observed on earth or the nullshift spectral line position.
     * @param period The orbital period.
     */
    double estimate_orbit_radius(double obs_lambda, double ref_lambda, double period);

    /**
     * @brief estimate the mass of an object with known mass orbiting around a star.
     * @param star_mass The mass of the star hosting an orbiting object.
     * @param star_drift The star lagrange point L1 (observed drift of the star).
     * @param orbit_radius The estimated orbit radius of the companion object (star, planet, cloud).
     */
    double estimate_secondary_mass(double star_mass, double star_drift, double orbit_radius);

    /**
     * @brief estimate the size of an object occulting a star in star_size units.
     * @param star_size The size of the occulted star.
     * @param dropoff_ratio The light curve dropoff during the transit. Linear scale. 0.0=no dropoff 1.0=totally occulted.
     */
    double estimate_secondary_size(double star_size, double dropoff_ratio);

    /**
     * @brief baseline_2d_projection Returns the coordinates of the projection of a single baseline targeting the object by coordinates
     * @param alt current altitude of the target.
     * @param az azimuth position of the target.
     * @param baseline the baseline in meters. Three-dimensional xyz north is z axis y is UTC0 x is UTC0+90°.
     * @param wavelength The observing electromagnetic wavelength, the lower the size increases.
     * @param uvresult result plane coordinates of the current projection given the baseline and target vector.
     */
    void baseline_2d_projection(double alt, double az, double baseline[3], double wavelength, double uvresult[2]);

    /**
     * @brief baseline_delay Returns the delay in meters of a single baseline targeting the object by coordinates
     * @param alt current altitude of the target.
     * @param az azimuth position of the target.
     * @param baseline the baseline in meters. Three-dimensional xyz north is z axis y is UTC0 x is UTC0+90°.
     * @param wavelength The observing electromagnetic wavelength, the lower the size increases.
     * @return double[2] UV plane coordinates of the current projection given the baseline and target vector.
     */
}

#endif