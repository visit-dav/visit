#! /bin/env cqperl

use CQPerlExt;

my $user = "";
my $pass = "";
my $output = "";
my $db = "";

if ($#ARGV == -1) {
  print "\nThis script lists available scoring rules.\n";
  print "\nRequired command line arguments:\n";
  print "\t-u  <user>   : User login\n";
  print "\t-p  <pass>   : User password\n";
  print "\t-db <db>     : Database\n";
  print "Optional command line arguments:\n";
  print "\t-d          : Additional debug info\n";
  print "\t-o <file>   : Write tab-delimited file\n";
  exit(0);
}

$debug = 0;

# Get command line options
while (@ARGV) {
    $_ = shift @ARGV;
    if ( m/^\-[hH]$/ ) {         # -h... or -H...
        &usage;
        exit(0);
    }
    elsif ( m/^-u$/ ) {
        $user = shift @ARGV;
    }
    elsif ( m/^-p$/ ) {
        $pass = shift @ARGV;
    }
    elsif ( m/^\-o$/ ) {
        $output = shift @ARGV;
    }
    elsif ( m/^-db$/ ) {
        $db = shift @ARGV;
    }
    elsif ( m/^-d$/) {
        $debug = 1;
    }
    else {
        die "Unrecognized argument: '$_'\n";
    }
}

if ($debug) {
    open(DEBUG, ">-");
}
else {
    open(DEBUG, ">/dev/null");
}

if (length($user) == 0) {
    print "Must supply username (-u)\n";
    exit(-1);
}
if (length($pass) == 0) {
    print "Must supply password (-p)\n";
    exit(-2);
}

# Open the ClearQuest session
my $session = CQSession::Build();

# Login
$session->UserLogon($user, $pass, $db, "");

# Query for all records resolved on the given branch
my $querydef = $session->BuildQuery("ScoringRule");
$querydef->BuildField("Name");
$querydef->BuildField("Description");
$querydef->BuildField("DefaultCustFactor");
$querydef->BuildField("LikelihoodPower");
$querydef->BuildField("SeriousnessScale");
$querydef->BuildField("UserImprovementScale");
$querydef->BuildField("DeveloperImprovementScale");
$querydef->BuildField("SeriousnessPower");
$querydef->BuildField("UserImprovementPower");
$querydef->BuildField("DeveloperImprovementPower");
$querydef->BuildField("TaskCostScale0");
$querydef->BuildField("TaskCostScale1");
$querydef->BuildField("TaskCostScale2");
$querydef->BuildField("TaskCostScale3");
$querydef->BuildField("TaskCostScale4");
$querydef->BuildField("TaskCostScale5");
$querydef->BuildField("ToolGrowthMultiplier");
$querydef->BuildField("CustomerFactorWeight");
$querydef->BuildField("ImportanceFactorWeight");
$querydef->BuildField("FinalAveragingPower");
$querydef->BuildField("WeightByTaskCost");


# If they specified '*' as the owner, then don't add in a
# query on that string
my $resultset = $session->BuildResultSet($querydef);
$resultset->Execute();


## ---------------------------------------------------------------------------
## ---------------------------------------------------------------------------

if ($output ne "")
{
    open(OUTPUT, ">$output");
}

## ---------------------------------------------------------------------------
## ---------------------------------------------------------------------------

# Iterate over the list and set the resolved field
while (($resultset->MoveNext()) == 1) {

    $name = $resultset->GetColumnValue(1);

    $default_customer_factor   = $resultset->GetColumnValue(3);

    $likelihood_power          = $resultset->GetColumnValue(4);

    $seriousness_scale         = $resultset->GetColumnValue(5);
    $user_improve_scale        = $resultset->GetColumnValue(6);
    $devel_improve_scale       = $resultset->GetColumnValue(7);

    $seriousness_power         = $resultset->GetColumnValue(8);
    $user_improve_power        = $resultset->GetColumnValue(9);
    $devel_improve_power       = $resultset->GetColumnValue(10);

    @taskcost_scale            =($resultset->GetColumnValue(11),
                                 $resultset->GetColumnValue(12),
                                 $resultset->GetColumnValue(13),
                                 $resultset->GetColumnValue(14),
                                 $resultset->GetColumnValue(15),
                                 $resultset->GetColumnValue(16));

    $toolgrowth_multiplier     = $resultset->GetColumnValue(17);

    $customer_factor_weight    = $resultset->GetColumnValue(18);
    $importance_factor_weight  = $resultset->GetColumnValue(19);

    $final_averaging_power     = $resultset->GetColumnValue(20);

    $weight_by_task_cost       = $resultset->GetColumnValue(21);

    print DEBUG "\n";
    print DEBUG "NAME = \"$name\"\n";
    print DEBUG "  default_customer_factor  =  $default_customer_factor\n";
    print DEBUG "  likelihood_power         =  $likelihood_power\n";
    print DEBUG "  seriousness_scale        =  $seriousness_scale\n";
    print DEBUG "  user_improve_scale       =  $user_improve_scale\n";
    print DEBUG "  devel_improve_scale      =  $devel_improve_scale\n";
    print DEBUG "  seriousness_power        =  $seriousness_power\n";
    print DEBUG "  user_improve_power       =  $user_improve_power\n";
    print DEBUG "  devel_improve_power      =  $devel_improve_power\n";
    print DEBUG "  taskcost_scale           = (@taskcost_scale)\n";
    print DEBUG "  toolgrowth_multiplier    =  $toolgrowth_multiplier\n";
    print DEBUG "  customer_factor_weight   =  $customer_factor_weight\n";
    print DEBUG "  importance_factor_weight =  $importance_factor_weight\n";
    print DEBUG "  final_averaging_power    =  $final_averaging_power\n";
    print DEBUG "  weight_by_task_cost      =  $weight_by_task_cost\n";

    if ($output ne "")
    {
        for ($i = 1 ; $i <= $resultset->GetNumberOfColumns() ; $i++)
        {
            $str = $resultset->GetColumnValue($i);
            $str =~ s/\t/\        /; # get rid of tabs, as we're using them as delimiters
            $str =~ s/\r//; # get rid of ^M's, too
            print OUTPUT $str."\t";
        }
        print OUTPUT "\n";
    }
    elsif (! $debug)
    {
        print "$name\n";
    }
}

if ($output ne "")
{
    close(OUTPUT);
}

# Close up the session
CQSession::Unbuild($session);


