#!/usr/bin/perl -w

# for most of the TMF filters, the parm vector is not used, and
# fix2.pl assumes as much by putting in an AIR_UNUSED declaration.
# This takes out the AIR_UNUSED for the filters that do actually
# use the parm vector (thanks to do the nearby kernel identification)

while (<>) {
    s|AIR_UNUSED\(parm\); /\* TMF_dn_cn_3ef \*/||g;
    s|AIR_UNUSED\(parm\); /\* TMF_dn_c1_4ef \*/||g;
    s|AIR_UNUSED\(parm\); /\* TMF_d1_cn_2ef \*/||g;
    s|AIR_UNUSED\(parm\); /\* TMF_d1_cn_4ef \*/||g;
    s|AIR_UNUSED\(parm\); /\* TMF_d1_c0_3ef \*/||g;
    s|AIR_UNUSED\(parm\); /\* TMF_d1_c0_4ef \*/||g;
    s|AIR_UNUSED\(parm\); /\* TMF_d2_cn_3ef \*/||g;
    s|AIR_UNUSED\(parm\); /\* TMF_d2_c1_4ef \*/||g;
    print;
}
