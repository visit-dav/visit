#include "AtomicProperties.h"

#include <visitstream.h>
#include <map>
#include <string>

std::map<std::string, int> elementname_to_atomicnumber;
std::map<std::string, int> residuename_to_number;
std::map<int, std::string> number_to_residuename;
std::map<std::string, int> residuename_to_abbr;
std::map<std::string, std::string> residuename_to_longname;

unsigned char aminocolors[KNOWN_AMINO_ACIDS][3] = {
    {0xBE, 0xA0, 0x6E}, // UNK
    {0xC8, 0xC8, 0xC8}, // ALA
    {0xFF, 0x69, 0xB4}, // ASX
    {0xE6, 0xE6, 0x00}, // CYS
    {0xE6, 0x0A, 0x0A}, // ASP
    {0xE6, 0x0A, 0x0A}, // GLU
    {0x32, 0x32, 0xAA}, // PHE
    {0xEB, 0xEB, 0xEB}, // GLY
    {0x82, 0x82, 0xD2}, // HIS
    {0x0F, 0x82, 0x0F}, // ILE
    {0x14, 0x5A, 0xFF}, // LYS
    {0x0F, 0x82, 0x0F}, // LEU
    {0xE6, 0xE6, 0x00}, // MET
    {0x00, 0xDC, 0xDC}, // ASN
    {0xDC, 0x96, 0x82}, // PRO
    {0x00, 0xDC, 0xDC}, // GLN
    {0x14, 0x5A, 0xFF}, // ARG
    {0xFA, 0x96, 0x00}, // SER
    {0xFA, 0x96, 0x00}, // THR
    {0x0F, 0x82, 0x0F}, // VAL
    {0xB4, 0x5A, 0xB4}, // TRP
    {0x32, 0x32, 0xAA}, // TYR
    {0xFF, 0x69, 0xB4}  // GLX
};


unsigned char shapelycolors[KNOWN_AMINO_ACIDS][3] = {
    {0xFF, 0x00, 0xFF}, // UNK
    {0x8C, 0xFF, 0x8C}, // ALA
    {0xFF, 0x00, 0xFF}, // ASX
    {0xFF, 0xFF, 0x70}, // CYS
    {0xA0, 0x00, 0x42}, // ASP
    {0x66, 0x00, 0x00}, // GLU
    {0x53, 0x4C, 0x52}, // PHE
    {0xFF, 0xFF, 0xFF}, // GLY
    {0x70, 0x70, 0xFF}, // HIS
    {0x00, 0x4C, 0x00}, // ILE
    {0x47, 0x47, 0xB8}, // LYS
    {0x45, 0x5E, 0x45}, // LEU
    {0xB8, 0xA0, 0x42}, // MET
    {0xFF, 0x7C, 0x70}, // ASN
    {0x52, 0x52, 0x52}, // PRO
    {0xFF, 0x4C, 0x4C}, // GLN
    {0x00, 0x00, 0x7C}, // ARG
    {0xFF, 0x70, 0x42}, // SER
    {0xB8, 0x4C, 0x00}, // THR
    {0xFF, 0x8C, 0xFF}, // VAL
    {0x4F, 0x46, 0x00}, // TRP
    {0x8C, 0x70, 0x4C}, // TYR
    {0xFF, 0x00, 0xFF}  // GLX
};

// This is the "shapely" scheme from RasMol. It's the
// same as above, but it doesn't distinguish, for instance,
// ASP and THR (among other pairs).
unsigned char old_shapelycolors[KNOWN_AMINO_ACIDS][3] = {
    {0xFF, 0x00, 0xFF}, // UNK
    {0x8C, 0xFF, 0x8C}, // ALA
    {0xFF, 0x00, 0xFF}, // ASX
    {0xFF, 0xFF, 0x70}, // CYS
    {0xA0, 0x00, 0x42}, // ASP
    {0x66, 0x00, 0x00}, // GLU
    {0x53, 0x4C, 0x42}, // PHE
    {0xFF, 0xFF, 0xFF}, // GLY
    {0x70, 0x70, 0xFF}, // HIS
    {0x00, 0x4C, 0x00}, // ILE
    {0x47, 0x47, 0xB8}, // LYS
    {0x45, 0x5E, 0x45}, // LEU
    {0xB8, 0xA0, 0x42}, // MET
    {0xFF, 0x7C, 0x70}, // ASN
    {0x53, 0x4C, 0x42}, // PRO
    {0xFF, 0x4C, 0x4C}, // GLN
    {0x00, 0x00, 0x7C}, // ARG
    {0xFF, 0x4C, 0x4C}, // SER
    {0xA0, 0x00, 0x42}, // THR
    {0xFF, 0xFF, 0xFF}, // VAL
    {0x53, 0x4C, 0x42}, // TRP
    {0xB8, 0xA0, 0x42}, // TYR
    {0xFF, 0x00, 0xFF}  // GLX
};


unsigned char jmolcolors[MAX_ELEMENT_NUMBER][3] = {
    { 0xFF, 0xFF, 0xFF }, // H
    { 0xD9, 0xFF, 0xFF }, // He
    { 0xCC, 0x80, 0xFF }, // Li
    { 0xC2, 0xFF, 0x00 }, // Be
    { 0xFF, 0xB5, 0xB5 }, // B
    { 0x90, 0x90, 0x90 }, // C
    { 0x30, 0x50, 0xF8 }, // N
    { 0xFF, 0x0D, 0x0D }, // O
    { 0x90, 0xE0, 0x50 }, // F
    { 0xB3, 0xE3, 0xF5 }, // Ne
    { 0xAB, 0x5C, 0xF2 }, // Na
    { 0x8A, 0xFF, 0x00 }, // Mg
    { 0xBF, 0xA6, 0xA6 }, // Al
    { 0xF0, 0xC8, 0xA0 }, // Si
    { 0xFF, 0x80, 0x00 }, // P
    { 0xFF, 0xFF, 0x30 }, // S
    { 0x1F, 0xF0, 0x1F }, // Cl
    { 0x80, 0xD1, 0xE3 }, // Ar
    { 0x8F, 0x40, 0xD4 }, // K
    { 0x3D, 0xFF, 0x00 }, // Ca
    { 0xE6, 0xE6, 0xE6 }, // Sc
    { 0xBF, 0xC2, 0xC7 }, // Ti
    { 0xA6, 0xA6, 0xAB }, // V
    { 0x8A, 0x99, 0xC7 }, // Cr
    { 0x9C, 0x7A, 0xC7 }, // Mn
    { 0xE0, 0x66, 0x33 }, // Fe
    { 0xF0, 0x90, 0xA0 }, // Co
    { 0x50, 0xD0, 0x50 }, // Ni
    { 0xC8, 0x80, 0x33 }, // Cu
    { 0x7D, 0x80, 0xB0 }, // Zn
    { 0xC2, 0x8F, 0x8F }, // Ga
    { 0x66, 0x8F, 0x8F }, // Ge
    { 0xBD, 0x80, 0xE3 }, // As
    { 0xFF, 0xA1, 0x00 }, // Se
    { 0xA6, 0x29, 0x29 }, // Br
    { 0x5C, 0xB8, 0xD1 }, // Kr
    { 0x70, 0x2E, 0xB0 }, // Rb
    { 0x00, 0xFF, 0x00 }, // Sr
    { 0x94, 0xFF, 0xFF }, // Y
    { 0x94, 0xE0, 0xE0 }, // Zr
    { 0x73, 0xC2, 0xC9 }, // Nb
    { 0x54, 0xB5, 0xB5 }, // Mo
    { 0x3B, 0x9E, 0x9E }, // Tc
    { 0x24, 0x8F, 0x8F }, // Ru
    { 0x0A, 0x7D, 0x8C }, // Rh
    { 0x00, 0x69, 0x85 }, // Pd
    { 0xC0, 0xC0, 0xC0 }, // Ag
    { 0xFF, 0xD9, 0x8F }, // Cd
    { 0xA6, 0x75, 0x73 }, // In
    { 0x66, 0x80, 0x80 }, // Sn
    { 0x9E, 0x63, 0xB5 }, // Sb
    { 0xD4, 0x7A, 0x00 }, // Te
    { 0x94, 0x00, 0x94 }, // I
    { 0x42, 0x9E, 0xB0 }, // Xe
    { 0x57, 0x17, 0x8F }, // Cs
    { 0x00, 0xC9, 0x00 }, // Ba
    { 0x70, 0xD4, 0xFF }, // La
    { 0xFF, 0xFF, 0xC7 }, // Ce
    { 0xD9, 0xFF, 0xC7 }, // Pr
    { 0xC7, 0xFF, 0xC7 }, // Nd
    { 0xA3, 0xFF, 0xC7 }, // Pm
    { 0x8F, 0xFF, 0xC7 }, // Sm
    { 0x61, 0xFF, 0xC7 }, // Eu
    { 0x45, 0xFF, 0xC7 }, // Gd
    { 0x30, 0xFF, 0xC7 }, // Tb
    { 0x1F, 0xFF, 0xC7 }, // Dy
    { 0x00, 0xFF, 0x9C }, // Ho
    { 0x00, 0xE6, 0x75 }, // Er
    { 0x00, 0xD4, 0x52 }, // Tm
    { 0x00, 0xBF, 0x38 }, // Yb
    { 0x00, 0xAB, 0x24 }, // Lu
    { 0x4D, 0xC2, 0xFF }, // Hf
    { 0x4D, 0xA6, 0xFF }, // Ta
    { 0x21, 0x94, 0xD6 }, // W
    { 0x26, 0x7D, 0xAB }, // Re
    { 0x26, 0x66, 0x96 }, // Os
    { 0x17, 0x54, 0x87 }, // Ir
    { 0xD0, 0xD0, 0xE0 }, // Pt
    { 0xFF, 0xD1, 0x23 }, // Au
    { 0xB8, 0xB8, 0xD0 }, // Hg
    { 0xA6, 0x54, 0x4D }, // Tl
    { 0x57, 0x59, 0x61 }, // Pb
    { 0x9E, 0x4F, 0xB5 }, // Bi
    { 0xAB, 0x5C, 0x00 }, // Po
    { 0x75, 0x4F, 0x45 }, // At
    { 0x42, 0x82, 0x96 }, // Rn
    { 0x42, 0x00, 0x66 }, // Fr
    { 0x00, 0x7D, 0x00 }, // Ra
    { 0x70, 0xAB, 0xFA }, // Ac
    { 0x00, 0xBA, 0xFF }, // Th
    { 0x00, 0xA1, 0xFF }, // Pa
    { 0x00, 0x8F, 0xFF }, // U
    { 0x00, 0x80, 0xFF }, // Np
    { 0x00, 0x6B, 0xFF }, // Pu
    { 0x54, 0x5C, 0xF2 }, // Am
    { 0x78, 0x5C, 0xE3 }, // Cm
    { 0x8A, 0x4F, 0xE3 }, // Bk
    { 0xA1, 0x36, 0xD4 }, // Cf
    { 0xB3, 0x1F, 0xD4 }, // Es
    { 0xB3, 0x1F, 0xBA }, // Fm
    { 0xB3, 0x0D, 0xA6 }, // Md
    { 0xBD, 0x0D, 0x87 }, // No
    { 0xC7, 0x00, 0x66 }, // Lr
    { 0xCC, 0x00, 0x59 }, // Rf
    { 0xD1, 0x00, 0x4F }, // Db
    { 0xD9, 0x00, 0x45 }, // Sg
    { 0xE0, 0x00, 0x38 }, // Bh
    { 0xE6, 0x00, 0x2E }, // Hs
    { 0xEB, 0x00, 0x26 }  // Mt
};

unsigned char rasmolcolors[MAX_ELEMENT_NUMBER][3] = {
    { 0xFF, 0xFF, 0xFF }, // H
    { 0xFF, 0xC0, 0xCB }, // He
    { 0xB2, 0x22, 0x22 }, // Li
    { 0xFF, 0x14, 0x93 }, // Be
    { 0x00, 0xFF, 0x00 }, // B
    { 0xC8, 0xC8, 0xC8 }, // C
    { 0x8F, 0x8F, 0xFF }, // N
    { 0xF0, 0x00, 0x00 }, // O
    { 0xDA, 0xA5, 0x20 }, // F
    { 0xFF, 0x14, 0x93 }, // Ne
    { 0x00, 0x00, 0xFF }, // Na
    { 0x22, 0x8B, 0x22 }, // Mg
    { 0x80, 0x80, 0x90 }, // Al
    { 0xDA, 0xA5, 0x20 }, // Si
    { 0xFF, 0xA5, 0x00 }, // P
    { 0xFF, 0xC8, 0x32 }, // S
    { 0x00, 0xFF, 0x00 }, // Cl
    { 0xFF, 0x14, 0x93 }, // Ar
    { 0xFF, 0x14, 0x93 }, // K
    { 0x80, 0x80, 0x90 }, // Ca
    { 0xFF, 0x14, 0x93 }, // Sc
    { 0x80, 0x80, 0x90 }, // Ti
    { 0xFF, 0x14, 0x93 }, // V
    { 0x80, 0x80, 0x90 }, // Cr
    { 0x80, 0x80, 0x90 }, // Mn
    { 0xFF, 0xA5, 0x00 }, // Fe
    { 0xFF, 0x14, 0x93 }, // Co
    { 0xA5, 0x2A, 0x2A }, // Ni
    { 0xA5, 0x2A, 0x2A }, // Cu
    { 0xA5, 0x2A, 0x2A }, // Zn
    { 0xFF, 0x14, 0x93 }, // Ga
    { 0xFF, 0x14, 0x93 }, // Ge
    { 0xFF, 0x14, 0x93 }, // As
    { 0xFF, 0x14, 0x93 }, // Se
    { 0xA5, 0x2A, 0x2A }, // Br
    { 0xFF, 0x14, 0x93 }, // Kr
    { 0xFF, 0x14, 0x93 }, // Rb
    { 0xFF, 0x14, 0x93 }, // Sr
    { 0xFF, 0x14, 0x93 }, // Y
    { 0xFF, 0x14, 0x93 }, // Zr
    { 0xFF, 0x14, 0x93 }, // Nb
    { 0xFF, 0x14, 0x93 }, // Mo
    { 0xFF, 0x14, 0x93 }, // Tc
    { 0xFF, 0x14, 0x93 }, // Ru
    { 0xFF, 0x14, 0x93 }, // Rh
    { 0xFF, 0x14, 0x93 }, // Pd
    { 0x80, 0x80, 0x90 }, // Ag
    { 0xFF, 0x14, 0x93 }, // Cd
    { 0xFF, 0x14, 0x93 }, // In
    { 0xFF, 0x14, 0x93 }, // Sn
    { 0xFF, 0x14, 0x93 }, // Sb
    { 0xFF, 0x14, 0x93 }, // Te
    { 0xA0, 0x20, 0xF0 }, // I
    { 0xFF, 0x14, 0x93 }, // Xe
    { 0xFF, 0x14, 0x93 }, // Cs
    { 0xFF, 0xA5, 0x00 }, // Ba
    { 0xFF, 0x14, 0x93 }, // La
    { 0xFF, 0x14, 0x93 }, // Ce
    { 0xFF, 0x14, 0x93 }, // Pr
    { 0xFF, 0x14, 0x93 }, // Nd
    { 0xFF, 0x14, 0x93 }, // Pm
    { 0xFF, 0x14, 0x93 }, // Sm
    { 0xFF, 0x14, 0x93 }, // Eu
    { 0xFF, 0x14, 0x93 }, // Gd
    { 0xFF, 0x14, 0x93 }, // Tb
    { 0xFF, 0x14, 0x93 }, // Dy
    { 0xFF, 0x14, 0x93 }, // Ho
    { 0xFF, 0x14, 0x93 }, // Er
    { 0xFF, 0x14, 0x93 }, // Tm
    { 0xFF, 0x14, 0x93 }, // Yb
    { 0xFF, 0x14, 0x93 }, // Lu
    { 0xFF, 0x14, 0x93 }, // Hf
    { 0xFF, 0x14, 0x93 }, // Ta
    { 0xFF, 0x14, 0x93 }, // W
    { 0xFF, 0x14, 0x93 }, // Re
    { 0xFF, 0x14, 0x93 }, // Os
    { 0xFF, 0x14, 0x93 }, // Ir
    { 0xFF, 0x14, 0x93 }, // Pt
    { 0xDA, 0xA5, 0x20 }, // Au
    { 0xFF, 0x14, 0x93 }, // Hg
    { 0xFF, 0x14, 0x93 }, // Tl
    { 0xFF, 0x14, 0x93 }, // Pb
    { 0xFF, 0x14, 0x93 }, // Bi
    { 0xFF, 0x14, 0x93 }, // Po
    { 0xFF, 0x14, 0x93 }, // At
    { 0xFF, 0x14, 0x93 }, // Rn
    { 0xFF, 0x14, 0x93 }, // Fr
    { 0xFF, 0x14, 0x93 }, // Ra
    { 0xFF, 0x14, 0x93 }, // Ac
    { 0xFF, 0x14, 0x93 }, // Th
    { 0xFF, 0x14, 0x93 }, // Pa
    { 0xFF, 0x14, 0x93 }, // U
    { 0xFF, 0x14, 0x93 }, // Np
    { 0xFF, 0x14, 0x93 }, // Pu
    { 0xFF, 0x14, 0x93 }, // Am
    { 0xFF, 0x14, 0x93 }, // Cm
    { 0xFF, 0x14, 0x93 }, // Bk
    { 0xFF, 0x14, 0x93 }, // Cf
    { 0xFF, 0x14, 0x93 }, // Es
    { 0xFF, 0x14, 0x93 }, // Fm
    { 0xFF, 0x14, 0x93 }, // Md
    { 0xFF, 0x14, 0x93 }, // No
    { 0xFF, 0x14, 0x93 }, // Lr
    { 0xFF, 0x14, 0x93 }, // Rf
    { 0xFF, 0x14, 0x93 }, // Db
    { 0xFF, 0x14, 0x93 }, // Sg
    { 0xFF, 0x14, 0x93 }, // Bh
    { 0xFF, 0x14, 0x93 }, // Hs
    { 0xFF, 0x14, 0x93 }  // Mt
};

float covalent_radius[MAX_ELEMENT_NUMBER] = {
    0.32, // H  1
    0.93, // He 2
    1.23, // Li 3
    0.90, // Be 4
    0.82, // B  5
    0.77, // C  6
    0.75, // N  7
    0.73, // O  8
    0.72, // F  9
    0.71, // Ne 10
    1.54, // Na 11
    1.36, // Mg 12
    1.18, // Al 13
    1.11, // Si 14
    1.06, // P  15
    1.02, // S  16
    0.99, // Cl 17
    0.98, // Ar 18
    2.03, // K  19
    1.74, // Ca 20
    1.44, // Sc 21
    1.32, // Ti 22
    1.22, // V  23
    1.18, // Cr 24
    1.17, // Mn 25
    1.17, // Fe 26
    1.16, // Co 27
    1.15, // Ni 28
    1.17, // Cu 29
    1.25, // Zn 30
    1.26, // Ga 31
    1.22, // Ge 32
    1.20, // As 33
    1.16, // Se 34
    1.14, // Br 35
    1.12, // Kr 36
    2.16, // Rb 37
    1.91, // Sr 38
    1.62, // Y  39
    1.45, // Zr 40
    1.34, // Nb 41
    1.30, // Mo 42
    1.27, // Tc 43
    1.25, // Ru 44
    1.25, // Rh 45
    1.28, // Pd 46
    1.34, // Ag 47
    1.48, // Cd 48
    1.44, // In 49
    1.41, // Sn 50
    1.40, // Sb 51
    1.36, // Te 52
    1.33, // I  53
    1.31, // Xe 54
    2.35, // Cs 55
    1.98, // Ba 56
    1.69, // La 57
    1.65, // Ce 58
    1.65, // Pr 59
    1.64, // Nd 60
    1.63, // Pm 61
    1.62, // Sm 62
    1.85, // Eu 63
    1.61, // Gd 64
    1.59, // Tb 65
    1.59, // Dy 66
    1.58, // Ho 67
    1.57, // Er 68
    1.56, // Tm 69
    1.74, // Yb 70
    1.56, // Lu 71
    1.44, // Hf 72
    1.34, // Ta 73
    1.30, // W  74
    1.28, // Re 75
    1.26, // Os 76
    1.27, // Ir 77
    1.30, // Pt 78
    1.34, // Au 79
    1.49, // Hg 80
    1.48, // Tl 81
    1.47, // Pb 82
    1.46, // Bi 83
    1.46, // Po 84
    1.45, // At 85
    0.1,  //
    0.1,  //
    0.1,  //
    0.1,  //
    1.65, // Th 90
    0.1,  //
    1.42, // U  92
    .1,
    .1,
    .1,
    .1,
    .1,
    .1,
    .1,
    .1,
    .1,
    .1,
    .1,
    .1,
    .1,
    .1,
    .1,
    .1,
    .1
};

float atomic_radius[MAX_ELEMENT_NUMBER] = {
    0.79, // H  1
    0.49, // He 2
    2.05, // Li 3
    1.40, // Be 4
    1.17, // B  5
    0.91, // C  6
    0.75, // N  7
    0.65, // O  8
    0.57, // F  9
    0.51, // Ne 10
    2.23, // Na 11
    1.72, // Mg 12
    1.82, // Al 13
    1.46, // Si 14
    1.23, // P  15
    1.09, // S  16
    0.97, // Cl 17
    0.88, // Ar 18
    2.77, // K  19
    2.23, // Ca 20
    2.09, // Sc 21
    2.00, // Ti 22
    1.92, // V  23
    1.85, // Cr 24
    1.79, // Mn 25
    1.72, // Fe 26
    1.67, // Co 27
    1.62, // Ni 28
    1.57, // Cu 29
    1.53, // Zn 30
    1.81, // Ga 31
    1.52, // Ge 32
    1.33, // As 33
    1.22, // Se 34
    1.12, // Br 35
    1.03, // Kr 36
    2.98, // Rb 37
    2.45, // Sr 38
    2.27, // Y  39
    2.16, // Zr 40
    2.08, // Nb 41
    2.01, // Mo 42
    1.95, // Tc 43
    1.89, // Ru 44
    1.83, // Rh 45
    1.79, // Pd 46
    1.75, // Ag 47
    1.71, // Cd 48
    2.00, // In 49
    1.72, // Sn 50
    1.53, // Sb 51
    1.42, // Te 52
    1.32, // I  53
    1.24, // Xe 54
    3.34, // Cs 55
    2.78, // Ba 56
    2.74, // La 57
    2.70, // Ce 58
    2.67, // Pr 59
    2.64, // Nd 60
    2.62, // Pm 61
    2.59, // Sm 62
    2.56, // Eu 63
    2.54, // Gd 64
    2.51, // Tb 65
    2.49, // Dy 66
    2.47, // Ho 67
    2.45, // Er 68
    2.42, // Tm 69
    2.40, // Yb 70
    2.25, // Lu 71
    2.16, // Hf 72
    2.09, // Ta 73
    2.02, // W  74
    1.97, // Re 75
    1.92, // Os 76
    1.87, // Ir 77
    1.83, // Pt 78
    1.79, // Au 79
    1.76, // Hg 80
    2.08, // Tl 81
    1.81, // Pb 82
    1.63, // Bi 83
    1.53, // Po 84
    1.43, // At 85
    1.34, // Rn 86
    .1,
    .1,
    .1,
    .1,
    .1,
    .1,
    .1,
    .1,
    .1,
    .1,
    .1,
    .1,
    .1,
    .1,
    .1,
    .1,
    .1,
    .1,
    .1,
    .1,
    .1,
    .1,
    .1
};

const char *element_names[MAX_ELEMENT_NUMBER] = {
    "H",  // 1
    "He", // 2
    "Li", // 3
    "Be", // 4
    "B",  // 5
    "C",  // 6
    "N",  // 7
    "O",  // 8
    "F",  // 9
    "Ne", // 10
    "Na", // 11
    "Mg", // 12
    "Al", // 13
    "Si", // 14
    "P",  // 15
    "S",  // 16
    "Cl", // 17
    "Ar", // 18
    "K",  // 19
    "Ca", // 20
    "Sc", // 21
    "Ti", // 22
    "V",  // 23
    "Cr", // 24
    "Mn", // 25
    "Fe", // 26
    "Co", // 27
    "Ni", // 28
    "Cu", // 29
    "Zn", // 30
    "Ga", // 31
    "Ge", // 32
    "As", // 33
    "Se", // 34
    "Br", // 35
    "Kr", // 36
    "Rb", // 37
    "Sr", // 38
    "Y",  // 39
    "Zr", // 40
    "Nb", // 41
    "Mo", // 42
    "Tc", // 43
    "Ru", // 44
    "Rh", // 45
    "Pd", // 46
    "Ag", // 47
    "Cd", // 48
    "In", // 49
    "Sn", // 50
    "Sb", // 51
    "Te", // 52
    "I",  // 53
    "Xe", // 54
    "Cs", // 55
    "Ba", // 56
    "La", // 57
    "Ce", // 58
    "Pr", // 59
    "Nd", // 60
    "Pm", // 61
    "Sm", // 62
    "Eu", // 63
    "Gd", // 64
    "Tb", // 65
    "Dy", // 66
    "Ho", // 67
    "Er", // 68
    "Tm", // 69
    "Yb", // 70
    "Lu", // 71
    "Hf", // 72
    "Ta", // 73
    "W",  // 74
    "Re", // 75
    "Os", // 76
    "Ir", // 77
    "Pt", // 78
    "Au", // 79
    "Hg", // 80
    "Tl", // 81
    "Pb", // 82
    "Bi", // 83
    "Po", // 84
    "At", // 85
    "Rn", // 86
    "Fr", // 87
    "Ra", // 88
    "Ac", // 89
    "Th", // 90
    "Pa", // 91
    "U",  // 92
    "Np", // 93
    "Pu", // 94
    "Am", // 95
    "Cm", // 96
    "Bk", // 97
    "Cf", // 98
    "Es", // 99
    "Fm", // 100
    "Md", // 101
    "No", // 102
    "Lr", // 103
    "Rf", // 104
    "Db", // 105
    "Sg", // 106
    "Bh", // 107
    "Hs", // 108
    "Mt", // 109
};

// These are sorted by the abbreviation to make the map
// tree more balanced.
const char *residue_names[KNOWN_AMINO_ACIDS] = {
    "UNK", //  0   Unknown
    "ALA", //  1   Alanine             A
    "ASX", //  2   ASP/ASN ambiguous   B
    "CYS", //  3   Cysteine            C
    "ASP", //  4   Aspartic acid       D
    "GLU", //  5   Glutamic acid       E
    "PHE", //  6   Phenylalanine       F
    "GLY", //  7   Glycine             G
    "HIS", //  8   Histidine           H
    "ILE", //  9   Isoleucine          I
    "LYS", // 10   Lysine              K
    "LEU", // 11   Leucine             L
    "MET", // 12   Methionine          M
    "ASN", // 13   Asparagine          N
    "PRO", // 14   Proline             P
    "GLN", // 15   Glutamine           Q
    "ARG", // 16   Arginine            R
    "SER", // 17   Serine              S
    "THR", // 18   Threonine           T
    "VAL", // 19   Valine              V
    "TRP", // 20   Tryptophan          W
    "TYR", // 21   Tyrosine            Y
    "GLX"  // 22   GLU/GLN ambiguous   Z
};

const char residue_abbrs[KNOWN_AMINO_ACIDS] = {
    '?', // UNK  Unknown
    'A', // ALA  Alanine
    'B', // ASX  ASP/ASN ambiguous
    'C', // CYS  Cysteine
    'D', // ASP  Aspartic acid
    'E', // GLU  Glutamic acid
    'F', // PHE  Phenylalanine
    'G', // GLY  Glycine
    'H', // HIS  Histidine
    'I', // ILE  Isoleucine
    'K', // LYS  Lysine
    'L', // LEU  Leucine
    'M', // MET  Methionine
    'N', // ASN  Asparagine
    'P', // PRO  Proline
    'Q', // GLN  Glutamine
    'R', // ARG  Arginine
    'S', // SER  Serine
    'T', // THR  Threonine
    'V', // VAL  Valine
    'W', // TRP  Tryptophan
    'Y', // TYR  Tyrosine
    'Z', // GLX  GLU/GLN ambiguous
};

static void
InitializeResidueNameToAbbrMap()
{
    // These are sorted by the abbreviation to make the map
    // tree more balanced.
    residuename_to_abbr["UNK"] = '?'; // Unknown             ?
    residuename_to_abbr["ALA"] = 'A'; // Alanine             A
    residuename_to_abbr["ASX"] = 'B'; // ASP/ASN ambiguous   B
    residuename_to_abbr["CYS"] = 'C'; // Cysteine            C
    residuename_to_abbr["ASP"] = 'D'; // Aspartic acid       D
    residuename_to_abbr["GLU"] = 'E'; // Glutamic acid       E
    residuename_to_abbr["PHE"] = 'F'; // Phenylalanine       F
    residuename_to_abbr["GLY"] = 'G'; // Glycine             G
    residuename_to_abbr["HIS"] = 'H'; // Histidine           H
    residuename_to_abbr["ILE"] = 'I'; // Isoleucine          I
    residuename_to_abbr["LYS"] = 'K'; // Lysine              K
    residuename_to_abbr["LEU"] = 'L'; // Leucine             L
    residuename_to_abbr["MET"] = 'M'; // Methionine          M
    residuename_to_abbr["ASN"] = 'N'; // Asparagine          N
    residuename_to_abbr["PRO"] = 'P'; // Proline             P
    residuename_to_abbr["GLN"] = 'Q'; // Glutamine           Q
    residuename_to_abbr["ARG"] = 'R'; // Arginine            R
    residuename_to_abbr["SER"] = 'S'; // Serine              S
    residuename_to_abbr["THR"] = 'T'; // Threonine           T
    residuename_to_abbr["VAL"] = 'V'; // Valine              V
    residuename_to_abbr["TRP"] = 'W'; // Tryptophan          W
    residuename_to_abbr["TYR"] = 'Y'; // Tyrosine            Y
    residuename_to_abbr["GLX"] = 'Z'; // GLU/GLN ambiguous   Z

    // Add water
    residuename_to_abbr["HOH"] = 'w'; // water
}

static void
InitializeResidueNameToNumberMap()
{
    int i, index = 0;
    for(i = 0; i < KNOWN_AMINO_ACIDS; ++i, ++index)
    {
        residuename_to_number[residue_names[i]] = index;
        number_to_residuename[index] = residue_names[i];
    }
 
    // Add DNA/RNA base pairs
    const char *dnarna_bases[] = {"A", "C", "T", "G", "U"};
    for(i = 0; i < 5; ++i, ++index)
    {
        residuename_to_number[dnarna_bases[i]] = index;
        number_to_residuename[index] = dnarna_bases[i];
    }

    // Add water
    residuename_to_number["HOH"] = index;
    number_to_residuename[index] = "HOH";
}

static void
InitializeResidueNameToLongNameMap()
{
    residuename_to_longname["UNK"] = "Unknown";
    residuename_to_longname["ALA"] = "Alanine";
    residuename_to_longname["ASX"] = "ASP/ASN ambiguous";
    residuename_to_longname["CYS"] = "Cysteine";
    residuename_to_longname["ASP"] = "Aspartic acid";
    residuename_to_longname["GLU"] = "Glutamic acid";
    residuename_to_longname["PHE"] = "Phenylalanine";
    residuename_to_longname["GLY"] = "Glycine";
    residuename_to_longname["HIS"] = "Histidine";
    residuename_to_longname["ILE"] = "Isoleucine";
    residuename_to_longname["LYS"] = "Lysine";
    residuename_to_longname["LEU"] = "Leucine";
    residuename_to_longname["MET"] = "Methionine";
    residuename_to_longname["ASN"] = "Asparagine";
    residuename_to_longname["PRO"] = "Proline";
    residuename_to_longname["GLN"] = "Glutamine";
    residuename_to_longname["ARG"] = "Arginine";
    residuename_to_longname["SER"] = "Serine";
    residuename_to_longname["THR"] = "Threonine";
    residuename_to_longname["VAL"] = "Valine";
    residuename_to_longname["TRP"] = "Tryptophan";
    residuename_to_longname["TYR"] = "Tyrosine";
    residuename_to_longname["GLX"] = "GLU/GLN ambiguous";

    // Add DNA/RNA base pairs
    residuename_to_longname["A"] = "Adenine";
    residuename_to_longname["C"] = "Guanine";
    residuename_to_longname["T"] = "Thymine";
    residuename_to_longname["G"] = "Cytosine";
    residuename_to_longname["U"] = "Uracil";

    // Add water
    residuename_to_longname["HOH"] = "Water";
}

static void
InitializeElementNameToAtomicNumberMap()
{
    // These are sorted by covalent radius to make the map
    // tree more balanced.
    elementname_to_atomicnumber["H"]  = 1;
    elementname_to_atomicnumber["Ne"] = 10;
    elementname_to_atomicnumber["F"]  = 9;
    elementname_to_atomicnumber["O"]  = 8;
    elementname_to_atomicnumber["N"]  = 7;
    elementname_to_atomicnumber["C"]  = 6;
    elementname_to_atomicnumber["B"]  = 5;
    elementname_to_atomicnumber["Be"] = 4;
    elementname_to_atomicnumber["He"] = 2;
    elementname_to_atomicnumber["Ar"] = 18;
    elementname_to_atomicnumber["Cl"] = 17;
    elementname_to_atomicnumber["S"]  = 16;
    elementname_to_atomicnumber["P"]  = 15;
    elementname_to_atomicnumber["Si"] = 14;
    elementname_to_atomicnumber["Kr"] = 36;
    elementname_to_atomicnumber["Br"] = 35;
    elementname_to_atomicnumber["Ni"] = 28;
    elementname_to_atomicnumber["Se"] = 34;
    elementname_to_atomicnumber["Co"] = 27;
    elementname_to_atomicnumber["Cu"] = 29;
    elementname_to_atomicnumber["Fe"] = 26;
    elementname_to_atomicnumber["Mn"] = 25;
    elementname_to_atomicnumber["Al"] = 13;
    elementname_to_atomicnumber["Cr"] = 24;
    elementname_to_atomicnumber["As"] = 33;
    elementname_to_atomicnumber["Ge"] = 32;
    elementname_to_atomicnumber["V"]  = 23;
    elementname_to_atomicnumber["Li"] = 3;
    elementname_to_atomicnumber["Rh"] = 45;
    elementname_to_atomicnumber["Ru"] = 44;
    elementname_to_atomicnumber["Zn"] = 30;
    elementname_to_atomicnumber["Ga"] = 31;
    elementname_to_atomicnumber["Os"] = 76;
    elementname_to_atomicnumber["Ir"] = 77;
    elementname_to_atomicnumber["Tc"] = 43;
    elementname_to_atomicnumber["Re"] = 75;
    elementname_to_atomicnumber["Pd"] = 46;
    elementname_to_atomicnumber["W"]  = 74;
    elementname_to_atomicnumber["Pt"] = 78;
    elementname_to_atomicnumber["Mo"] = 42;
    elementname_to_atomicnumber["Xe"] = 54;
    elementname_to_atomicnumber["Ti"] = 22;
    elementname_to_atomicnumber["I"]  = 53;
    elementname_to_atomicnumber["Ta"] = 73;
    elementname_to_atomicnumber["Nb"] = 41;
    elementname_to_atomicnumber["Ag"] = 47;
    elementname_to_atomicnumber["Au"] = 79;
    elementname_to_atomicnumber["Te"] = 52;
    elementname_to_atomicnumber["Mg"] = 12;
    elementname_to_atomicnumber["Sb"] = 51;
    elementname_to_atomicnumber["Sn"] = 50;
    elementname_to_atomicnumber["U"]  = 92;
    elementname_to_atomicnumber["In"] = 49;
    elementname_to_atomicnumber["Sc"] = 21;
    elementname_to_atomicnumber["Hf"] = 72;
    elementname_to_atomicnumber["Zr"] = 40;
    elementname_to_atomicnumber["At"] = 85;
    elementname_to_atomicnumber["Bi"] = 83;
    elementname_to_atomicnumber["Po"] = 84;
    elementname_to_atomicnumber["Pb"] = 82;
    elementname_to_atomicnumber["Cd"] = 48;
    elementname_to_atomicnumber["Tl"] = 81;
    elementname_to_atomicnumber["Hg"] = 80;
    elementname_to_atomicnumber["Na"] = 11;
    elementname_to_atomicnumber["Tm"] = 69;
    elementname_to_atomicnumber["Lu"] = 71;
    elementname_to_atomicnumber["Er"] = 68;
    elementname_to_atomicnumber["Ho"] = 67;
    elementname_to_atomicnumber["Dy"] = 66;
    elementname_to_atomicnumber["Tb"] = 65;
    elementname_to_atomicnumber["Gd"] = 64;
    elementname_to_atomicnumber["Y"]  = 39;
    elementname_to_atomicnumber["Sm"] = 62;
    elementname_to_atomicnumber["Pm"] = 61;
    elementname_to_atomicnumber["Nd"] = 60;
    elementname_to_atomicnumber["Th"] = 90;
    elementname_to_atomicnumber["Ce"] = 58;
    elementname_to_atomicnumber["Pr"] = 59;
    elementname_to_atomicnumber["La"] = 57;
    elementname_to_atomicnumber["Yb"] = 70;
    elementname_to_atomicnumber["Ca"] = 20;
    elementname_to_atomicnumber["Eu"] = 63;
    elementname_to_atomicnumber["Sr"] = 38;
    elementname_to_atomicnumber["Ba"] = 56;
    elementname_to_atomicnumber["K"]  = 19;
    elementname_to_atomicnumber["Rb"] = 37;
    elementname_to_atomicnumber["Cs"] = 55;
}

static void PrintColorTablesFor_avtColorTables();

static bool maps_initialized = false;
void
InitializeAtomicPropertyMaps()
{
    if (maps_initialized)
        return;

    maps_initialized = true;
        
    InitializeElementNameToAtomicNumberMap();
    InitializeResidueNameToNumberMap();
    InitializeResidueNameToAbbrMap();
    InitializeResidueNameToLongNameMap();

    // PrintColorTablesFor_avtColorTables();
}

int ElementNameToAtomicNumber(const char *element)
{
    char name[3];
    name[0] = element[0];
    name[1] = element[1];
    name[2] = '\0';

    InitializeAtomicPropertyMaps();

    // Make sure the first character is upper case.
    if(name[0] >= 'a' && name[0] <= 'z')
        name[0] -= ('a' - 'A');

    // Make sure the second character is lower case.
    if(name[1] >= 'A' && name[1] <= 'Z')
        name[1] += ('a' - 'A');

    // Look up the value without accidentally inserting bad values
    // into the map.
    std::map<std::string, int>::const_iterator it = 
        elementname_to_atomicnumber.find(name);

    return (it != elementname_to_atomicnumber.end()) ? it->second : -1;
}

int ResiduenameToNumber(const char *name)
{
    InitializeAtomicPropertyMaps();

    // Advance past leading spaces.
    const char *name2 = name;
    while(*name2 == ' ' && *name2 != '\0') ++name2;

    std::map<std::string, int>::const_iterator it = 
        residuename_to_number.find(name2);
    return (it != residuename_to_number.end()) ? it->second : -1;
}

const char *NumberToResiduename(int num)
{
    InitializeAtomicPropertyMaps();

    std::map<int, std::string>::const_iterator it = 
        number_to_residuename.find(num);
    return (it != number_to_residuename.end()) ? it->second.c_str() : 0;
}

int NumberOfKnownResidues()
{
    InitializeAtomicPropertyMaps();

    return residuename_to_number.size();
}

int ResiduenameToAbbr(const char *name)
{
    InitializeAtomicPropertyMaps();

    // Advance past leading spaces.
    const char *name2 = name;
    while(*name2 == ' ' && *name2 != '\0') ++name2;

    std::map<std::string, int>::const_iterator it = 
        residuename_to_abbr.find(name2);
    return (it != residuename_to_abbr.end()) ? it->second : -1;
}

void AddResiduename(const char *name, const char *longname)
{
    InitializeAtomicPropertyMaps();

    // Advance past leading spaces.
    const char *name2 = name;
    while(*name2 == ' ' && *name2 != '\0') ++name2;
    const char *lname2 = longname;
    while(*lname2 == ' ' && *lname2 != '\0') ++lname2;

    std::map<std::string, int>::const_iterator it = 
        residuename_to_number.find(name2);
    if(it == residuename_to_number.end())
    {
        // Look for the largest number in the map.
        int m = 0;
        for(std::map<std::string, int>::const_iterator i =
            residuename_to_number.begin();
            i != residuename_to_number.end(); ++i)
        {
            if(i->second > m)
                m = i->second;
        }

        // Add the new residue name.
        residuename_to_number[name2] = m+1;
        number_to_residuename[m+1] = name2;

        // Store the long name too.
        residuename_to_longname[name2] = lname2;
    }
}

const char *ResiduenameToLongName(const char *name)
{
    InitializeAtomicPropertyMaps();

    std::map<std::string, std::string>::const_iterator it = 
        residuename_to_longname.find(name);
    return (it != residuename_to_longname.end()) ? it->second.c_str() : 0;
}

int ResidueLongnameMaxlen()
{
    InitializeAtomicPropertyMaps();

    int m = 0;
    for(std::map<std::string, std::string>::const_iterator i =
        residuename_to_longname.begin();
        i != residuename_to_longname.end(); ++i)
    {
        if(i->second.size() > m)
            m = i->second.size();
    }

    return m;
}

static void PrintColorTablesFor_avtColorTables()
{
    cout << "static const float ct_shapely_colors[] = {\n";
    for (int i=0; i<KNOWN_AMINO_ACIDS; i++)
    {
        char s[1000];
        float v = float(i)/float(KNOWN_AMINO_ACIDS-1);
        float r = float(shapelycolors[i][0]) / 255.;
        float g = float(shapelycolors[i][1]) / 255.;
        float b = float(shapelycolors[i][2]) / 255.;
        char sv[100];
        sprintf(sv, "%.3f", v);
        strcat(sv,"f,");
        char sr[100];
        sprintf(sr, "%.3f", r);
        strcat(sr,"f,");
        char sg[100];
        sprintf(sg, "%.3f", g);
        strcat(sg,"f,");
        char sb[100];
        sprintf(sb, "%.3f", b);
        strcat(sb,"f,");
        sprintf(s, " %-8s%-8s%-8s%s",sv,sr,sg,sb);
        cout << s<< endl;        
    }
    cout << "};\n";

    cout << "static const float ct_amino_colors[] = {\n";
    for (int i=0; i<KNOWN_AMINO_ACIDS; i++)
    {
        char s[1000];
        float v = float(i)/float(KNOWN_AMINO_ACIDS-1);
        float r = float(aminocolors[i][0]) / 255.;
        float g = float(aminocolors[i][1]) / 255.;
        float b = float(aminocolors[i][2]) / 255.;
        char sv[100];
        sprintf(sv, "%.3f", v);
        strcat(sv,"f,");
        char sr[100];
        sprintf(sr, "%.3f", r);
        strcat(sr,"f,");
        char sg[100];
        sprintf(sg, "%.3f", g);
        strcat(sg,"f,");
        char sb[100];
        sprintf(sb, "%.3f", b);
        strcat(sb,"f,");
        sprintf(s, " %-8s%-8s%-8s%s",sv,sr,sg,sb);
        cout << s<< endl;        
    }
    cout << "};\n";

    cout << "static const float ct_jmol_colors[] = {\n";
    for (int i=0; i<MAX_ELEMENT_NUMBER; i++)
    {
        char s[1000];
        float v = float(i)/float(MAX_ELEMENT_NUMBER-1);
        float r = float(jmolcolors[i][0]) / 255.;
        float g = float(jmolcolors[i][1]) / 255.;
        float b = float(jmolcolors[i][2]) / 255.;
        char sv[100];
        sprintf(sv, "%.3f", v);
        strcat(sv,"f,");
        char sr[100];
        sprintf(sr, "%.3f", r);
        strcat(sr,"f,");
        char sg[100];
        sprintf(sg, "%.3f", g);
        strcat(sg,"f,");
        char sb[100];
        sprintf(sb, "%.3f", b);
        strcat(sb,"f,");
        sprintf(s, " %-8s%-8s%-8s%s",sv,sr,sg,sb);
        cout << s<< endl;        
    }
    cout << "};\n";

    cout << "static const float ct_rasmol_colors[] = {\n";
    for (int i=0; i<MAX_ELEMENT_NUMBER; i++)
    {
        char s[1000];
        float v = float(i)/float(MAX_ELEMENT_NUMBER-1);
        float r = float(rasmolcolors[i][0]) / 255.;
        float g = float(rasmolcolors[i][1]) / 255.;
        float b = float(rasmolcolors[i][2]) / 255.;
        char sv[100];
        sprintf(sv, "%.3f", v);
        strcat(sv,"f,");
        char sr[100];
        sprintf(sr, "%.3f", r);
        strcat(sr,"f,");
        char sg[100];
        sprintf(sg, "%.3f", g);
        strcat(sg,"f,");
        char sb[100];
        sprintf(sb, "%.3f", b);
        strcat(sb,"f,");
        sprintf(s, " %-8s%-8s%-8s%s",sv,sr,sg,sb);
        cout << s<< endl;        
    }
    cout << "};\n";
}

