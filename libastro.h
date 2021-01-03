/*
 * libastro.h
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
}
