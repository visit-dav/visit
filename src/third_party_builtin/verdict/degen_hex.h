/* --------------------------------------------------------------------------------
    Purpose: Logic to handle degenerate hexahedra

    Mark C. Miller, Wed Apr 16 10:40:20 PDT 2025

    Handle cases where a HEX entity (8 nodes) is used to store a tet, pyramid or
    wedge by duplicating nodes. This is quite common in engineering codes.

    Degeneracy logic:

    --------------------+----------------------------------------------------------
        Ordinary Hex    |                Degenerate Hex Cases
    --------------------+------------------+-----------------+---------------------
        VTK_HEXAHEDRON  |    VTK_WEDGE     |    VTK_PYRAMID  |     VTK_TETRA
                        |   3==2, 7==6     |    7==6==5==4   |  3==2,7==6==5==4
                        |                  |                 | 
           7--------6   |          .- 7|6  |      4|5|6|7    |        4|5|6|7
          /|       /|   |       .-    /|   |        /\       |          /\
         / |      / |   |    .-      / |   |       /.|\      |         / |\
        4--------5  |   |  4--------5  |   |      /. | \     |        /  | \
        |  |     |  |   |  |        |  |   |     /.  |  \    |       /   |  \
        |  3-----|--2   |  |       .- 3|2  |    3.----|--2   |      /     |.3|2
        | /      | /    |  |    .-  | /    |   /.     | /    |     /   .- | /
        |/       |/     |  | .-     |/     |  /.      |/     |    / .-    |/
        0--------1      |  0--------1      | 0--------1      |   0--------1
                        |                  |                 |
    --------------------+------------------+-----------------+---------------------
    Duplication patterns|    01223455      |    01234444     |      01224444
    --------------------+------------------+-----------------+---------------------

    Let c(i) be coordinate of node local index i, T/F = true/false result of test
    Test c(7)==c(6)
      F: VTK_HEXAHEDRON, Done (1 compare, common case)
      T: Could be a degenerate case
        Test c(6)==c(5)
          F: Could be degen Wedge or FUNKY
            Test c(3)==c(2)
              F: FUNKY but proceed as hex, done (3 compares)
              T: Degen Wedge, done (3 compares)
          T: Could be degen Tet, Pyramid or FUNKY
            Test c(5)==c(4)
              F: FUNKY but proceed as hex, done (3 compares)
              T: Degen Tet or Pyramid
                Test c(3)==c(2)
                  T: Degen Tet, done (4 compares)
                  F: Degen Pyramid, done (4 compares)

   There are many ways to construct degenerate tets, pyramids and wedges in a hex.
   Those supported here are constructed, reasonably, by collapsing the highest numbered
   nodes on a face in the original hex. This places duplicate nodes at the later set
   of 8 nodes in a hex. So, we process them in reverse order to maximize likelihood of
   early resolution.

   The verdict library has several metrics defined for hex and tet elements but only 
   the volume metric for pyramid and wedge elements.

   Mark C. Miller, Wed Apr 16 10:42:08 PDT 2025
   -------------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------------
   Function: equal_coords
   Purpose: Determine if two coordinates are equal

   Mark C. Miller, Wed Apr 16 10:42:08 PDT 2025
   -------------------------------------------------------------------------------- */
static int equal_coords(VERDICT_REAL const c1[3], VERDICT_REAL const c2[3])
{
    return c1[0] == c2[0] && c1[1] == c2[1] && c1[2] == c2[2];
}

/* --------------------------------------------------------------------------------
   Function: degen_case
   Purpose: Detect degeneracy cases with minimal performance impact.

   Implement the logic described above to use minimum work to detect the cases.
   When degenerate cases are detected, we use the uniq argument returned to
   caller to indicate which nodes it should use from the original, hex, coordinates.
   Note that we don't do an *exhaustive* detection here...that is, ensuring the 
   *entire* duplication pattern is observed. We use the *minimum* number of
   coordinate comparisons to maintain decent performance and arrive at reasonable
   conclusions.

   Mark C. Miller, Wed Apr 16 10:42:08 PDT 2025
   -------------------------------------------------------------------------------- */
static int degen_case(VERDICT_REAL const c[][3], int uniq[])
{
    if (!equal_coords(c[7],c[6]))
        return 8; /* ordinary hex, common case */

    if (equal_coords(c[6],c[5]))
    {
        /* Could be a degen pyramid or tet or something FUNKY */
        if (!equal_coords(c[5],c[4]))
            return 8; /* FUNKY, but treat as hex */

        /* Could still be a degen pyramid or tet */
        if (equal_coords(c[3],c[2]))
        {
            /* Use entries 0,1,2 and 4 */
            uniq[0] = 0; uniq[1] = 1; uniq[2] = 2; uniq[3] = 4;
            return 4; /* Degen tet */
        }
        else
        {
            /* Use entries 0,1,2,3 and 4 */
            uniq[0] = 0; uniq[1] = 1; uniq[2] = 2; uniq[3] = 3;
            uniq[4] = 4;
            return 5; /* Degen pyramid */
        }
    }
    else
    {
        /* Could be a degen wedge or something FUNKY */
        if (equal_coords(c[3],c[2]))
        {
            /* Use entries 0,1,2, and 4,5,6 */
            uniq[0] = 0; uniq[1] = 1; uniq[2] = 2;
            uniq[3] = 4; uniq[4] = 5; uniq[5] = 6;
            return 6; /* Degen wedge */
        }
    }
    return 8; /* Just treat everything else as a hex, as it originally would have */
}

/* Use in cases where there exists only a tet-version of the associated hex metric */
#define HANDLE_DGEN_TET(TFUNC)                                                      \
    int uniq[4];                                                                    \
    if (degen_case(coordinates, uniq) == 4)                                         \
    {                                                                               \
      VERDICT_REAL tet_coordinates[4][3] = {                                        \
        coordinates[uniq[0]][0], coordinates[uniq[0]][1], coordinates[uniq[0]][2],  \
        coordinates[uniq[1]][0], coordinates[uniq[1]][1], coordinates[uniq[1]][2],  \
        coordinates[uniq[2]][0], coordinates[uniq[2]][1], coordinates[uniq[2]][2],  \
        coordinates[uniq[3]][0], coordinates[uniq[3]][1], coordinates[uniq[3]][2]}; \
        return TFUNC(4, tet_coordinates);                                           \
    }

/* Use in cases where there exists all versions of the associated hex metric */
#define HANDLE_DGEN_ALL(TFUNC, PFUNC, WFUNC)                                       \
  int uniq[6];                                                                     \
  switch (degen_case(coordinates, uniq))                                           \
  {                                                                                \
    case 4:                                                                        \
    {                                                                              \
      VERDICT_REAL tet_coordinates[4][3] = {                                       \
        coordinates[uniq[0]][0], coordinates[uniq[0]][1], coordinates[uniq[0]][2], \
        coordinates[uniq[1]][0], coordinates[uniq[1]][1], coordinates[uniq[1]][2], \
        coordinates[uniq[2]][0], coordinates[uniq[2]][1], coordinates[uniq[2]][2], \
        coordinates[uniq[3]][0], coordinates[uniq[3]][1], coordinates[uniq[3]][2]};\
        return TFUNC(4, tet_coordinates);                                          \
    }                                                                              \
    case 5:                                                                        \
    {                                                                              \
      VERDICT_REAL pyramid_coordinates[5][3] = {                                   \
        coordinates[uniq[0]][0], coordinates[uniq[0]][1], coordinates[uniq[0]][2], \
        coordinates[uniq[1]][0], coordinates[uniq[1]][1], coordinates[uniq[1]][2], \
        coordinates[uniq[2]][0], coordinates[uniq[2]][1], coordinates[uniq[2]][2], \
        coordinates[uniq[3]][0], coordinates[uniq[3]][1], coordinates[uniq[3]][2], \
        coordinates[uniq[4]][0], coordinates[uniq[4]][1], coordinates[uniq[4]][2]};\
        return PFUNC(5, pyramid_coordinates);                                      \
    }                                                                              \
    case 6:                                                                        \
    {                                                                              \
      VERDICT_REAL wedge_coordinates[6][3] = {                                     \
        coordinates[uniq[0]][0], coordinates[uniq[0]][1], coordinates[uniq[0]][2], \
        coordinates[uniq[1]][0], coordinates[uniq[1]][1], coordinates[uniq[1]][2], \
        coordinates[uniq[2]][0], coordinates[uniq[2]][1], coordinates[uniq[2]][2], \
        coordinates[uniq[3]][0], coordinates[uniq[3]][1], coordinates[uniq[3]][2], \
        coordinates[uniq[4]][0], coordinates[uniq[4]][1], coordinates[uniq[4]][2], \
        coordinates[uniq[5]][0], coordinates[uniq[5]][1], coordinates[uniq[5]][2]};\
        return WFUNC(6, wedge_coordinates);                                        \
    }                                                                              \
    default: break;                                                                \
  }
