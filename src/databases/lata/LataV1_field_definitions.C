/****************************************************************************
* Copyright (c) 2022, CEA
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
* 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*****************************************************************************/

#include <LataV1_field_definitions.h>
#include <Motcle.h>

typedef struct
{
  const char * name;
  int shape; // Vector size (-1 => dimension of the problem)
} StdComponents;

// COMPOSANTES EN MAJUSCULES !!!!!
// Components are checked in the same order than this array.
// We assume that the component has been found if component
// name begins with the string in this array. For example
// if the lata file contains INDICATRICE_INTERF, we will
// find that it is an "INDICATRICE" component.
// Therefore, long names must be placed before short names:
// If we have a component "K" and a component "K_EPS", then
// "K_EPS" must be placed before "K", otherwise "K_EPS" will
// never be found.
const StdComponents std_components[] =
{
  { "VITESSE",             -1 },
  { "primal",              -1 },
  { "VORTICITE",           -2 },
  { "MOYENNE_VITESSE",     -1 },
  { "ECART_TYPE_VITESSE",  -1 },
  { "MOYENNE_VORTICITE",   -2 },
  { "ECART_TYPE_VORTICITE", -2 },
  { "GRADIENT_PRESSION",            -1 },
  { "DERIVEE_U_ETOILE",             -1 },
  { "TERME_DIFFUSION_VITESSE",      -1 },
  { "TERME_CONVECTION_VITESSE",     -1 },
  { "TERME_SOURCE_VITESSE",         -1 },
  { "GRAD",         -1 },
  { "NORMALE_INTERFACE",            -1 },
  { "K_EPS",                             2 },
  { "ACCELERATION",        -1 },
  { "CHAMP_VECTORIEL",     -1},
  { "2_",     2},
  { "3_",     3},
  { "6_",     6},
  { "9_",     9},
  { "",                  1 }
  // Empty label means end of the table
};

int latav1_component_shape(const Motcle& compo)
{
  int i = 0;
  while (std_components[i].name[0] != 0)
    {
      if (compo.debute_par(std_components[i].name))
        return std_components[i].shape;
      i++;
    }
  return 1;
}
