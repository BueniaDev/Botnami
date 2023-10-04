/*
    This file is part of Botnami.
    Copyright (C) 2023 BueniaDev.

    Botnami is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Botnami is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Botnami.  If not, see <https://www.gnu.org/licenses/>.
*/

    
#ifndef BOTNAMI_API_H
#define BOTNAMI_API_H

#if defined(_MSC_VER) && !defined(BOTNAMI_STATIC)
    #ifdef BOTNAMI_EXPORTS
        #define BOTNAMI_API __declspec(dllexport)
    #else
        #define BOTNAMI_API __declspec(dllimport)
    #endif // BOTNAMI_EXPORTS
#else
    #define BOTNAMI_API
#endif // _MSC_VER

#endif // BOTNAMI_API_H
