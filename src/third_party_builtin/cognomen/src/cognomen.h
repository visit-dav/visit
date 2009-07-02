/*
 *  This file is part of Cognomen.
 *
 *  Cognomen is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Cognomen is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Cognomen.  If not, see <http://www.gnu.org/licenses/>.
 *****
 * Main interface for Cognomen library.
 *****/
#ifndef COGNOMEN_H
#define COGNOMEN_H

#include <cog/bool.h>
#include <cog/types.h>

#if __GNUC__ >= 4 && defined(IN_COGNOMEN)
# define API __attribute__ ((visibility("default")))
#else
# define API /* nothing */
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** Initialization for the Cognomen library.
 * identify must be called by all nodes in lockstep.  It initializes
 * the library and performs node identification.  You only need to do this once
 * per process (run). */
void API cog_identify();

/** Returns the cognomen identifier for this process.  All processes on the
 * same node will receive the same identifier. */
void API cog_me(cog_id * const);

/** Fill a set with the ranks of processes local to the given process. */
void API cog_set_local(cog_set * const, const int);

/** @return the minimum rank defined in the given set */
int API cog_set_min(const cog_set * const);

/** @return the maximum rank defined in the given set */
int API cog_set_max(const cog_set * const);

/** @return true if the given rank falls in the given set. */
bool API cog_set_intersect(const cog_set * const, int);

#ifdef __cplusplus
}
#endif

#endif /* COGNOMEN_H */
