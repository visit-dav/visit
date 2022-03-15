# Initialize any variables you may want to export
function bv_blosc_initialize
{
    # help!
    export DO_BLOSC="maybe"
}

# Enables the module
function bv_blosc_enable
{
    DO_BLOSC="yes"
}

# Disables the module
function bv_blosc_disable
{
    DO_BLOSC="no"
}

# Where to get the module, the version, etc...
function bv_blosc_info
{
    echo ""
}

# Ensure the module has been downloaded and extracted properly. Set and check variables here..
function bv_blosc_ensure
{
    echo ""
}

# What other modules does this module depend on. Example "adios" returns string "mxml"
function bv_blosc_depends_on
{
    echo ""
}

# build the module
function bv_blosc_build
{
    echo ""
}

function bv_blosc_print
{
    printf ""
}

function bv_blosc_print_usage
{
    printf ""
}

function bv_blosc_host_profile
{
    printf ""
}

function bv_blosc_dry_run
{
    printf ""
}

function bv_blosc_is_installed
{
    printf ""
}

function bv_blosc_is_enabled
{
    printf ""
}
