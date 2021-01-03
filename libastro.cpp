/*
 * libastro.cpp
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */
/************************************************* 

Copyright: 2020-2021 Max Qian. All rights reserved

Author:Max Qian

E-mail:astro_air@126.com

Date:2021-1-4

Description:Celestial coordinate transformation

Using:libnova<https://github.com/JohannesBuchner/libnova>

**************************************************/

#include "libastro.h"

#include <math.h>
#include <libnova/precession.h>
#include <libnova/aberration.h>
#include <libnova/nutation.h>

namespace AstroAir
{
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
}
