// NOTE - This code incomplete and requires underlined portions
// to be replaced with code to read values from your file format.

void
avtXXXFileFormat::GetCycles(std::vector<int> &cycles)
{
    int ncycles, *vals = 0;
    ncycles = OPEN FILE AND READ THE NUMBER OF CYCLES;
    READ ncycles INTEGER VALUES INTO THE vals ARRAY;

    // Store the cycles in the vector.
    for(int i = 0; i < ncycles; ++i)
        cycles.push_back(vals[i]);

    delete [] vals;
}

void
avtXXXFileFormat::GetTime(std::vector<double> &times)
{
    int ntimes;
    double *vals = 0;
    ntimes = OPEN FILE AND READ THE NUMBER OF TIMES;
    READ ntimes DOUBLE VALUES INTO THE vals ARRAY;

    // Store the times in the vector.
    for(int i = 0; i < ntimes; ++i)
        times.push_back(vals[i]);

    delete [] vals;
}

int
avtXXXXFileFormat::GetNTimesteps(void)
{
    std::vector<double> times;
    GetTimes(times);

    return times.size();
}
