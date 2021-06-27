/*
 * libastro.cpp
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

Date:2021-2-15

Description:Celestial coordinate transformation

Using:libnova<https://github.com/JohannesBuchner/libnova>

**************************************************/

#include "libastro.h"
#include "telescope/air_nova.h"

#include <math.h>
#include <libnova/precession.h>
#include <libnova/aberration.h>
#include <libnova/nutation.h>
#include <libnova/julian_day.h>
#include <libnova/sidereal_time.h>
#include <libnova/ln_types.h>
#include <libnova/transform.h>

namespace AstroAir
{
    LibAstro *NOVA;

    void LibAstro::ObservedToJ2000(ln_equ_posn * observed, double jd, ln_equ_posn * J2000pos)
    {
        ln_equ_posn tempPos;
        ln_get_equ_aber(observed, jd, &tempPos);
        tempPos.ra = observed->ra - (tempPos.ra - observed->ra);
        tempPos.dec = observed->dec * 2 - tempPos.dec;
        ln_get_equ_nut(&tempPos, jd, true);
        ln_get_equ_prec2(&tempPos, jd, JD2000, J2000pos);
    }
    
    void LibAstro::J2000toObserved(ln_equ_posn *J2000pos, double jd, ln_equ_posn * observed)
    {
        ln_equ_posn tempPosn;
        ln_get_equ_prec2(J2000pos, JD2000, jd, &tempPosn);
        ln_get_equ_nut(&tempPosn, jd, false);
        ln_get_equ_aber(&tempPosn, jd, observed);
    }

    void LibAstro::ln_get_equ_nut(ln_equ_posn *posn, double jd, bool reverse)
    {
        struct ln_nutation nut;
        ln_get_nutation (jd, &nut);
        double mean_ra, mean_dec, delta_ra, delta_dec;
        mean_ra = ln_deg_to_rad(posn->ra);
        mean_dec = ln_deg_to_rad(posn->dec);
        double nut_ecliptic = ln_deg_to_rad(nut.ecliptic + nut.obliquity);
        double sin_ecliptic = sin(nut_ecliptic);
        double sin_ra = sin(mean_ra);
        double cos_ra = cos(mean_ra);
        double tan_dec = tan(mean_dec);
        delta_ra = (cos (nut_ecliptic) + sin_ecliptic * sin_ra * tan_dec) * nut.longitude - cos_ra * tan_dec * nut.obliquity;
        delta_dec = (sin_ecliptic * cos_ra) * nut.longitude + sin_ra * nut.obliquity;
        if (reverse)
        {
            delta_ra = -delta_ra;
            delta_dec = -delta_dec;
        }
        posn->ra += delta_ra;
        posn->dec += delta_dec;
    }

    int extractISOTime(const char *timestr, struct ln_date *iso_date)
    {
        struct tm utm;
        if (strptime(timestr, "%Y/%m/%dT%H:%M:%S", &utm))
        {
            ln_get_date_from_tm(&utm, iso_date);
            return (0);
        }
        if (strptime(timestr, "%Y-%m-%dT%H:%M:%S", &utm))
        {
            ln_get_date_from_tm(&utm, iso_date);
            return (0);
        }
        return (-1);
    }

    double get_local_sidereal_time(double longitude)
    {
        double SD = ln_get_apparent_sidereal_time(ln_get_julian_from_sys()) - (360.0 - longitude) / 15.0;
        return range24(SD);
    }

    void get_hrz_from_equ(struct ln_equ_posn *object, struct ln_lnlat_posn *observer, double JD, struct ln_hrz_posn *position)
    {
        ln_get_hrz_from_equ(object, observer, JD, position);
        position->az -= 180;
        if (position->az < 0)
            position->az += 360;
    }

    void get_equ_from_hrz(struct ln_hrz_posn *object, struct ln_lnlat_posn *observer, double JD, struct ln_equ_posn *position)
    {
        struct ln_hrz_posn libnova_object;
        libnova_object.az = object->az + 180;
        if (libnova_object.az > 360)
            libnova_object.az -= 360;
        libnova_object.alt = object->alt;
        ln_get_equ_from_hrz(&libnova_object, observer, JD, position);
    }
}
