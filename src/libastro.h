/*
 * libastro.h
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

#ifndef _LIBASTRO_H_
#define _LIBASTRO_H_

#include <libnova/utility.h>

namespace AstroAir
{
    class LibAstro
    {
        public:
            static void ObservedToJ2000(ln_equ_posn * observed, double jd, ln_equ_posn * J2000pos);
            static void J2000toObserved(ln_equ_posn *J2000pos, double jd, ln_equ_posn * observed);
            
        protected:
            static void ln_get_equ_nut(ln_equ_posn *posn, double jd, bool reverse = false);
    };
    int extractISOTime(const char *timestr, struct ln_date *iso_date);
    double get_local_sidereal_time(double longitude);
    void get_hrz_from_equ(struct ln_equ_posn *object, struct ln_lnlat_posn *observer, double JD, struct ln_hrz_posn *position);
    void get_equ_from_hrz(struct ln_hrz_posn *object, struct ln_lnlat_posn *observer, double JD, struct ln_equ_posn *position);

    extern LibAstro *NOVA;
}

#endif
